#ifndef JSON_H
#define JSON_H

#include <stdbool.h>
#include <stdint.h>

#define JSON_IGNORE_TRAILING_GARBAGE
#define JSON_NEST_LIMIT 1024
#define JSON_FAST_DOUBLE_PRINT
#define JSON_FAST_DOUBLE_PRINT_PRECISION 17
#define JSON_FAST_DOUBLE_PRINT_PRECISION_EXP 1e17

typedef struct json_object_t
{
    unsigned int entry_count;
    unsigned int start_idx;  // in the keyval buffer
} json_object_t;

typedef struct json_array_t
{
    unsigned int entry_count;
    unsigned int start_idx;  // in the keyval buffer
} json_array_t;

typedef union json_number_t
{
    double num_real;
    int64_t num_int;
} json_number_t;

typedef struct json_value_t
{
    enum
    {
        JSON_OBJECT,
        JSON_ARRAY,
        JSON_STRING,
        JSON_NUMBER_INT,
        JSON_NUMBER_FLOAT,
        JSON_BOOL,
        JSON_NULL,

        JSON_KEY
    } type;
    union
    {
        json_object_t object;
        json_array_t array;
        struct
        {
            unsigned int str_idx;
            unsigned int str_len;
        };
        json_number_t num;
        bool boolean;
    };
} json_value_t;

typedef struct json_result_t
{
    bool accepted;
    union
    {
        json_value_t value;
        struct
        {
            const char* reason;
            int index; // TODO : replace with (line, col) pair
        } error;
    };
} json_result_t;

typedef struct json_key_value_t
{
    struct json_value_t value;
    unsigned int next_value_idx;
    unsigned int str_idx;
    unsigned int str_len;
} json_key_value_t;

typedef void*(realloc_callback_t)(void*, int);

typedef struct json_context_t
{
    char* string_buffer; unsigned int string_buffer_size; realloc_callback_t* string_realloc;
    json_key_value_t* key_val_buffer; unsigned int key_val_buffer_size; realloc_callback_t* key_val_realloc;
    unsigned int string_buffer_brk; unsigned int key_val_buffer_brk;
} json_context_t;

json_result_t parse_json        (const char* input, int input_size, json_context_t* context);
unsigned int  print_json        (json_value_t* value, json_context_t* context, bool pretty, char* buffer, unsigned int buffer_size);
json_value_t *json_pointer      (const char* pointer, json_value_t* root, json_context_t* context);
const char   *json_get_string   (json_value_t* val, json_context_t* context);
json_value_t *json_create_string(const char* str, json_context_t* context);
json_value_t *json_create_value (json_context_t* context, uint8_t type);
void          json_object_add   (json_context_t* context, json_value_t* object, const char* key, json_value_t* value);
void          json_array_add    (json_context_t* context, json_value_t* object, json_value_t* value);
json_value_t *json_array_get    (json_context_t* context, json_value_t* array, unsigned idx);

bool json_require_float(double* val, const char* ptr, json_value_t* root, json_context_t* context);

bool json_require_int(int64_t* val, const char* ptr, json_value_t* root, json_context_t* context);

const char* json_string_or(const char* alternative, const char* ptr, json_value_t* root, json_context_t* context);

float json_float_or(float alternative, const char* ptr, json_value_t* root, json_context_t* context);

int64_t json_int_or(int64_t alternative, const char* ptr, json_value_t* root, json_context_t* context);

#endif // JSON_H
