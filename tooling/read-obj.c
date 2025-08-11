// Read object from index array
// If only a location is provided, load data from obj file to array
// Otherwise load data directly from array

// \x01 is a seperator, \x02 is a null value, \x03 is a marker that that data region is empty

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"
#include "../utils/uthash.h"

typedef struct {
    uint32_t obj_loc;
    uint16_t obj_size;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

typedef enum {
    TYPE_LOC,
    TYPE_MAP,
} LocOrDataType;

typedef struct {
    char *name;                 // key
    AttributeValue *data;       // value
    UT_hash_handle hh;          // makes this struct hashable
} Data;

/**
 * Deep copy an AttributeValue.
 */
AttributeValue *copy_attribute_value(const AttributeValue *src) {
    AttributeValue *copy = malloc(sizeof(AttributeValue));
    if (!copy) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    copy->type = src->type;
    switch (src->type) {
        case TYPE_INT:
            copy->value.i = src->value.i;
            break;
        case TYPE_FLOAT:
            copy->value.f = src->value.f;
            break;
        case TYPE_STR:
            copy->value.s = strdup(src->value.s);
            if (!copy->value.s) {
                perror("strdup");
                free(copy);
                exit(EXIT_FAILURE);
            }
            break;
    }
    return copy;
}


/**
 * Create a new Data object.
 */
Data *create_data(const char *name, const AttributeValue *value) {
    Data *d = malloc(sizeof(Data));
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
void insert_data(Data **table, const char *name, const AttributeValue *value) {
    Data *existing;
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
        Data *d = create_data(name, value);
        HASH_ADD_KEYPTR(hh, *table, d->name, strlen(d->name), d);
    }
}


typedef struct {
    LocOrDataType type;
    union {
        IndexArrayEntry* idx_entry;
        Data *map;
    };
} LocOrData;

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

typedef struct {
    char* name;
    UT_array* attributes
} TableStructure;

typedef struct {
    char* name;
    AttributeType type;
    UT_array* properties; // Attributes for the property
} Attribute;

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

typedef struct {
    AttributeType type;
    union {
        int i;
        float f;
        char *s;
    } value;
} AttributeValue;

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

Data* read_obj(const char* db_path, DBIndex* db_index, uint32_t obj_id, uint32_t table_id) {
    StructureObjectsArray* soa = utarray_eltptr(db_index->index_table_array, table_id);
    if (!soa) {
        fprintf(stderr, "Table not found\n");
        return NULL;
    }

    LocOrData* entry = utarray_eltptr(soa->objects, obj_id);
    if (!entry) {
        fprintf(stderr, "Object not found\n");
        return NULL;
    }

    if (entry->type == TYPE_LOC) {
        IndexArrayEntry* entry_loc = entry->idx_entry;
        if (entry_loc->obj_loc == UINT32_MAX && entry_loc->idx_loc == UINT32_MAX) {
            fprintf(stderr, "Object location is empty\n");
            return NULL;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/db/db.obj", db_path);
        FILE* obj_file = fopen(path, "rb");
        if (!obj_file) {
            perror("Failed to open object file");
            return NULL;
        }

        if (fseek(obj_file, entry_loc->obj_loc, SEEK_SET) != 0) {
            perror("Seek failed");
            fclose(obj_file);
            return NULL;
        }

        Data* data = NULL; // empty hash table

        TableStructure* table_schema = utarray_eltptr(db_index->schema_table_array, table_id);
        UT_array* attributes = table_schema->attributes;

        char buffer[256];
        size_t buf_len = 0;
        uint32_t current_attr_index = 0;

        for (uint32_t pos = 0; pos < entry_loc->obj_size; pos++) {
            int ch = fgetc(obj_file);
            if (ch == EOF) break;

            if (ch == '\x01') { // separator
                buffer[buf_len] = '\0'; // Terminate string safely
                Attribute* current_attribute = utarray_eltptr(attributes, current_attr_index);
                AttributeValue val = parse_str_to_val(buffer, current_attribute->type);
                insert_data(&data, current_attribute->name, &val);
                if (val.type == TYPE_STR) free(val.value.s); // cleanup after copy
                buf_len = 0;
                current_attr_index++;
            } else {
                if (buf_len < sizeof(buffer) - 1) {
                    buffer[buf_len++] = (char)ch;
                }
            }
        }

        fclose(obj_file);
        return data;
    }
    else if (entry->type == TYPE_MAP) {
        return entry->map;
    }

    return NULL;
}
