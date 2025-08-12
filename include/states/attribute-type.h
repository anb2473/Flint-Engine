#ifndef ATTRIBUTE_TYPE_H
#define ATTRIBUTE_TYPE_H

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

char *attr_type_to_str(AttributeType type) {
    switch (type)
    {
    case TYPE_INT:
        return "int";
    case TYPE_STR:
        return "str";
    case TYPE_FLOAT:
        return "float";
    default:
        return "unknown";
    }
}

#endif