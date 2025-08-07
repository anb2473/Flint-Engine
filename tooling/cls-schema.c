// Clear all tables

#include <stdio.h>

// Clear the schema file, removing all tables

int cls_obj(char* db_path) {
    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "w");

    if (schema_file == NULL) {
        perror("Failed to clear schema.flint file");
        return 1;
    }

    fclose(schema_file);

    return 0;
}