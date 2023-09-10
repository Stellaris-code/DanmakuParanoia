#ifndef ASL_H
#define ASL_H

#include <stdint.h>
#include <limits.h>

#include "utils/dynarray.h"
#include "utils/hash_table.h"
#include "math/easing.h"

// Corresponds to a string inside the string table.
// Each str_entry_t corresponds to a unique string.
// Furthermore, entry1 == entry2 <=> string(entry1) == string(entry2)
// This is made possible by the use of a hash table compiling all the already-encountered strings.
typedef uint16_t str_entry_t;
#define INVALID_STR_ENTRY ((uint16_t)-1)

#define MAX_ARG_COUNT 4
#define MAX_STACK_DEPTH 8
#define MAX_CONTROL_ACTION_EXTRA_BYTES 24
#define MAX_COROUTINES 64
#define MAX_STRINGS (1 << (sizeof(str_entry_t)*8))-1 // (2^16)-1 = 65535 for uint16_t

typedef enum asl_type_t
{
    ASL_define = 0,
    ASL_parallel,
    ASL_seq,
    ASL_for,
    ASL_play,
    ASL_timed_action,

    ASL_TYPE_MAX
} asl_type_t;

typedef enum asl_arg_type_t
{
    ASLArgAngle,
    ASLArgLength,
    ASLArgDuration,
    ASLArgString
} asl_arg_type_t;

// Used to signal dynamic arg values like percentage, that can vary depending on the execution context
typedef enum asl_arg_dynamic_tag_t
{
    Static = 0,
    Percentage
} asl_arg_dynamic_tag_t;

typedef struct asl_argument_t
{
    uint8_t type; // asl_arg_type_t
    uint8_t dyn_tag; // asl_arg_dynamic_tag_t
    uint8_t interp; // easing_function_enum
    str_entry_t name;
    union
    {
        float angle_rad;
        float length_px;
        float length_pct; // percentage
        float duration_s;
        str_entry_t string;
    };
} asl_argument_t;

// Represents the role of a floating point argument (e.g. used for handling screen dimension percentages)
typedef enum asl_flt_arg_role_t
{
    Width,
    Height,
    PosX,
    PosY,
    Distance,
    Angle,
    Other
} asl_flt_arg_role_t;

typedef struct asl_base_t
{
    asl_type_t type;
    unsigned size;
    struct asl_base_t* next;
} asl_base_t;

typedef struct asl_control_action_t
{
    asl_base_t base;
    uint8_t extra_data[MAX_CONTROL_ACTION_EXTRA_BYTES];
} asl_control_action_t;

#define DEFINE_ACTION(name, ...) typedef struct asl_##name##_t { \
    asl_base_t base; \
    __VA_ARGS__ \
} asl_##name##_t; \
_Static_assert(sizeof(asl_##name##_t) <= sizeof(asl_control_action_t), "Action type is too large compared to action base class max size");

#define DEFINE_CONTROL_ACTION(name, ...) \
    DEFINE_ACTION(name, __VA_ARGS__)

#define DEFINE_INSTANT_ACTION(name, ...) \
    DEFINE_ACTION(name, __VA_ARGS__)

typedef struct asl_timed_action_t
{
    asl_base_t base;

    float duration;
    float start_time;
    str_entry_t action_name;
    uint8_t arg_count;
    asl_argument_t args[MAX_ARG_COUNT]; // up to four args;
} asl_timed_action_t;

typedef struct asl_macro_entry_t
{
    str_entry_t name;
    asl_base_t* action_list;
} asl_macro_entry_t;

typedef struct asl_program_t
{
    asl_base_t* action_list;
    DYNARRAY(asl_macro_entry_t) macros;
    DYNARRAY(asl_base_t*) allocated_action_list; // lists of pointers of all actions ever allocated, for easy memory management
    char* string_data; // Contains literals separated by '\0'
    uint32_t string_data_idx;
    DYNARRAY(uint32_t) string_table; // each uint16_t entry corresponds to an offset within the string table
    hash_table_t string_hash_table; // A hash table that associates a string to it's corresponding string table entry if it exists
} asl_program_t;

typedef struct asl_coroutine_t
{
    asl_base_t* action; // NULL is this coroutine entry is not active
    float start_time;
    uint64_t dependencies; // a 64-bit bitmap of the indexes of tasks that are required to be completed before the execution of this one
    asl_base_t* end_ptr;
} asl_coroutine_t;

// Contains data related to the context of the executing environment
typedef struct asl_env_info_t
{
    unsigned screen_width, screen_height;
} asl_env_info_t;

typedef struct asl_exec_state_t
{
    asl_program_t* program;
    float elapsed_time;
    asl_env_info_t env_info;
    unsigned current_coroutine;
    asl_coroutine_t coroutines[MAX_COROUTINES];
} asl_exec_state_t;

typedef struct timed_action_handler_t
{
    const char* action_name;
    int (*handler)(asl_timed_action_t*, asl_exec_state_t*, float);
} timed_action_handler_t;

timed_action_handler_t* find_timed_action(const char* name);

// Returns the string referred to by the string table entry
const char *asl_string(const asl_program_t* program, str_entry_t idx);

// Parse an ASL program into an asl_program_t structure
asl_program_t asl_parse(const char* source);
// Inits an ASL execution state
void asl_init_exec(asl_exec_state_t* state, asl_program_t* program);
// Frees the memory allocated by an ASL program
void asl_cleanup(asl_program_t* program);
// Executes a step of the ASL program
int asl_step_run(asl_exec_state_t* state, float dt);

// Functions used by custom ASL timed actions
float get_argument_flt(const asl_timed_action_t* asl_action, asl_exec_state_t *state, asl_argument_t* ptr, asl_flt_arg_role_t role);
float get_argument_flt_nointerp(asl_argument_t* ptr);
asl_argument_t* get_argument_ptr(const asl_program_t* program, const char* arg_name, int default_pos, asl_timed_action_t *action);

DEFINE_CONTROL_ACTION(define,
    struct
    {
        str_entry_t anim_name;
        asl_base_t* action_list;
        asl_base_t* actual_next_action;
    };
    )

DEFINE_CONTROL_ACTION(seq,
    struct
    {
        asl_base_t* action_list;
        asl_base_t* end_ptr;
    };
    )

typedef struct parallel_entry_t
{
    asl_base_t* action_list;
    asl_base_t* end_ptr;
} parallel_entry_t;

DEFINE_CONTROL_ACTION(parallel,
    struct {
        DYNARRAY(parallel_entry_t) entry_list;
    };
    )

DEFINE_CONTROL_ACTION(for,
    struct {
    unsigned i;
    unsigned loop_count;
    asl_base_t* action_list;
    };
    )

DEFINE_CONTROL_ACTION(play,
    struct {
    str_entry_t macro_name;
    };
    )

typedef void(*action_destructor)(asl_base_t*);
extern action_destructor action_destructors[ASL_TYPE_MAX];

#endif // ASL_H
