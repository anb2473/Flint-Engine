// Clear all objects

#include <stdio.h>
#include "../utils/utarray.h"
#include <stdint.h>

// Include structs
#include "../structs/db-index.h"
#include "../structs/structure-objects-array.h"

// Include icds
#include "../icds/structure-objects-array-icd.h"

// Clear the idx and obj files, removing all objects

int cls_obj(char* db_path, DBIndex db_index) {
    FILE* idx_file = fopen(strcat(db_path, "/db/db.idx"), "w");

    if (idx_file == NULL) {
        perror("Failed to clear db.idx file");
        return 1;
    }

    fclose(idx_file);

    FILE* obj_file = fopen(strcat(db_path, "/db/db.obj"), "w");

    if (obj_file == NULL) {
        perror("Failed to clear db.obj file");
        return 1;
    }

    fclose(obj_file);

    // Clear index table array - this will be recreated with the new structure
    utarray_free(db_index.index_table_array);
    utarray_new(db_index.index_table_array, &structure_objects_array_icd);

    return 0;
}