#include "properties.h"

#include "utils/hash_table.h"
#include "sys/cleanup.h"

static hash_table_t property_table;

void register_script_property(const char *id, script_property_t property)
{
    hash_table_insert(&property_table, id, (hash_value_t){.property = property});
}

static void clear_property_table()
{
    hash_table_clear(&property_table);
}

static void release_property_table()
{
    free_hash_table(&property_table);
}
void init_properties()
{
    property_table = mk_hash_table(211); // prime
    register_cleanup(clear_property_table, GamestateEnd);
    register_cleanup(release_property_table, AppEnd);
}

script_property_t *get_property(const char *id)
{
    hash_value_t* value = hash_table_get(&property_table, id);
    if (!value)
        return NULL;
    return &value->property;
}
