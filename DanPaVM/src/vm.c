#include "vm.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "syscalls.h"

#ifndef MIN
    #define MIN(a,b) \
       ({ __typeof__ (a) _a = (a); \
           __typeof__ (b) _b = (b); \
         _a < _b ? _a : _b; })
#endif

// returns CLEARED memory
uint16_t alloc_memory_region(vm_state_t *vm, uint32_t size)
{
    for (uint32_t i = vm->next_likely_free_region; i < MAX_MEMORY_REGIONS; ++i)
    {
        // free
        if (vm->mem_regions[i].base == NULL)
        {
            vm->mem_regions[i].base = calloc(size, sizeof(var_t));
            vm->mem_regions[i].size = size;

            vm->next_likely_free_region = i+1;
            ++vm->allocated_region_count;
            return i;
        }
    }
    if (vm->next_likely_free_region != 0)
    {
        vm->next_likely_free_region = 0;
        // try searching again, starting from the beginning of the memory this time
        return alloc_memory_region(vm, size);
    }

    // if we are out of memory regions, try running the gc
    if (vm_run_gc(vm))
    {
        // tail-call to retry
        return alloc_memory_region(vm, size);
    }

    // if we reach this point, even running the GC couldn't help

    fprintf(stderr, "out of memory regions\n");
    assert(vm->allocated_region_count == MAX_MEMORY_REGIONS);
    abort();
}

void resize_memory_region(vm_state_t *vm, uint16_t region_id, uint32_t size)
{
    vm->mem_regions[region_id].base = realloc(vm->mem_regions[region_id].base, size * sizeof(var_t));
    vm->mem_regions[region_id].size = size;
}

void free_memory_region(vm_state_t *vm, uint16_t region_id)
{
    assert(vm->mem_regions[region_id].base != NULL);

    free(vm->mem_regions[region_id].base);
    vm->mem_regions[region_id].base = NULL;
    vm->mem_regions[region_id].size = 0;
    --vm->allocated_region_count;

    assert(vm->mem_regions[region_id].base == NULL);
}

vm_state_t *create_vm(const char *script_path)
{
    FILE *input = fopen(script_path, "rb");
    fseek(input, 0, SEEK_END);
    long fsize = ftell(input);
    rewind(input);  /* same as rewind(f); */

    if (fsize <= 0)
    {
        fprintf(stderr, "could not read input file");
        return NULL;
    }

    printf("size of the vm_state structure : %llu\n", sizeof(vm_state_t));

    char signature[4];
    fread(signature, 1, 4, input);

    if (strncmp(signature, "DNPX", 4) != 0)
    {
        printf("invalid signature\n");
        return NULL;
    }

    uint32_t init_addr;
    fread(&init_addr, sizeof(uint32_t), 1, input);
    uint32_t main_addr;
    fread(&main_addr, sizeof(uint32_t), 1, input);

    uint16_t string_table_size;
    fread(&string_table_size, sizeof(uint16_t), 1, input);
    string_entry_t* string_table = malloc(sizeof(string_entry_t) * string_table_size);
    for (int i = 0; i < string_table_size; ++i)
    {
        fread(&string_table[i].str_len, sizeof(uint16_t), 1, input);
        string_table[i].str = malloc(string_table[i].str_len);
        fread(string_table[i].str, 1, string_table[i].str_len, input);
    }

    uint8_t* buffer = malloc(fsize);
    fread(buffer, 1, fsize, input);
    fclose(input);

    vm_state_t* vm = calloc(1, sizeof(vm_state_t));
    vm->exec_buffer = buffer;
    vm->pc = vm->init_addr = init_addr;
    vm->main_addr = main_addr;
    vm->rand_seed = time(0);
    vm->string_table = string_table;
    vm->string_table_size = string_table_size;
    vm->sp = 0xFFF8;
    vm->allocated_region_count = 0;
    init_syscalls(vm);

    return vm;
}

void cleanup_vm(vm_state_t *vm)
{
    for (int i = 0; i < MAX_MEMORY_REGIONS; ++i)
    {
        if (vm->mem_regions->base)
            free_memory_region(vm, i);
    }
    for (int i = 0; i < vm->string_table_size; ++i)
    {
        free(vm->string_table[i].str);
    }
    free(vm->string_table);
    free(vm->exec_buffer);
    free(vm);
}

void vm_reset(vm_state_t *vm)
{
    vm->pc = vm->main_addr;
    vm->sp = 0xFFF8;
    vm->call_depth = 0;
    vm->stopped = 0;
}

void vm_clear(vm_state_t *vm)
{
    for (int i = 0; i < MAX_MEMORY_REGIONS; ++i)
    {
        if (vm->mem_regions[i].base)
            free_memory_region(vm, i);
    }
    assert(vm->allocated_region_count == 0);

    memset(vm->mem_regions, 0, sizeof(memory_region_t)*MAX_MEMORY_REGIONS);
    memset(vm->loc_var_pages, 0, sizeof(var_t)*LVAR_PAGES*VAR_COUNT);
    memset(vm->glob_vars, 0, sizeof(var_t)*VAR_COUNT);
}

void vm_run_init(vm_state_t *vm)
{
    vm_reset(vm);
    vm->pc = vm->init_addr;
    vm_run(vm);
}

void vm_read_str(vm_state_t *vm, var_t str, char* buf, unsigned buf_size)
{
    if (VAR_TYPE(str) != STR)
        return;

    unsigned j;
    for (j = 0; j < MIN(vm->mem_regions[VAR_OBJECT(str)].size, buf_size); ++j)
    {
        buf[j] = VAR_VAL(vm->mem_regions[VAR_OBJECT(str)].base[j]);
    }
    if (j < buf_size)
        buf[j] = '\0';

    buf[buf_size-1] = '\0'; // to ensure that buf is always a null-terminated string

}
