#ifndef TABLE_STRUCTURE_ICD_H
#define TABLE_STRUCTURE_ICD_H

#include "../structs/table-structure.h"
#include "../utils/utarray.h"
#include "../structs/attribute.h"
#include "../icds/attribute-icd.h"

// Init function for TableStructure
void table_structure_init(void *p);

// Copy function (deep copy of name and attributes)
void table_structure_copy(void *dst, const void *src);

// Destructor (free name and attributes)
void table_structure_dtor(void *p);

// UT_icd for TableStructure
extern UT_icd table_structure_icd;

#endif