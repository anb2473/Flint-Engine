#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "stdio.h"
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

AttributeType parse_attribute_type(const char* type_str) {
    if (strcmp(type_str, "int") == 0) {
        return TYPE_INT;
    } else if (strcmp(type_str, "str") == 0) {
        return TYPE_STR;
    } else if (strcmp(type_str, "float") == 0) {
        return TYPE_FLOAT;
    }
    return -1; // Invalid type
}

Attribute* create_attribute(const char* name, AttributeType type) {
    Attribute* attr = (Attribute*)malloc(sizeof(Attribute));
    attr->name = strdup(name);
    attr->type = type;
    attr->properties = NULL;
    return attr;
}

Attribute* alloc_attr() {
    Attribute *attr = malloc(sizeof(Attribute));
    utarray_new(attr->properties, &char_icd); // Load UT_array to properties
    return attr;
}

AttributeValue parse_str_to_val(const char *input, AttributeType type) {
    AttributeValue attr;
    attr.type = type;

    switch (type) {
        case TYPE_INT:
            attr.value.i = atoi(input); // convert string to int
            break;
        case TYPE_FLOAT:
            attr.value.f = strtof(input, NULL); // convert string to float
            break;
        case TYPE_STR:
            attr.value.s = strdup(input); // duplicate string (malloc'd)
            break;
        default:
            fprintf(stderr, "Unknown type\n");
            exit(EXIT_FAILURE);
    }

    return attr;
}

#endif