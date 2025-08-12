// Clear all objects:
// Clear the db.idx and db.obj files, and the index_table_array

#include <stdio.h>
#include "../../include/utils/utarray.h"
#include <stdint.h>

// Define return codes
#define CLS_OBJ_SUCCESS 0
#define CLS_OBJ_ERR_IDX 1
#define CLS_OBJ_ERR_OBJ 2

// Include limits for IO operations
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Include header file to expose functions
#include "../../include/tooling/cls-obj.h"

// Include structs
#include "../../include/structs/db-index.h"
#include "../../include/structs/structure-objects-array.h"

// Include icds for creating new UT_array objects
#include "../../include/icds/structure-objects-array-icd.h"

int cls_obj(const char* db_path, DBIndex* db_index) {
    /*
        Return codes:
        0 -- Success (CLS_OBJ_SUCCESS)
        1 -- Failed to clear db.idx file (CLS_OBJ_ERR_IDX)
        2 -- Failed to clear db.obj file (CLS_OBJ_ERR_OBJ)

        Args:
        char* db_path -- should not have trailing "\"
        DBIndex db_index
    */

    // Clear the db.idx file
    char idx_path[PATH_MAX];
    snprintf(idx_path, sizeof(idx_path), "%s/db/db.idx", db_path);
    FILE* idx_file = fopen(idx_path, "w");  // Using "w" mode to write over the existing file

    if (idx_file == NULL) {
        perror("Failed to clear db.idx file");
        return CLS_OBJ_ERR_IDX;
    }

    fclose(idx_file);

    // Clear the db.obj file
    char obj_path[PATH_MAX];
    snprintf(obj_path, sizeof(obj_path), "%s/db/db.obj", db_path);
    FILE* obj_file = fopen(obj_path, "w");  // Using "w" mode to write over the existing file

    if (obj_file == NULL) {
        perror("Failed to clear db.obj file");
        return CLS_OBJ_ERR_OBJ;
    }

    fclose(obj_file);

    // Free the index_table_array and replace with an empty array
    if (db_index->index_table_array != NULL) {
        utarray_free(db_index->index_table_array);
    }
    utarray_new(db_index->index_table_array, &structure_objects_array_icd);

    return CLS_OBJ_SUCCESS;   // Success
}