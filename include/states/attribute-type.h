#ifndef ATTRIBUTE_TYPE_H
#define ATTRIBUTE_TYPE_H

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

char *attr_type_to_str(AttributeType type);

#endif