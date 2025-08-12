#ifndef DB_INDEX_H
#define DB_INDEX_H

#include "../icds/obj-location-icd.h"

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

DBIndex make_db_index(UT_array* schema_table_array, UT_array* index_table_array);

#endif // DB_INDEX_H 