#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>

#include "motion.h"

#define MAX_ENTITIES 256
#define MAX_ENTITY_ATTRIBUTES 64
#define MAX_ENTITY_NAME_LEN 64

typedef union entity_value_t
{
    const char* str;
    int64_t   num;
    uint64_t unum;
    double real;
    void* ptr;
    motion_data_t motion;
} entity_value_t;

void clear_entities();
void register_entity(const char* name);

entity_value_t* get_entity_value(const char* name, const char* key);
void  set_entity_value(const char* name, const char* key, const entity_value_t* val);

#endif // ENTITY_H
