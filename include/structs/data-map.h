#ifndef DATA_MAP_H
#define DATA_MAP_H

#include "../structs/attribute.h"
#include "../utils/uthash.h"

typedef struct {
    char *name;                 // key
    AttributeValue *data;       // value
    UT_hash_handle hh;          // makes this struct hashable
} DataMap;

DataMap *create_data(const char *name, const AttributeValue *value);

void insert_data(DataMap **table, const char *name, const AttributeValue *value);

#endif