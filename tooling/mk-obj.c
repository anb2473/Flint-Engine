// Create a new object

#include <stdint.h>
#include <stdbool.h>
#include "../utils/utarray.h"
#include "../utils/uthash.h"
#include <stdio.h>

typedef struct {
    char* key;
    char* value;
    UT_hash_handle hh;
} HashItem;


typedef struct {
    char* name;
    UT_array* attributes
} TableStructure;

typedef struct {
    char* name;
    AttributeType type;
    UT_array* properties; // Attributes for the property
} Attribute;

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

char* attr_type_to_str(AttributeType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_STR: return "str";
        case TYPE_FLOAT: return "float";
        default: return "unknown";
    }
}

bool is_property(char* property_name, UT_array* properties) {
    for (int i = 0; i < utarray_len(properties); i++) {
        if (utarray_eltptr(properties, i) == property_name) {
            return true;
        }
    }
    return false;
}

typedef struct {
    uint32_t object_id;
    uint8_t  table_id; // The id of the table structure (maximum 256 tables)
    uint16_t size; // The size of the object in bytes (maximum 65,535 bytes in a single object)
} IndexEntry;

typedef struct {
    uint32_t obj_loc;
    uint32_t idx_loc;
} IndexArrayEntry;

int mk_obj(const char* db_path, DBIndex* db_index, uint32_t structure_id, HashItem* data) {
    // Data will be given as a hashmap
    // Data must be stored as an array
    // Cross check each value in the hashmap with the schema to find the correct position

    // Create a char representation of the object, find its length, 
    // and then write its metadata to the db.idx and its raw data to the db.obj
    
    // Char representation format should look like:
    // sHello World\x02i0

    char* obj_data = "";

    UT_array* attributes = ((TableStructure*) utarray_eltptr(db_index->schema_table_array, structure_id))->attributes;
    size_t attr_len = utarray_len(attributes);

    for (int i = 0; i < attr_len; i++) {
        Attribute* attr = (Attribute*) utarray_eltptr(attributes, i);
        HashItem* item;
        HASH_FIND_STR(data, attr->name, item);
        if (item) {
            obj_data = strcat(obj_data, item->value);
            if (i != attr_len - 1) {  // if not last attr, add a seperator
                obj_data = strcat(obj_data, "\x01");
            }
        } else {
            if (is_property("nullable", attr->properties)) {    // Check if property is nullable
                obj_data = strcat(obj_data, "\x02");
            } else {
                perror("Nulled non nullable value");
                return 1;
            }
        }
    }

    FILE* obj_file = fopen(strcat(db_path, "/db/db.obj"), "ab");
    fprintf(obj_data, obj_file);
    
    // Get file length
    fseek(obj_file, 0, SEEK_END); // Move file pointer to the end
    long obj_file_len = ftell(obj_file);  // Get current position (which is the size)

    fclose(obj_file);
    free(obj_file);
    
    FILE* idx_file = fopen(strcat(db_path, "/db/db.idx"), "ab");

    // Number of items in the structure array + 1 is the id of the new object
    int new_object_id = utarray_len((UT_array*) utarray_eltptr(db_index->index_table_array, structure_id)) + 1;

    IndexEntry entry = {
        .object_id = new_object_id,
        .table_id = structure_id,
        .size = strlen(obj_data),
    };

    fprintf(&entry, sizeof(IndexEntry), 1, idx_file);
    fclose(idx_file);
    free(idx_file);

    UT_array* structure_obj_array = (UT_array*) utarray_eltptr(db_index->index_table_array, structure_id);

    uint32_t* max_filled_ptr = (uint32_t*) utarray_eltptr(structure_obj_array, 0);
    uint32_t max_filled;
    if (max_filled_ptr >= utarray_len(structure_obj_array) || max_filled_ptr == UINT32_MAX) {
        max_filled_ptr = UINT32_MAX;
        max_filled = UINT32_MAX;
    }
    else {
        max_filled_ptr = max_filled_ptr++;  // Increment max filled
        max_filled = *max_filled_ptr;
    }

    if (utarray_len(db_index->empty_indexes) == 0 && max_filled == UINT32_MAX) {
        // Create new object in array
        IndexArrayEntry array_entry = {
            .obj_loc = obj_file_len,
            .idx_loc = new_object_id
        };
        utarray_push_back(structure_obj_array, &array_entry);

        return 0;
    }
    if (max_filled != UINT32_MAX) {
        uint32_t* empty_index_slot = (uint32_t*) utarray_eltptr(structure_obj_array, max_filled);

        if (empty_index_slot != UINT32_MAX) {
            perror("Index marked empty contains uncleared data");
            return 1;
        }

         // Create new object in array
        IndexArrayEntry array_entry = {
            .obj_loc = obj_file_len,
            .idx_loc = new_object_id
        };
        empty_index_slot = &array_entry;
        
        return 0;
    }
}