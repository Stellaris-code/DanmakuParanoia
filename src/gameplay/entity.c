#include "entity.h"

#include <string.h>

#include "sys/log.h"

typedef struct entity_attribute_t
{
    const char* key;
    entity_value_t val;
    struct entity_attribute_t* next_attribute;
} entity_attribute_t;

typedef struct entity_t
{
    char name[MAX_ENTITY_NAME_LEN];
    entity_attribute_t* first_attribute;
} entity_t;

static entity_attribute_t attributes[MAX_ENTITIES*MAX_ENTITY_ATTRIBUTES];
static entity_t entities[MAX_ENTITIES];

void clear_entities()
{
    memset(entities  , 0, sizeof(entity_t)*MAX_ENTITIES);
    memset(attributes, 0, sizeof(entity_attribute_t)*MAX_ENTITIES*MAX_ENTITY_ATTRIBUTES);
}

void register_entity(const char *name)
{
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        if (entities[i].name[0] == '\0')
        {
            strncpy(entities[i].name, name, MAX_ENTITY_NAME_LEN);
            entities[i].name[MAX_ENTITY_NAME_LEN-1] = '\0'; // be safe
            entities[i].first_attribute = NULL;
            return;
        }
    }

    trace_log(LOG_WARNING, "couldn't register entity '%s'", name);
}

static entity_t* find_entity(const char* name)
{
    for (int i = 0; i < MAX_ENTITIES; ++i)
    {
        if (entities[i].name[0] == '\0')
            continue;
        if (strcmp(entities[i].name, name) == 0)
            return &entities[i];
    }

    return NULL;
}

static entity_attribute_t* alloc_attribute()
{
    for (int i = 0; i < MAX_ENTITIES*MAX_ENTITY_ATTRIBUTES; ++i)
    {
        if (attributes[i].key == NULL)
        {
            attributes[i].next_attribute = NULL;
            return &attributes[i];
        }
    }

    trace_log(LOG_WARNING, "couldn't register attribute");
    return NULL;
}

entity_value_t *get_entity_value(const char *name, const char *key)
{
    entity_t* entity = find_entity(name);
    if (!entity)
        return NULL;

    entity_attribute_t* attr_ptr = entity->first_attribute;
    while (attr_ptr)
    {
        if (strcmp(key, attr_ptr->key) == 0)
            return &attr_ptr->val;

        attr_ptr = attr_ptr->next_attribute;
    }

    return NULL;
}

void set_entity_value(const char *name, const char *key, const entity_value_t *val)
{
    entity_t* entity = find_entity(name);
    if (!entity)
        return;

    entity_attribute_t** attr_ptr = &entity->first_attribute;
    while (*attr_ptr)
    {
        if (strcmp((*attr_ptr)->key, key) == 0)
        {
            (*attr_ptr)->val = *val;
            return;
        }
        attr_ptr = &((*attr_ptr)->next_attribute);
    }
    // not found, create the attribute
    *attr_ptr = alloc_attribute();
    (*attr_ptr)->key = key;
    (*attr_ptr)->val = *val;
    (*attr_ptr)->next_attribute = NULL;
}
