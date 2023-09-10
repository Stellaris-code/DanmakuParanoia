#ifndef PROPRETIES_H
#define PROPRETIES_H

#include <stdbool.h>

typedef union property_value_t
{
    const char* str;
    int ival;
    float rval;
} property_value_t;

typedef struct script_property_t
{
    bool is_constant;
    enum PropertyType
    {
        PropertyString,
        PropertyInt,
        PropertyReal
    } type;
    union
    {
        // dynamic property
        struct
        {
            property_value_t (*getter)();
            void (*setter)(property_value_t);
        };
        property_value_t const_val;
    };
} script_property_t;

void init_properties();

void register_script_property(const char* id, script_property_t property);
script_property_t *get_property(const char* id);

#endif // PROPRETIES_H
