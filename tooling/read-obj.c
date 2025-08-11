// Read object from index array
// If only a location is provided, load data from obj file to array
// Otherwise load data directly from array

// \x01 is a seperator, \x02 is a null value, \x03 is a marker that that data region is empty

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"
#include "../utils/uthash.h"

typedef struct {
    uint32_t obj_loc;
    uint16_t obj_size;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

typedef enum {
    TYPE_LOC,
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

Data* interpret_obj_str(char* obj_str) {

}

// TODO: Implement read_obj function
Data* read_obj(const char* db_path, DBIndex* db_index, uint32_t obj_id, uint32_t table_id) {
    StructureObjectsArray* soa = (StructureObjectsArray*)utarray_eltptr(db_index->index_table_array, table_id);

    if (!soa) {
        perror("Table not found");
        return 1;
    }

    LocOrData* entry = (LocOrData*)utarray_eltptr(soa->objects, obj_id);

    if (!entry) {
        perror("Object not found");
        return 1;
    }

    if (entry->type == TYPE_LOC) {
        IndexArrayEntry* entry_loc = entry->idx_entry;
        if (entry_loc->obj_loc == UINT32_MAX && entry_loc->idx_loc == UINT32_MAX) {
            perror("Object location is empty");
            return 1;
        }
    
        // Open the object file and read the data at the specified location
        FILE* obj_file = fopen(strcat(db_path, "/db/db.obj"), "rb");
        if (!obj_file) {
            perror("Failed to open object file");
            return 1;
        }
    
        // Seek to the object location
        if (fseek(obj_file, entry_loc->obj_loc, SEEK_SET) != 0) {
            perror("Failed to seek to object location");
            fclose(obj_file);
            return 1;
        }
    
        long current_pos = 0;

        while (current_pos < entry_loc->obj_size) { // Read object data
            current_pos++;
        }
        
        fclose(obj_file);
        return 0;
    }
    else if (entry->type == TYPE_MAP) {
        Data* entry_map = entry->map;
        return entry_map;
    }
}