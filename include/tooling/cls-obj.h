#ifndef CLS_OBJ_H
#define CLS_OBJ_H

// Clears the db.idx and db.obj files and resets the index_table_array.

#include "../../include/structs/db-index.h"

// Returns:
//   CLS_OBJ_SUCCESS (0) on success,
//   CLS_OBJ_ERR_IDX (1) if clearing db.idx fails,
//   CLS_OBJ_ERR_OBJ (2) if clearing db.obj fails.


int cls_obj(const char* db_path, DBIndex* db_index);

#endif