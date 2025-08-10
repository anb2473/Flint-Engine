// Clear all tables

#include <stdio.h>
#include "../utils/utarray.h"
#include <stdint.h>

// Clear the schema file, removing all tables

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

typedef struct {
    char* name;
    UT_array* attributes
} TableStructure;

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

typedef struct {
    char* name;
    AttributeType type;
    UT_array* properties; // Attributes for the property
} Attribute;

// Init function for TableStructure
void table_structure_init(void *p) {
    TableStructure *ts = (TableStructure*)p;
    ts->name = NULL;
    ts->attributes = NULL;
}

// Custom destructor for Attribute
void attribute_free(Attribute* attr) {
    if (!attr) return;
    free(attr->name);

    if (attr->properties) {
        Attribute* prop = NULL;
        while ((prop = (Attribute*)utarray_next(attr->properties, prop))) {
            attribute_free(prop);
        }
        utarray_free(attr->properties);
    }

    free(attr);
}

// Copy function for UT_array (deep copy for nested UT_arrays)
void attribute_copy(void* dst, const void* src) {
    Attribute* d = (Attribute*)dst;
    const Attribute* s = (const Attribute*)src;
    d->name = strdup(s->name);
    d->type = s->type;

    if (s->properties) {
        utarray_new(d->properties, &attribute_icd);
        Attribute* p = NULL;
        while ((p = (Attribute*)utarray_next(s->properties, p))) {
            Attribute copy = {0};
            attribute_copy(&copy, p);
            utarray_push_back(d->properties, &copy);
        }
    } else {
        d->properties = NULL;
    }
}

void attribute_dtor(void* attr) {
    attribute_free((Attribute*)attr);
}

UT_icd attribute_icd = {
    sizeof(Attribute),
    NULL,
    attribute_copy,
    attribute_dtor
};

// Copy function (deep copy of name and attributes)
void table_structure_copy(void *dst, const void *src) {
    TableStructure *d = (TableStructure*)dst;
    const TableStructure *s = (const TableStructure*)src;

    d->name = s->name ? strdup(s->name) : NULL;

    if (s->attributes) {
        utarray_new(d->attributes, &attribute_icd);
        Attribute *a = NULL;
        while ((a = (Attribute*)utarray_next(s->attributes, a))) {
            Attribute copy = {0};
            attribute_copy(&copy, a);
            utarray_push_back(d->attributes, &copy);
        }
    } else {
        d->attributes = NULL;
    }
}

// Destructor (free name and attributes)
void table_structure_dtor(void *p) {
    TableStructure *ts = (TableStructure*)p;
    if (ts->name) free(ts->name);
    if (ts->attributes) {
        utarray_free(ts->attributes);
        ts->attributes = NULL;
    }
}

// UT_icd for TableStructure
UT_icd table_structure_icd = {
    sizeof(TableStructure),
    table_structure_init,
    table_structure_copy,
    table_structure_dtor
};

int cls_obj(char* db_path, DBIndex* db_index) {
    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "w");

    if (schema_file == NULL) {
        perror("Failed to clear schema.flint file");
        return 1;
    }

    fclose(schema_file);

    utarray_new(db_index->schema_table_array, &table_structure_icd);   // Clear index table array

    return 0;
}