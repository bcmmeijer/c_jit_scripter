/* functions to interact with the winapi */
extern void* GetModuleHandleA(const char*);
extern void* GetProcAddress(void* module, const char* name);
extern void* LoadLibraryA(const char* name);
extern unsigned long GetLastError();
/*-------------------------------------- */

/* useful funcs */
int printf(const char* fmt, ...);
void* memset(void* dst, int val, unsigned long len);
/* ------------ */

extern void invoke_something(int amt);

int main(int argc, char** argv) {
 
    invoke_something(3);

    return 0;
}