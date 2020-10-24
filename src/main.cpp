#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <iostream>

#include "libtcc.h"

void invoke_something(int amt) {
    while (amt--) printf("invoked something\n");
}

int main(int argc, char** argv) {

    TCCState*   s;
    FILE*       f = NULL;
    char*       buffer = NULL;
    size_t      scriptsize = 0;

    do {
        f = fopen("script.c", "r");
        if (!f) return -1;

        fseek(f, 0, SEEK_END);
        scriptsize = ftell(f);
        rewind(f);

        buffer = (char*)malloc(scriptsize + 1);
        if (!buffer) return -1;

        memset(buffer, 0, scriptsize);
        fread(buffer, 1, scriptsize, f);
        fclose(f);

        buffer[scriptsize] = 0;

        s = tcc_new();
        if (!s) break;

        tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

        /*
        tcc_add_symbol(s, "GetProcAddress", GetProcAddress);
        tcc_add_symbol(s, "GetModuleHandleA", GetModuleHandleA);
        tcc_add_symbol(s, "LoadLibraryA", LoadLibraryA);
        tcc_add_symbol(s, "GetLastError", GetLastError);
        */

        tcc_add_symbol(s, "invoke_something", invoke_something);

        if (tcc_compile_string(s, buffer) == -1)
            break;

        if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
            break;

        auto entry = reinterpret_cast<int(*)(int, char**)>(
            tcc_get_symbol(s, "main"));

        if (!entry) break;

        entry(0, nullptr);

    } while (0);

    tcc_delete(s);
    free(buffer);

    return 0;
}