#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "../states/attribute-type.h"
#include "../utils/utarray.h"

static UT_icd char_icd = {sizeof(char), NULL, NULL, NULL};

typedef struct {
    char *name;
    AttributeType type;
    UT_array *properties; // Attributes for the property
} Attribute;

typedef struct {
    AttributeType type;
    union {
        int i;
        float f;
        char *s;
    } value;
} AttributeValue;

AttributeType parse_attribute_type(const char* type_str);

Attribute* create_attribute(const char* name, AttributeType type, UT_array* properties);

Attribute* alloc_attr();

AttributeValue parse_str_to_val(const char *input, AttributeType type);

AttributeValue* copy_attribute_value(const AttributeValue *value);

#endif