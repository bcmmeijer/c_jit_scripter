#include <type_traits>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <memory>
#include <Windows.h>
#include <functional>
#include <cassert>
#include <sstream>

#include "libtcc.h"
#include "ioredirect.hpp"

using _defer = std::shared_ptr<void>;
#define defer(x, code) _defer x(nullptr, [&](...){ code; })

template <typename T> struct return_type { using type = T; };
template <>           struct return_type<void> { using type = bool; };
template <typename T> using  return_type_t = typename return_type<T>::type;

class tcc {
public:
    tcc() : _s(nullptr) {}
    ~tcc() { destroy(); }

    bool init() {
        if (_s) tcc_delete(_s);
        _s = tcc_new();
        return _s != nullptr;
    }

    void destroy() {
        tcc_delete(_s);
        _s = nullptr;
    }

    bool reinit() {
        destroy();
        return init();
    }

    bool compile_string(const char* str) {
        tcc_set_output_type(_s, TCC_OUTPUT_MEMORY);
        return tcc_compile_string(_s, str) != -1;
    }

    bool compile_file(const char* file) {
        FILE* f = nullptr;
        size_t scriptsize = 0;
        std::vector<char> buffer;

        fopen_s(&f, file, "r");
        if (!f) return false;

        fseek(f, 0, SEEK_END);
        scriptsize = ftell(f);
        rewind(f);

        buffer.resize(scriptsize + 1);
        fread(buffer.data(), 1, scriptsize, f);
        fclose(f);

        buffer[scriptsize] = 0;

        return compile_string(buffer.data());
    }

    template <typename T>
    T get_symbol(const char* symbol) {
        tcc_relocate(_s, TCC_RELOCATE_AUTO);
        return reinterpret_cast<T>(tcc_get_symbol(_s, symbol));
    }

    bool add_symbol(const char* symbol, void* ptr) {
        return tcc_add_symbol(_s, symbol, ptr);
    }


    template <typename T, typename ... Args>
    std::pair<bool, return_type_t<T>> run_string(const char* str, const char* entry, Args ... args) {
        if (!compile_string(str))
            return { false, return_type_t<T>(0) };
        return { true, _runner<T>(entry, std::forward<Args>(args)...) };
    }

    template <typename T, typename ... Args>
    std::pair<bool, return_type_t<T>> run_file(const char* file, const char* entry, Args ... args) {
        if (!compile_file(file))
            return { false, return_type_t<T>(0) };
        return { true, _runner<T>(entry, std::forward<Args>(args)...) };
    }

    std::stringstream& get_stdout() { return _ss; }

private:

    void stdio_redirect(bool enable = true) {
        static io_redirect io;
        if (enable) {
            io.Init();
            io.BeginCapture();
        }
        else {
            io.EndCapture();
            _ss << io.GetCapture();
        }
    }

    template <typename T, typename ... Args>
    return_type_t<T> _runner(const char* entry, Args ... args) {
        constexpr bool argc = sizeof ... (args) != 0;
        defer(do_reinit, this->reinit());

        auto e = get_symbol<T(*)(Args...)>(entry);
        if (!e) return return_type_t < T>(0);

        stdio_redirect();
        defer(do_restore_redirect, stdio_redirect(false));

        if constexpr (std::is_same_v<T, void>) {
            if constexpr (argc) e(args...);
            else e();
            return return_type_t < T>(0);
        }
        else {
            return_type_t<T> r;
            if constexpr (argc) r = e(args...);
            else r = e();
            return r;
        }
    }

private:
    TCCState* _s;
    std::stringstream _ss;
};

void test_module_print() { 
    printf("test 1 2 3\n"); 
}

struct _test_module {
    void(*print)() = test_module_print;
} test_module;

static void* module_require(const char* module) {
    static std::unordered_map<std::string, void*> modules = {
        { "test_module", &test_module }
    };
    return modules[module];
}

static void* function_resolver(const char* dll, const char* proc) {
    HMODULE mod = nullptr;
    mod = GetModuleHandleA(dll);
    if (!mod) mod = LoadLibraryA(dll);
    if (!mod) return nullptr;
    return GetProcAddress(mod, proc);
}

int main(int argc, char** argv) {

    tcc runner;

    runner.init();
    runner.add_symbol("require", module_require);
    runner.add_symbol("resolver", function_resolver);

    runner.compile_string("int add_me(int x, int y) { return x + y; }");
    auto [success, ret] = runner.run_string<int>(R"(
        int a(int x, int y) {
            int res = add_me(x, y);
            printf("result of add_me: %d\n", res);
            return res;
        })","a", 10, 10);

    assert(ret == 20);
    printf("program output:\n%s\n", runner.get_stdout().str().c_str());

    return 0;
}
