// Create a new table

#include <stdint.h>
#include "../../include/utils/utarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/structs/attribute.h"

// Include structs
#include "../../include/structs/db-index.h"
#include "../../include/structs/table-structure.h"

// Include icds
#include "../../include/icds/attribute-icd.h"

int mk_table(const char* db_path, DBIndex* db_index, char* table_name, UT_array* attributes) {
    TableStructure* table_structure = alloc_table_structure(); // Allocate memory for the table structure

    table_structure->name = strdup(table_name); // Allocate memory for the table name
    table_structure->attributes = attributes; // Assign the attributes to the table structure

    utarray_push_back(db_index->schema_table_array, table_structure);   // Add the table structure to the schema array

    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "ab"); // Open schema file in append mode
    if (schema_file == NULL) {
        perror("Error opening schema.flint file");
        return -1; // Return error code
    }

    // Build the attribute string
    char attribute_str[4096] = ""; // Use a fixed buffer instead of dynamic allocation
    for (int i = 0; i < utarray_len(attributes); i++) {
        Attribute* attr = (Attribute*)utarray_eltptr(attributes, i);
        if (i > 0) {
            strcat(attribute_str, ", ");
        }
        
        // Add type and name
        strcat(attribute_str, attr_type_to_str(attr->type));
        strcat(attribute_str, " *");
        strcat(attribute_str, attr->name);
        
        // Add properties if they exist
        if (attr->properties && utarray_len(attr->properties) > 0) {
            for (int j = 0; j < utarray_len(attr->properties); j++) {
                char* prop = (char*)utarray_eltptr(attr->properties, j);
                if (prop) {
                    strcat(attribute_str, " @");
                    strcat(attribute_str, prop);
                }
            }
        }
    }

    // Write the table definition to the schema file
    fprintf(schema_file, "%s {\n%s\n}\n", table_structure->name, attribute_str);
    
    fclose(schema_file);
    return 0;
}