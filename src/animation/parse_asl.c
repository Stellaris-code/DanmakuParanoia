#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "anim_sequence.h"

#define MAX_NEST_DEPTH 32
#define SPACES_PER_TAB 4

void parse_program(const char* source, asl_program_t* program);

typedef struct source_location_t
{
    const char* filename;
    const char* line_ptr;
    const char* ptr;
    int line;
} source_location_t;

typedef struct parsing_context_t
{
    source_location_t loc;
    asl_program_t* program;
    unsigned nest_depth;
    unsigned indent_len_stack[MAX_NEST_DEPTH];
} parsing_context_t;

#define CREATE_CONTROL_ACTION(ctx, action) \
    ({ asl_##action##_t* tmp = danpa_alloc(sizeof(asl_##action##_t)); \
    DYNARRAY_ADD((ctx)->program->allocated_action_list, (asl_base_t*)tmp); \
    tmp->base.size = sizeof(asl_##action##_t); tmp->base.type = ASL_##action; \
    tmp->base.next = NULL; \
    tmp; })

#define CREATE_TIMED_ACTION(ctx) \
    ({ asl_timed_action_t* tmp = danpa_alloc(sizeof(asl_timed_action_t)); \
    DYNARRAY_ADD((ctx)->program->allocated_action_list, (asl_base_t*)tmp); \
    tmp->base.size = sizeof(asl_timed_action_t); tmp->base.type = ASL_timed_action; \
    tmp->base.next = NULL; \
    tmp; })

static inline void push_indent_len(parsing_context_t* ctx, unsigned len)
{
    ++ctx->nest_depth;
    assert(ctx->nest_depth < MAX_NEST_DEPTH);

    ctx->indent_len_stack[ctx->nest_depth] = len;
}

static inline void pop_indent_len(parsing_context_t* ctx)
{
    assert(ctx->nest_depth > 0);

    --ctx->nest_depth;
}

static inline void update_loc_newline(parsing_context_t* ctx, const char* line_start)
{
    ++ctx->loc.line;
    ctx->loc.ptr = ctx->loc.line_ptr = line_start;
}

typedef enum lexer_flags
{
    STOP_ON_NEWLINE = (1<<0),
    STOP_ON_PREPROC = (1<<1),
    STARTS_ON_NEWLINE = (1<<2),
    LEX_SINGLE_TOKEN = (1<<3)
} lexer_flags;

_Noreturn static void error()
{
    abort();
}

static inline void expect(parsing_context_t* ctx, char c)
{
    if (*ctx->loc.ptr != c)
        error();
    ++ctx->loc.ptr;
}

static inline int is_newline(const char* ptr)
{
    return *ptr == '\n' || strncmp(ptr, "\r\n", 2) == 0;
}
static inline void skip_newline(const char** ptr)
{
    if (strncmp(*ptr, "\r\n", 2) == 0 || strncmp(*ptr, "\n\r", 2) == 0)
        *ptr += 2;
    else
        ++*ptr;
}

// returns 1 if it's on a new line
static inline int skip_whitespace(parsing_context_t* ctx)
{
    // skip all whitespace
    while (isspace(*ctx->loc.ptr))
        if (is_newline(ctx->loc.ptr))
            return 1;
        else
            ++ctx->loc.ptr;

    return 0;
}

// returns 1 if it's on a new line
static inline int skip_whitespace_newlines(parsing_context_t* ctx)
{
    int on_new_line = 0;

    // skip all whitespace
    while (isspace(*ctx->loc.ptr))
    {
        if (is_newline(ctx->loc.ptr))
        {
            on_new_line = 1;

            skip_newline(&ctx->loc.ptr);
            update_loc_newline(ctx, ctx->loc.ptr);
        }
        else
        {
            ++ctx->loc.ptr;
        }
    }

    return on_new_line;
}

static inline unsigned indent_level(const char* line_ptr, const char** end_of_indent)
{
    unsigned indent_len = 0;
    while (*line_ptr == '\t' || *line_ptr == ' ')
    {
        if (*line_ptr == '\t')
            indent_len += SPACES_PER_TAB;
        else if (*line_ptr == ' ')
            ++indent_len;
        ++line_ptr;
    }

    if (end_of_indent)
        *end_of_indent = line_ptr;

    return indent_len;
}


static inline void expect_newline(parsing_context_t* ctx)
{
    if (!is_newline(ctx->loc.ptr))
        error();
    skip_newline(&ctx->loc.ptr);
}

static inline void expect_current_indent(parsing_context_t* ctx)
{
    assert(ctx->nest_depth < MAX_NEST_DEPTH);

    unsigned target_indent_len = ctx->indent_len_stack[ctx->nest_depth];
    unsigned actual_indent_len = indent_level(ctx->loc.ptr, &ctx->loc.ptr);
    if (actual_indent_len != target_indent_len)
        error();
}

static inline void expect_previous_indent(parsing_context_t* ctx)
{
    assert(ctx->nest_depth > 0);

    pop_indent_len(ctx);
    expect_current_indent(ctx);
}

static inline void expect_deeper_indent(parsing_context_t* ctx)
{
    if (ctx->nest_depth >= MAX_NEST_DEPTH-1)
        error(); // nest depth too large

    unsigned previous_indent_len = ctx->indent_len_stack[ctx->nest_depth];
    unsigned actual_indent_len = indent_level(ctx->loc.ptr, &ctx->loc.ptr);
    if (actual_indent_len <= previous_indent_len)
        error();

    push_indent_len(ctx, actual_indent_len);
}

// Returns an index to within the string table
static str_entry_t parse_literal(parsing_context_t* ctx)
{
    if (!isalpha(*ctx->loc.ptr))
        return INVALID_STR_ENTRY;

    const char* start = ctx->loc.ptr;
    while (isalpha(*ctx->loc.ptr))
        ++ctx->loc.ptr;


    str_entry_t entry;

    unsigned len = ctx->loc.ptr - start;
    uint32_t data_off = ctx->program->string_data_idx;
    char* str = ctx->program->string_data + data_off;
    memcpy(str, start, len);
    str[len] = '\0';

    hash_value_t* hash_val;
    // String has already been encountered
    if ((hash_val = hash_table_get(&ctx->program->string_hash_table, str)))
    {
        return hash_val->idx;
    }
    else
    {
        // advance the string data index to mark the string data zone as used
        ctx->program->string_data_idx += len + 1; // plus the null terminator
        entry = ctx->program->string_table.size;
        if (entry >= MAX_STRINGS)
            error();

        DYNARRAY_ADD(ctx->program->string_table, data_off); // insert the offset into the table
        hash_table_insert(&ctx->program->string_hash_table, str, (hash_value_t){.idx = entry});

        return entry;
    }
}

static void parse_argument(parsing_context_t* ctx, asl_argument_t* arg)
{
    skip_whitespace(ctx);

    str_entry_t arg_name = INVALID_STR_ENTRY;

    memset(arg, 0, sizeof(asl_argument_t));

    // named argument
    if (*ctx->loc.ptr == '[')
    {

        ++ctx->loc.ptr;
        arg_name = parse_literal(ctx);
        expect(ctx, ']');
        skip_whitespace(ctx);
        expect(ctx, ':');

        arg->name = arg_name;
    }

    skip_whitespace(ctx);
    // parse number
    if (isdigit(*ctx->loc.ptr) || *ctx->loc.ptr == '+' || *ctx->loc.ptr == '-')
    {
        char* pEnd;
        float data = strtof(ctx->loc.ptr, &pEnd);
        ctx->loc.ptr = pEnd;

        skip_whitespace(ctx);

        // utf-8 encoding for '°'
        if (strncmp(ctx->loc.ptr, "\xc2\xb0", 2) == 0)
        {
            ctx->loc.ptr += 2;

            arg->type = ASLArgAngle;
            arg->angle_rad = data / 180.0 * 3.1415926; // deg to rad
        }
        else if (strncmp(ctx->loc.ptr, "rad", 3) == 0)
        {
            ctx->loc.ptr += 3;

            arg->type = ASLArgAngle;
            arg->angle_rad = data;
        }
        else if (strncmp(ctx->loc.ptr, "px", 2) == 0)
        {
            ctx->loc.ptr += 2;

            arg->type = ASLArgLength;
            arg->length_px = data;
        }
        else if (strncmp(ctx->loc.ptr, "%", 1) == 0)
        {
            ++ctx->loc.ptr;

            arg->type = ASLArgLength;
            arg->length_pct = data/100.f;
            arg->dyn_tag = Percentage;
        }
        else if (strncmp(ctx->loc.ptr, "s", 1) == 0)
        {
            ++ctx->loc.ptr;

            arg->type = ASLArgDuration;
            arg->duration_s = data;
        }
        else if (strncmp(ctx->loc.ptr, "sec", 3) == 0)
        {
            ctx->loc.ptr += 3;

            arg->type = ASLArgDuration;
            arg->duration_s = data;
        }
        else if (strncmp(ctx->loc.ptr, "ms", 2) == 0)
        {
            ctx->loc.ptr += 2;

            arg->type = ASLArgDuration;
            arg->duration_s = data*1000.f;
        }
    }
    else if (*ctx->loc.ptr == '"')
    {
        ++ctx->loc.ptr;
        str_entry_t str = parse_literal(ctx);
        expect(ctx, '"');

        arg->type = ASLArgString;
        arg->string = str;
    }
    else if (isalpha(*ctx->loc.ptr))
    {
        str_entry_t str = parse_literal(ctx);

        arg->type = ASLArgString;
        arg->string = str;
    }

    arg->interp = strToEasing("none");
    // interpolation type
    if (*ctx->loc.ptr == '@')
    {
        ++ctx->loc.ptr;
        str_entry_t interp_func = parse_literal(ctx);
        if (interp_func == INVALID_STR_ENTRY)
            error();

        arg->interp = strToEasing(asl_string(ctx->program, interp_func));
    }

}

static asl_base_t* parse_action(parsing_context_t* ctx);

static void parse_then(parsing_context_t* ctx, asl_seq_t* seq)
{
    assert(strncmp(ctx->loc.ptr, "then", 4) == 0);
    ctx->loc.ptr += 4;

    skip_whitespace(ctx);
    expect_newline(ctx);
    expect_current_indent(ctx);

    asl_base_t* action_ptr = seq->action_list;

    do
    {
        action_ptr->next = parse_action(ctx);

        action_ptr = action_ptr->next;
    } while (indent_level(ctx->loc.ptr, NULL) == ctx->indent_len_stack[ctx->nest_depth]);

    seq->end_ptr = action_ptr->next;
}

static void parse_parallel(parsing_context_t* ctx, asl_parallel_t* parallel)
{
    skip_whitespace(ctx);
    expect_newline(ctx);
    expect_deeper_indent(ctx);

    DYNARRAY_INIT(parallel->entry_list, 4);
    DYNARRAY_RESIZE(parallel->entry_list, 1);

    parallel_entry_t* parallel_ptr = &parallel->entry_list.ptr[0];

    do
    {
        asl_base_t* action = parse_action(ctx);
        // "then" continuation statement:
        unsigned indent_lvl = indent_level(ctx->loc.ptr, NULL);
        if (indent_lvl > ctx->indent_len_stack[ctx->nest_depth])
        {
            asl_seq_t* seq = CREATE_CONTROL_ACTION(ctx, seq);
            seq->action_list = action;

            push_indent_len(ctx, indent_lvl);
            skip_whitespace(ctx);
            if (strncmp(ctx->loc.ptr, "then", 4) == 0 && isspace(ctx->loc.ptr[4]))
            {
                parse_then(ctx, seq);
                pop_indent_len(ctx);
            }
            else
                error();

            parallel_ptr->action_list = seq->action_list;
            parallel_ptr->end_ptr = seq->end_ptr;
        }
        else
        {
            parallel_ptr->action_list = action;
            parallel_ptr->end_ptr = action->next;
        }

        if (indent_level(ctx->loc.ptr, NULL) == ctx->indent_len_stack[ctx->nest_depth])
        {
            DYNARRAY_ADD(parallel->entry_list, {});
            parallel_ptr = &parallel->entry_list.ptr[parallel->entry_list.size-1];
        }
        else
            break;
    } while (1);
    pop_indent_len(ctx);
}

static void parse_for(parsing_context_t* ctx, asl_for_t* asl_for)
{
    skip_whitespace(ctx);

    char* pEnd;
    long loop_count = strtol(ctx->loc.ptr, &pEnd, 10);
    ctx->loc.ptr = pEnd;

    asl_for->i = 0;
    asl_for->loop_count = loop_count;

    skip_whitespace(ctx);

    if (strncmp(ctx->loc.ptr, "times", 5) == 0 && isspace(ctx->loc.ptr[5]))
    {
        ctx->loc.ptr += 5;
        skip_whitespace(ctx);
    }

    expect_newline(ctx);
    expect_deeper_indent(ctx);

    asl_for->action_list = parse_action(ctx);
    asl_base_t* action_ptr = asl_for->action_list;
    while (indent_level(ctx->loc.ptr, NULL) == ctx->indent_len_stack[ctx->nest_depth])
    {
        action_ptr->next = parse_action(ctx);
        action_ptr = action_ptr->next;
    }

    action_ptr->next = (asl_base_t*)asl_for; // reloop

    pop_indent_len(ctx);
}

static void parse_play(parsing_context_t* ctx, asl_play_t* play)
{
    skip_whitespace(ctx);

    expect(ctx, '"');
    play->macro_name = parse_literal(ctx);
    expect(ctx, '"');

    skip_whitespace(ctx);
    expect_newline(ctx);
}

static void parse_define(parsing_context_t* ctx, asl_define_t *define);
static asl_base_t *parse_action(parsing_context_t* ctx)
{
    str_entry_t action_name_entry = INVALID_STR_ENTRY;

    skip_whitespace(ctx);
    action_name_entry = parse_literal(ctx);
    if (action_name_entry == INVALID_STR_ENTRY)
        error();
    if (!isspace(*ctx->loc.ptr))
        error();

    const char* action_name = asl_string(ctx->program, action_name_entry);
    printf("action : '%s'\n", action_name);

    if (strcmp(action_name, "parallel") == 0)
    {
        asl_parallel_t* parallel = CREATE_CONTROL_ACTION(ctx, parallel);
        parse_parallel(ctx, parallel);
        return (asl_base_t*)parallel;
    }
    else if (strcmp(action_name, "for") == 0)
    {
        asl_for_t* asl_for = CREATE_CONTROL_ACTION(ctx, for);
        parse_for(ctx, asl_for);
        return (asl_base_t*)asl_for;
    }
    else if (strcmp(action_name, "play") == 0)
    {
        asl_play_t* asl_play = CREATE_CONTROL_ACTION(ctx, play);
        parse_play(ctx, asl_play);
        return (asl_base_t*)asl_play;
    }

    skip_whitespace(ctx);

    asl_timed_action_t* action = CREATE_TIMED_ACTION(ctx);
    action->action_name = action_name_entry;
    action->arg_count = 0;
    action->start_time = -1; // < 0 means the action isn't started yet

    // argument, not a duration specifier
    if (!(
                (strncmp(ctx->loc.ptr, "in", 2) == 0 && isspace(ctx->loc.ptr[2])) ||
                (strncmp(ctx->loc.ptr, "for", 3) == 0 && isspace(ctx->loc.ptr[3])))
            )
    {

        action->arg_count = 1;

        parse_argument(ctx, &action->args[0]);
        skip_whitespace(ctx);
        while (*ctx->loc.ptr == ',')
        {
            if (action->arg_count >= MAX_ARG_COUNT)
                error();

            ++ctx->loc.ptr;
            parse_argument(ctx, &action->args[action->arg_count]);
            skip_whitespace(ctx);

            ++action->arg_count;
        }
        skip_whitespace(ctx);
    }


    if ((strncmp(ctx->loc.ptr, "in", 2) == 0 && isspace(ctx->loc.ptr[2])) ||
            (strncmp(ctx->loc.ptr, "for", 3) == 0 && isspace(ctx->loc.ptr[3])))
    {
        ctx->loc.ptr += 3;
        skip_whitespace(ctx);

        char* pEnd;
        action->duration = strtof(ctx->loc.ptr, &pEnd);
        ctx->loc.ptr = pEnd;

        skip_whitespace(ctx);

        if (strncmp(ctx->loc.ptr, "s", 1) == 0 && isspace(ctx->loc.ptr[1]))
        {
            ++ctx->loc.ptr;
            action->duration *= 1.f;
        }
        if (strncmp(ctx->loc.ptr, "sec", 3) == 0 && isspace(ctx->loc.ptr[3]))
        {
            ctx->loc.ptr += 3;
            action->duration *= 1.f;
        }
        if (strncmp(ctx->loc.ptr, "ms", 2) == 0 && isspace(ctx->loc.ptr[2]))
        {
            ctx->loc.ptr += 2;
            action->duration *= 1000.f;
        }
    }

    skip_whitespace(ctx);
    expect_newline(ctx);

    return (asl_base_t*)action;
}

static void parse_define(parsing_context_t* ctx, asl_define_t* define)
{
    ctx->loc.ptr += 6; // strlen("define")

    skip_whitespace(ctx);
    expect(ctx, '"');
    define->anim_name = parse_literal(ctx);
    expect(ctx, '"');

    skip_whitespace(ctx);
    expect_newline(ctx);
    expect_deeper_indent(ctx);

    define->action_list = NULL;
    asl_base_t** action_ptr = &define->action_list;

    do
    {
        *action_ptr = parse_action(ctx);

        action_ptr = &(*action_ptr)->next;
    } while (indent_level(ctx->loc.ptr, NULL) == ctx->indent_len_stack[ctx->nest_depth]);

    pop_indent_len(ctx);
}

void parse_program(const char* source, asl_program_t* program)
{
    parsing_context_t ctx;
    ctx.loc.ptr = ctx.loc.line_ptr = source;
    ctx.loc.line = 1; ctx.loc.filename = "<source>";
    ctx.program = program;
    ctx.nest_depth = 0;
    ctx.indent_len_stack[ctx.nest_depth] = 0;

    DYNARRAY_INIT(program->macros, 8);
    DYNARRAY_INIT(program->allocated_action_list, 256);

    program->string_data = danpa_alloc(sizeof(char) * strlen(source)); // Won't ever have more string data that the input program
    program->string_data_idx = 0;
    DYNARRAY_INIT(program->string_table, 256);
    program->string_hash_table = mk_hash_table(1031); // prime

    program->action_list = NULL;
    asl_base_t** action_ptr = &program->action_list;

    while (*ctx.loc.ptr)
    {
        skip_whitespace_newlines(&ctx);
        if (strncmp(ctx.loc.ptr, "define", 6) == 0 && isspace(ctx.loc.ptr[6]))
        {
            asl_define_t* define = CREATE_CONTROL_ACTION(&ctx, define);
            parse_define(&ctx, define);

            asl_macro_entry_t macro;
            macro.name = define->anim_name;
            macro.action_list = define->action_list;

            *action_ptr = (asl_base_t*)define;
            action_ptr = &define->actual_next_action;
            define->base.next = NULL;

            DYNARRAY_ADD(program->macros, macro);
        }
        else
        {
            asl_base_t* action = parse_action(&ctx);

            *action_ptr = action;
            action_ptr = &(*action_ptr)->next;
        }
    }
}

// TODO : faire un véritable error handling!!
asl_program_t asl_parse(const char* source)
{
    asl_program_t program;
    parse_program(source, &program);

    return program;
}
