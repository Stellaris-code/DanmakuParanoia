#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "vm.h"

#if 0

static void syscall_hello_world(vm_state_t* vm)
{
    (void)vm;
    printf("Hello world!\n");
}

int main()
{
    vm_state_t* vm = create_vm("in.bin");

    register_syscall(vm, 42, syscall_hello_world);

    vm_run(vm);

    vm_run_gc(vm);

    cleanup_vm(vm);

    return 0;
}

#endif
