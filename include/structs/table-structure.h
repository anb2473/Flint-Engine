#ifndef TABLE_STRUCTURE_H
#define TABLE_STRUCTURE_H

#include "../utils/utarray.h"

typedef struct {
    char* name;
    UT_array* attributes;
} TableStructure;

TableStructure* alloc_table_structure() {
    TableStructure *table_structure = malloc(sizeof(TableStructure));
    utarray_new(table_structure->attributes, &attribute_icd);
    return table_structure;
}

#endif