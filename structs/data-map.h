#ifndef DATA_MAP_H
#define DATA_MAP_H

#include "../utils/uthash.h"
#include "../structs/attribute.h"

typedef struct {
    char *name;                 // key
    AttributeValue *data;       // value
    UT_hash_handle hh;          // makes this struct hashable
} DataMap;


/**
 * Create a new Data object.
 */
DataMap *create_data(const char *name, const AttributeValue *value) {
    DataMap *d = malloc(sizeof(DataMap));
    if (!d) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    d->name = strdup(name);
    if (!d->name) {
        perror("strdup");
        free(d);
        exit(EXIT_FAILURE);
    }

    d->data = copy_attribute_value(value);
    return d;
}

/**
 * Insert or update a value in the hash table.
 */
void insert_data(DataMap **table, const char *name, const AttributeValue *value) {
    DataMap *existing;
    HASH_FIND_STR(*table, name, existing);

    if (existing) {
        // Free the old value
        if (existing->data->type == TYPE_STR) {
            free(existing->data->value.s);
        }
        free(existing->data);

        // Store a deep copy of the new value
        existing->data = copy_attribute_value(value);
    } else {
        DataMap *d = create_data(name, value);
        HASH_ADD_KEYPTR(hh, *table, d->name, strlen(d->name), d);
    }
}

#endif