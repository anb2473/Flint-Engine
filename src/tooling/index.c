// Index database

#include <stdio.h>
#include <stdint.h>
#include <ctype.h> 
#include "../../include/utils/utarray.h"
#include "../../include/utils/uthash.h"

// Include header file to expose functions
#include "../../include/tooling/index-db.h"

// Include str utils
#include "../../include/str-utils/append-char.h"

// Include limits for IO operations
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// Include structs
#include "../../include/structs/db-index.h"
#include "../../include/structs/index-array-entry.h"
#include "../../include/structs/structure-objects-array.h"
#include "../../include/structs/index-entry.h"
#include "../../include/structs/data-map.h"
#include "../../include/structs/attribute.h"
#include "../../include/structs/table-structure.h"
#include "../../include/structs/loc-or-data.h"
#include "../../include/structs/obj-location.h"
#include "../../include/structs/schema-parser-result.h"

// Include icds
#include "../../include/icds/index-array-entry-icd.h"
#include "../../include/icds/structure-objects-array-icd.h"
#include "../../include/icds/attribute-icd.h"
#include "../../include/icds/loc-or-data-icd.h"
#include "../../include/icds/table-structure-icd.h"
#include "../../include/icds/obj-location-icd.h"

// Include states
#include "../../include/states/schema-scanner-state.h"

// Go through every schema in the schema.flint file and every object in the db.idx file, and store their data for future use
// Whenever a request for an object is made, load its contents from its location and then add it to the FILO cache

