
#include "config.h"



#if defined(_WIN32)
    #include "stdint_windows.h"
#elif (defined(__linux__ )&&!defined (__ANDROID__))
    #include "stdint_linux.h"
#elif (defined(__linux__ )&&defined (__ANDROID__))
    #include "stdint_android.h"
#elif (defined (__APPLE__)&&defined( __MACH__))
    #include "stdint_apple.h"
#endif



