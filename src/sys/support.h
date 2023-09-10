#ifndef SUPPORT_H
#define SUPPORT_H

/// Tests the support of the OS and CPU of required features.
/// Return NULL if support is ok, or a string containing the reason of the failure of the check.
const char* test_cpu_os_support();

#endif // SUPPORT_H
