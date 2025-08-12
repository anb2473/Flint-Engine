// Initiate the database

#include <stdio.h>
#include <string.h>

// Include header file to expose functions
#include "../../include/tooling/init.h"

// Include limits for IO operations
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include "../../include/io-utils/mkdir.h"

// Create /db directory, .env file, schema.flint file (declaring the tables), db.obj file (a list of every single objects data),
// db.idx file (a list of every single objects location with a fixed length for indexing)

int init(char* db_path) {
    // make /db directory
    char db_dir[PATH_MAX];
    snprintf(db_dir, sizeof(db_dir), "%s/db", db_path);
    int ret_code = mk_dir(db_dir);
    if (ret_code != 0) {
        perror("Failed to create database directory");
        return ret_code;
    }

    FILE *env_fptr;
    char env_path[PATH_MAX];
    snprintf(env_path, sizeof(env_path), "%s/db.env", db_dir);
    env_fptr = fopen(env_path, "wb");
    if (env_fptr == NULL) {
        perror("Failed to create .env file");
        return 1;
    }
    fclose(env_fptr);

    FILE *schema_fptr;
    char schema_file[PATH_MAX];
    snprintf(schema_file, sizeof(schema_file), "%s/schema.flint", db_dir);
    schema_fptr = fopen(schema_file, "wb");
    if (schema_fptr == NULL) {
        perror("Failed to create schema.flint file");
        return 1;
    }
    fclose(schema_fptr);

    FILE *idx_fptr;
    char idx_path[PATH_MAX];
    snprintf(idx_path, sizeof(idx_path), "%s/db.idx", db_dir);
    idx_fptr = fopen(idx_path, "wb");   // b is for binary mode (to avoid writing a full byte per character, e.g., 800 is 3 bytes)
    if (idx_fptr == NULL) {
        perror("Failed to create db.idx file");
        return 1;
    }
    fclose(idx_fptr);

    FILE *obj_fptr;
    char obj_path[PATH_MAX];
    snprintf(obj_path, sizeof(obj_path), "%s/db.obj", db_dir);
    obj_fptr = fopen(obj_path, "wb");
    if (obj_fptr == NULL) {
        perror("Failed to create db.obj file");
        return 1;
    }
    fclose(obj_fptr);
    
    return 0;
}