SchemaParserResult load_schema(const char* db_path) {
    SchemaParserResult schema_result = {NULL, 0};

    // Open the db.idx file

    int read_byte_as_int; // Use 'int' for fgetc's return value to check for EOF
    unsigned char current_byte; // Use unsigned char for the actual byte value

    char schema_path[PATH_MAX];
    snprintf(schema_path, sizeof(schema_path), "%s/db/db.idx", db_path);
    FILE* schema_file = fopen(schema_path, "rb");

    // Each entry in the schema has a name and a structure
    // name {
    //    str *property_name @property_attr,
    //    int *property_name
    // }

    // Which would be stored as:
    // Structure [
    //     name, UT_ARRAY [
    //       Attribute [
    //          str, property_name, UT_ARRAY [ property_attr ]
    //       ] 
    //       Attribute [
    //          int, property_name, UT_ARRAY []
    //       ]
    //    ]
    // ]

    UT_array* schema_table_array;
    utarray_new(schema_table_array, &table_structure_icd); // Create a new outer UT_array for table structures

    TableStructure* table_structure = alloc_table_structure(); // Allocate memory for the table structure

    SchemaScannerState state = OUTSIDE_STRUCTURE; // Initial state

    char name[256] = {0}; // Buffer to hold the name of the structure
    char attribute_type[256] = {0};
    char attribute_name[256] = {0};
    char property[256] = {0};

    Attribute* current_attribute = NULL; // Pointer to the current attribute being processed
    
    if (schema_file == NULL) {
        perror("Error opening schema.flint file");
        return schema_result;   // {NULL, 0}
    }

    while ((read_byte_as_int = fgetc(schema_file)) != EOF) {    // Read until end of file
        // Cast the int result from fgetc to unsigned char for processing the byte value
        current_byte = (unsigned char)read_byte_as_int;

        if (isspace(current_byte) || iscntrl(current_byte)) {
            continue; // Skip whitespace characters or invisible control characters
        }

        switch (state) {
            case OUTSIDE_STRUCTURE:  
                // one case: name {
                if (current_byte == '{') {
                    table_structure->name = strdup(name); // Allocate memory for the name
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                append_char(name, 256, current_byte);
            case INSIDE_TYPE:
                // one case: type *name
                if (current_byte == '*') {
                    current_attribute = alloc_attr(); // Allocate memory for the current attribute
                    current_attribute->type = parse_attribute_type(attribute_type); // Parse the attribute type
                    state = INSIDE_NAME; // Transition to inside name
                    continue;
                }
                append_char(attribute_type, 256, current_byte);
            case INSIDE_NAME:
                // three cases, name @property, name, name }
                if (current_byte == '@') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    memset(attribute_name, 0, 256); // Flush attribute name
                    state = INSIDE_PROPERTY; // Transition to inside attribute
                    continue;
                }
                if (current_byte == ',') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    memset(attribute_name, 0, 256); // Flush attribute name
                    Attribute* prop_attr = create_attribute(current_attribute->name, parse_attribute_type(attribute_type), current_attribute->properties);
                    memset(attribute_type, 0, 256); // Flush attribute type
                    utarray_push_back(table_structure->attributes, prop_attr);
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    memset(attribute_name, 0, 256); // Flush attribute name
                    Attribute* prop_attr = create_attribute(current_attribute->name, parse_attribute_type(attribute_type), current_attribute->properties);
                    memset(attribute_type, 0, 256); // Flush attribute type
                    utarray_push_back(table_structure->attributes, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_result.length++;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                append_char(attribute_name, 256, current_byte);
            case INSIDE_PROPERTY:
                // three cases: @attr @attr, @attr {, @attr,
                if (current_byte == '@') {
                    state = INSIDE_PROPERTY; // Transition to inside attribute
                    utarray_push_back(current_attribute->properties, property);
                    continue;
                }
                if (current_byte == ',') {
                    utarray_push_back(current_attribute->properties, property);
                    Attribute* prop_attr = create_attribute(current_attribute->name, parse_attribute_type(attribute_type), current_attribute->properties);
                    memset(attribute_type, 0, 256); // Flush attribute type
                    utarray_push_back(table_structure->attributes, prop_attr);
                    free(prop_attr);  // Because utarray copies by value                    
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    utarray_push_back(current_attribute->properties, property);
                    Attribute* prop_attr = create_attribute(current_attribute->name, parse_attribute_type(attribute_type), current_attribute->properties);
                    memset(attribute_type, 0, 256); // Flush attribute type
                    utarray_push_back(table_structure->attributes, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_result.length++;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                append_char(property, 256, current_byte);
        }
    }

    schema_result.schema_table_array = schema_table_array;

    return schema_result;
}

UT_array* load_idx(const char* db_path, SchemaParserResult schema_result) {
    IndexEntry buffer;

    // Open the db.idx file
    char idx_path[PATH_MAX];
    snprintf(idx_path, sizeof(idx_path), "%s/db/db.idx", db_path);    
    FILE* idx_file = fopen(idx_path, "rb");    // idx is binary

    if (idx_file == NULL) {
        perror("Error opening db.idx file");
        return NULL;
    }

    UT_array *index_table_array = NULL;

    utarray_new(index_table_array, &structure_objects_array_icd);
    for (int i = 0; i < schema_result.length; i++) {
        StructureObjectsArray* soa = (StructureObjectsArray*) malloc(sizeof(StructureObjectsArray));
        if (soa == NULL) {
            perror("Failed to allocate structure array, low memory");
            return NULL;
        }
        soa->reserved_int = 0;  // Initialize reserved integer
        utarray_new(soa->objects, &loc_or_data_icd);  // Create array for LocOrData objects
        
        utarray_push_back(index_table_array, soa);
    }

    uint32_t object_loc = 0;

    uint32_t idx_loc = 0;

    while (fread(&buffer, sizeof(IndexEntry), 1, idx_file) == 1) {  // Read every index entry in the file
        StructureObjectsArray* soa = *(StructureObjectsArray**)utarray_eltptr(index_table_array, buffer.table_id);
        
        if (!soa) {
            // Ensure the outer array is big enough
            while (utarray_len(index_table_array) <= buffer.table_id) {
                StructureObjectsArray* empty_soa = (StructureObjectsArray*) malloc(sizeof(StructureObjectsArray));
                if (empty_soa == NULL) {
                    perror("Failed to allocate structure array, low memory");
                    return NULL;
                }
                empty_soa->reserved_int = 0;
                utarray_new(empty_soa->objects, &loc_or_data_icd);
                utarray_push_back(index_table_array, empty_soa);
            }
            
            soa = *(StructureObjectsArray**)utarray_eltptr(index_table_array, buffer.table_id);
            if (!soa) {
                perror("Failed to resize table array to include new table");
                return NULL;
            }
        }

        // Ensure the objects array is big enough for the object_id
        size_t required_size = buffer.object_id + 1;
        size_t current_size = utarray_len(soa->objects);
        
        if (required_size > current_size) {
            // Expand array with empty LocOrData slots
            size_t new_size = current_size == 0 ? 64 : current_size;
            while (new_size <= buffer.object_id) {
                new_size *= 2;  // double size each time until big enough
            }
            
            // Add empty slots with initialized LocOrData objects
            for (size_t i = current_size; i < new_size; i++) {
                LocOrData empty_lod;
                loc_or_data_init(&empty_lod);
                utarray_push_back(soa->objects, &empty_lod);
            }
        }

        // Get the LocOrData object at the specific object_id position
        LocOrData* lod_ptr = *(LocOrData**)utarray_eltptr(soa->objects, buffer.object_id);
        if (lod_ptr == NULL) {
            perror("Failed to load location or data at object id");
            return NULL;
        }
        // Set the type to TYPE_LOC since we're storing an IndexArrayEntry
        lod_ptr->type = TYPE_LOC;
        
        // Allocate a new IndexArrayEntry and set its values
        IndexArrayEntry* new_entry = malloc(sizeof(IndexArrayEntry));
        if (new_entry == NULL) {
            perror("Failed to allocate index array entry, low memory");
            return NULL;
        }
        new_entry->obj_loc = object_loc;
        new_entry->obj_size = buffer.size;
        new_entry->idx_loc = idx_loc;
        
        // Store the pointer in the union
        lod_ptr->idx_entry = new_entry;

        // Increment the reserved integer for next object
        soa->reserved_int++;
        
        object_loc += buffer.size; // Increment the object location by the size of the object

        idx_loc++;
    }

    fclose(idx_file);

    return index_table_array;
}

DBIndex index_db(const char* db_path) {
    // Scan the schema.flint file and extract all schema information
    // use the number of tables in the schema to create a utarray of each table 
    // to a utarray of objects inside the table

    // This means that finding an object table by id is an O(1) operation,
    // finding an object by id is an O(1) operation,
    // adding a new object is an O(1) operation,
    // when we want to remove an id, we set the location at that index to UINT32_MAX, marking it as deleted,
    // and then we add that id to a registry of available ids for reuse when we need to add a new object
    // We also have a seperate check program which can be run and makes sure that all 
    // empty id's are marked and the system is running nominally

    SchemaParserResult schema_result = load_schema(db_path);

    // Each entry in the db.obj has an id and structure
    // Hello World\x020
    // Which translates to:
    // str "Hello World",
    // int 0
    // Types are derived from the schema

    // Output format:
    // UT_ARRAY [
    //    StructureObjectsArray [
    //      reserved_int,
    //      UT_ARRAY [
    //        LocOrData { TYPE_LOC, IndexArrayEntry* { location_1, idx_1 } },
    //        LocOrData { TYPE_LOC, IndexArrayEntry* { location_2, idx_2 } },
    //      ]
    //    ]
    // ]

    UT_array* index_table_array = load_idx(db_path, schema_result);
    
    return make_db_index(schema_result.schema_table_array, index_table_array);
}