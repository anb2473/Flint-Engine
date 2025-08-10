// Clear all objects

#include <stdio.h>
#include "../utils/utarray.h"
#include <stdint.h>

// Clear the idx and obj files, removing all objects

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

// Function to initialize a new slot in the outer array.
// It sets the UT_array* pointer to NULL.
static void inner_utarray_ptr_init(void *elt) {
    UT_array **ptr = (UT_array**)elt;
    *ptr = NULL;
}

// Function to destroy an element in the outer array.
// If the slot holds a pointer to an inner UT_array, free that inner UT_array.
static void inner_utarray_ptr_dtor(void *elt) {
    UT_array **ptr = (UT_array**)elt;
    if (*ptr != NULL) {
        utarray_free(*ptr); // Free the inner UT_array
        *ptr = NULL;        // Set the pointer to NULL for safety
    }
}

// The UT_icd for the outer array. Its elements are UT_array* (pointers).
static UT_icd outer_utarray_icd = {
    sizeof(UT_array*),      // Size of each element is the size of a pointer to UT_array
    inner_utarray_ptr_init, // Custom init function
    NULL,                   // Default copy (bitwise copy of pointer) is okay if not deep copying
    inner_utarray_ptr_dtor  // Custom dtor function
};

int cls_obj(char* db_path, DBIndex db_index) {
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

    utarray_new(db_index.index_table_array, &outer_utarray_icd);   // Clear index table array

    return 0;
}