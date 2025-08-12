#include "../../include/structs/db-index.h"
#include "../../include/utils/utarray.h"

DBIndex make_db_index(UT_array* schema_table_array, UT_array* index_table_array) {
    DBIndex result;
    result.schema_table_array = schema_table_array;
    result.index_table_array = index_table_array;
    utarray_new(result.empty_indexes, &obj_location_icd);
    return result;
}