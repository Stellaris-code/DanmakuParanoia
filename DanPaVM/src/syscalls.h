#ifndef SYSCALLS_H_INCLUDED
#define SYSCALLS_H_INCLUDED

#include "vm.h"

void init_syscalls(vm_state_t* vm);

void vm_syscall(vm_state_t* vm, int syscall_id);

#endif // SYSCALLS_H_INCLUDED
