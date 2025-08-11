// Initiate the database

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif

// Create /db directory, .env file, schema.flint file (declaring the tables), db.obj file (a list of every single objects data),
// db.idx file (a list of every single objects location with a fixed length for indexing)

int mk_dir(const char* path) {
    #ifdef _WIN32
        if (CreateDirectory(path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            return 0;
        } else {
            return 1;
        }
    #else
        if (mkdir(path, 0755) == 0 || errno == EEXIST) {
            return 0;
        } else {
            perror("mkdir");
            return 1;
        }
    #endif
}

int init(char* db_path) {
    // make /db directory
    char* db_dir = strcat(db_path, "/db");
    int ret_code = mk_dir(db_dir);
    if (ret_code != 0) {
        printf(stderr, "Failed to create database directory: %s\n", db_path);
        return ret_code;
    }

    FILE *fptr;
    fptr = fopen(strcat(db_dir, ".env"), "wb");

    FILE *fptr;
    fptr = fopen(strcat(db_dir, "schema.flint"), "wb");

    FILE *fptr;
    fptr = fopen(strcat(db_dir, "db.idx"), "wb");   // b is for binary mode (to avoid writing a full byte per character, e.g., 800 is 3 bytes)

    FILE *fptr;
    fptr = fopen(strcat(db_dir, "db.obj"), "wb");
    
    return 0;
}