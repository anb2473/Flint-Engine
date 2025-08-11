// Read object from index array
// If only a location is provided, load data from obj file to array
// Otherwise load data directly from array

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"
#include "../utils/uthash.h"

typedef struct {
    uint32_t obj_loc;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

typedef enum {
    TYPE_INT,
    TYPE_MAP,
} LocOrDataType;

typedef struct {
    char* name;                 // key
    char* data;
    UT_hash_handle hh;      // makes this hashable
} Data;

typedef struct {
    LocOrDataType type;
    union {
        IndexArrayEntry* idx_entry;
        Data *map;
    };
} LocOrData;

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

// TODO: Implement read_obj function
int read_obj(const char* db_path, DBIndex* db_index, uint32_t obj_id, uint32_t table_id) {
    StructureObjectsArray* soa = (StructureObjectsArray*)utarray_eltptr(db_index->index_table_array, table_id);

    if (!soa) {
        perror("Table not found");
        return 1;
    }

    IndexArrayEntry* entry = (IndexArrayEntry*)utarray_eltptr(soa->objects, obj_id);

    if (!entry) {
        perror("Object not found");
        return 1;
    }

    if (entry->obj_loc == UINT32_MAX) {
        perror("Object has been deleted");
        return 1;
    }

    // Open the object file and read the data at the specified location
    FILE* obj_file = fopen(strcat(db_path, "/db/db.obj"), "rb");
    if (!obj_file) {
        perror("Failed to open object file");
        return 1;
    }

    // Seek to the object location
    if (fseek(obj_file, entry->obj_loc, SEEK_SET) != 0) {
        perror("Failed to seek to object location");
        fclose(obj_file);
        return 1;
    }

    // TODO: Read and parse the object data based on the schema
    // This would involve reading the binary data and converting it to the appropriate format
    
    fclose(obj_file);
    return 0;
}