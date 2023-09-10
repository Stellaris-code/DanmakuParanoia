#ifndef SCRIPT_SYSCALLS_H
#define SCRIPT_SYSCALLS_H

#include "vm.h"

#define MAX_PROPERTY_LEN 64

void init_scripting_syscalls(vm_state_t* vm);

#endif // SCRIPT_SYSCALLS_H
