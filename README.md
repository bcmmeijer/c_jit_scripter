# c_jit_scripter
Jit based C scripting using the TinyC Compiler.

premake5_reload.bat generates visual studio 2019 files. Change this to generate other types.

 ---- COMPILE TCC FOR STATIC LIBRARY  ----
 ---- https://github.com/TinyCC/tinycc ---

> comment out function tccpp.c@3741
> comment out tcc.h@341-343
> add tcc.h@344
#undef TCC_LIBTCC1
#define TCC_LIBTCC1 "" "

> change tcc.h@ ~1860 #define tcc_warning
#ifdef DEBUG
#define tcc_warning         TCC_SET_STATE(_tcc_warning)
#else
#define tcc_warning(...)

> cl /c /EHsc libtcc.c
> lib libtcc.obj