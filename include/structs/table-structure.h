#ifndef TABLE_STRUCTURE_H
#define TABLE_STRUCTURE_H

#include "../utils/utarray.h"

// Include icds
#include "../icds/attribute-icd.h"

typedef struct {
    char* name;
    UT_array* attributes;
} TableStructure;

TableStructure* alloc_table_structure();

#endif