// Read object from index array
// If only a location is provided, load data from obj file to array
// Otherwise load data directly from array

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"

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

// TODO: Implement read_obj function
int read_obj(const char* db_path, DBIndex* db_index, uint32_t obj_id, uint32_t table_id) {
    // Implementation needed
    return 0;
}