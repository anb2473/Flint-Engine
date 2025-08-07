// Clear all objects

#include <stdio.h>

// Clear the idx and obj files, removing all objects

int cls_obj(char* db_path) {
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

    return 0;
}