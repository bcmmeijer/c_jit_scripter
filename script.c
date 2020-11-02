void* require(const char* name);
void* resolver(const char* dll, const char* proc);
int   printf(const char*, ...);

typedef struct {
    void(*print)();
} test_module;

int entry(int argc, char** argv) {
    test_module* mod = require("test_module");
    mod->print();

    ((int(*)(void*, char*, char*, int))resolver("user32.dll", "MessageBoxA"))
        (0, "message", "title", 0);

    return 0;
}

