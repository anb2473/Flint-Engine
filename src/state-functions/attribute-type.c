#include "../../include/states/attribute-type.h"

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