// Clear all objects

#include <stdio.h>
#include "../utils/utarray.h"
#include <stdint.h>

// Clear the idx and obj files, removing all objects

typedef struct {
    uint32_t obj_loc;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

// Forward declaration of the UT_icd
extern UT_icd structure_objects_array_icd;

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