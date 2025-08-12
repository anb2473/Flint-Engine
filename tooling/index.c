// Index database

#include <stdio.h>
#include <stdint.h>
#include <ctype.h> 
#include "../utils/utarray.h"
#include "../utils/uthash.h"

// Include structs
#include "../structs/db-index.h"
#include "../structs/index-array-entry.h"
#include "../structs/structure-objects-array.h"
#include "../structs/index-entry.h"
#include "../structs/data-map.h"
#include "../structs/attribute.h"
#include "../structs/table-structure.h"
#include "../structs/loc-or-data.h"
#include "../structs/obj-location.h"

// Include icds
#include "../icds/index-array-entry-icd.h"
#include "../icds/structure-objects-array-icd.h"
#include "../icds/attribute-icd.h"
#include "../icds/loc-or-data-icd.h"
#include "../icds/table-structure-icd.h"
#include "../icds/obj-location-icd.h"

// Include states
#include "../states/schema-scanner-state.h"

// Go through every schema in the schema.flint file and every object in the db.idx file, and store their data for future use
// Whenever a request for an object is made, load its contents from its location and then add it to the FILO cache

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

    // Open the db.idx file

    int read_byte_as_int; // Use 'int' for fgetc's return value to check for EOF
    unsigned char current_byte; // Use unsigned char for the actual byte value

    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "rb");

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

    char* name = ""; // Buffer to hold the name of the structure
    char* attribute_type = "";
    char* attribute_name = "";
    char* property = "";

    Attribute* current_attribute = NULL; // Pointer to the current attribute being processed

    uint32_t schema_table_array_len = 0;

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
                name += current_byte;
            case INSIDE_TYPE:
                // one case: type *name
                if (current_byte == '*') {
                    current_attribute = alloc_attr(); // Allocate memory for the current attribute
                    current_attribute->type = parse_attribute_type(attribute_type); // Parse the attribute type
                    state = INSIDE_NAME; // Transition to inside name
                    continue;
                }
                attribute_type += current_byte;
            case INSIDE_NAME:
                // three cases, name @property, name, name {
                if (current_byte == '@') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    state = INSIDE_ATTR; // Transition to inside attribute
                    continue;
                }
                if (current_byte == ',') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_table_array_len += 1;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                attribute_name += current_byte;
            case INSIDE_ATTR:
                // three cases: @attr @attr, @attr {, @attr,
                if (current_byte == '@') {
                    create_attribute(property, current_attribute->properties);
                    state = INSIDE_ATTR; // Transition to inside attribute
                    continue;
                }
                if (current_byte == ',') {
                    create_attribute(property, current_attribute->properties);
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value                    
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    create_attribute(property, current_attribute->properties);
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_table_array_len += 1;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                property += current_byte;
        }
    }

    if (schema_file == NULL) {
        perror("Error opening schema.flint file");
        DBIndex error_result = {NULL, NULL};
        return error_result;
    }

    // Each entry in the db.obj has an id and structure
    // Hello World\x020
    // Which translates to:
    // str "Hello World",
    // int 0
    // Types are derived from the schema

    IndexEntry buffer;

    // Open the db.idx file
    FILE* idx_file = fopen(strcat(db_path, "/db/db.idx"), "rb");    // idx is binary

    if (idx_file == NULL) {
        perror("Error opening db.idx file");
        DBIndex error_result = {NULL, NULL};
        return error_result;
    }

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

    UT_array *index_table_array;

    utarray_new(index_table_array, &structure_objects_array_icd);
    for (int i = 0; i < schema_table_array_len; i++) {
        StructureObjectsArray soa;
        soa.reserved_int = 0;  // Initialize reserved integer
        utarray_new(soa.objects, &loc_or_data_icd);  // Create array for LocOrData objects
        
        utarray_push_back(index_table_array, &soa);
    }

    uint32_t object_loc = 0;

    uint32_t idx_loc = 0;

    while (fread(&buffer, sizeof(IndexEntry), 1, idx_file) == 1) {  // Read every index entry in the file
        StructureObjectsArray* soa = (StructureObjectsArray*)utarray_eltptr(index_table_array, buffer.table_id);
        
        if (!soa) {
            // Create a new StructureObjectsArray if it doesn't exist
            StructureObjectsArray new_soa;
            new_soa.reserved_int = 0;
            utarray_new(new_soa.objects, &loc_or_data_icd);
            
            // Ensure the outer array is big enough
            while (utarray_len(index_table_array) <= buffer.table_id) {
                StructureObjectsArray empty_soa;
                empty_soa.reserved_int = 0;
                utarray_new(empty_soa.objects, &loc_or_data_icd);
                utarray_push_back(index_table_array, &empty_soa);
            }
            
            soa = (StructureObjectsArray*)utarray_eltptr(index_table_array, buffer.table_id);
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

        // Get the reserved integer from the StructureObjectsArray
        uint32_t id = soa->reserved_int;

        // Get the LocOrData object at the specific object_id position
        LocOrData* lod_ptr = (LocOrData*)utarray_eltptr(soa->objects, buffer.object_id);
        if (lod_ptr) {
            // Set the type to TYPE_LOC since we're storing an IndexArrayEntry
            lod_ptr->type = TYPE_LOC;
            
            // Allocate a new IndexArrayEntry and set its values
            IndexArrayEntry* new_entry = malloc(sizeof(IndexArrayEntry));
            new_entry->obj_loc = object_loc;
            new_entry->obj_size = buffer.size;
            new_entry->idx_loc = idx_loc;
            
            // Store the pointer in the union
            lod_ptr->idx_entry = new_entry;
        }

        // Increment the reserved integer for next object
        soa->reserved_int++;
        
        object_loc += buffer.size; // Increment the object location by the size of the object

        idx_loc++;
    }
    
    return make_db_index(schema_table_array, index_table_array);
}