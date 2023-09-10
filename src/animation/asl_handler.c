#include "asl_handler.h"

#include "animation/anim_sequence.h"
#include "sys/cleanup.h"
#include "sys/fs.h"
#include "sys/log.h"

typedef struct asl_entry_t
{
    asl_program_t program;
    asl_exec_state_t state;
} asl_entry_t;

DEFINE_RESOURCE_POOL(asl_pool, asl_entry_t, MAX_ASL_SCRIPTS);

void cleanup_asl_scripting();

void init_asl_scripting()
{
    register_cleanup(&cleanup_asl_scripting, GamestateEnd);
}

void cleanup_asl_scripting()
{
    FOR_EACH_RES(asl_pool, id, asl_entry_t*, ptr)
    {
        asl_cleanup(&ptr->program);
    }
}

asl_id_t load_asl_script(const char *filename)
{
    unsigned long len;
    char* file_contents = read_file(filename, &len);
    if (!file_contents || len == 0)
    {
        trace_log(LOG_WARNING, "Unable to read ASL file %s", filename);
        return INVALID_ASL_ID;
    }

    asl_id_t id = res_pool_alloc(&asl_pool);
    if (id == INVALID_RES_ID)
        return id;

    asl_entry_t* ptr = res_pool_get(&asl_pool, id);
    ptr->program = asl_parse(file_contents);
    asl_init_exec(&ptr->state, &ptr->program);

    return id;
}

void update_asl_scripts(float dt)
{
    FOR_EACH_RES(asl_pool, id, asl_entry_t*, ptr)
    {
        asl_step_run(&ptr->state, dt);
    }
}
