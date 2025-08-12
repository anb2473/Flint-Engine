#include "../../include/structs/attribute.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Attribute* create_attribute(const char* name, AttributeType type, UT_array* properties) {
    Attribute* attr = (Attribute*)malloc(sizeof(Attribute));
    attr->name = strdup(name);
    attr->type = type;
    attr->properties = properties;
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

AttributeValue* copy_attribute_value(const AttributeValue *value) {
    AttributeValue *copy = malloc(sizeof(AttributeValue));
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    copy->type = value->type;
    
    switch (value->type) {
        case TYPE_INT:
            copy->value.i = value->value.i;
            break;
        case TYPE_FLOAT:
            copy->value.f = value->value.f;
            break;
        case TYPE_STR:
            copy->value.s = strdup(value->value.s);
            if (!copy->value.s) {
                perror("strdup");
                free(copy);
                exit(EXIT_FAILURE);
            }
            break;
        default:
            fprintf(stderr, "Unknown attribute type in copy\n");
            free(copy);
            exit(EXIT_FAILURE);
    }
    
    return copy;
}