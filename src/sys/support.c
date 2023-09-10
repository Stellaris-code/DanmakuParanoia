#include "support.h"

const char *test_cpu_os_support()
{
    if (!__builtin_cpu_supports("sse2"))
        return "CPU needs to support at least SSE2";
    return 0;
}
