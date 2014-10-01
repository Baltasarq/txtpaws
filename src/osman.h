// osman.h
/*
    OS Manager

    Detects the OS which we are compiling for.
*/

#ifdef _Windows
    #define OS_WINDOWS
#else
    #ifdef __WIN32__
        #define OS_WINDOWS
    #else
        #ifdef __linux__
            #define OS_UNIX
        #else
            #ifdef __unix
                #define OS_UNIX
            #endif
        #endif
    #endif
#endif
