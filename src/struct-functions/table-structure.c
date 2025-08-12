#include "../../include/structs/table-structure.h"

TableStructure* alloc_table_structure() {
    TableStructure *table_structure = malloc(sizeof(TableStructure));
    utarray_new(table_structure->attributes, &attribute_icd);
    return table_structure;
}