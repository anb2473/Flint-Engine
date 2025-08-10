// Remove an object

#include "../utils/utarray.h"
#include <stdint.h>
#include <stdio.h>

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

typedef struct {
    uint32_t obj_loc;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct {
    uint32_t object_id;
    uint8_t  table_id; // The id of the table structure (maximum 256 tables)
    uint16_t size; // The size of the object in bytes (maximum 65,535 bytes in a single object)
} IndexEntry;

int remove_index_entry(const char* db_path, uint32_t idx_location) {
    char idx_filename[512];
    snprintf(idx_filename, sizeof(idx_filename), "%s/db/db.idx", db_path);

    char temp_filename[512];
    snprintf(temp_filename, sizeof(temp_filename), "%s/db/temp.idx", db_path);

    FILE* idx_file = fopen(idx_filename, "rb");
    FILE* temp_file = fopen(temp_filename, "wb");
    if (!idx_file || !temp_file) {
        perror("Error opening index files");
        if (idx_file) fclose(idx_file);
        if (temp_file) fclose(temp_file);
        return 1;
    }

    IndexEntry entry;
    uint32_t current_idx = 0;
    while (fread(&entry, sizeof(IndexEntry), 1, idx_file) == 1) {
        if (current_idx != idx_location) {
            fwrite(&entry, sizeof(IndexEntry), 1, temp_file);
        }
        current_idx++;
    }

    fclose(idx_file);
    fclose(temp_file);

    // Replace the original file with the modified temporary file
    if (remove(idx_filename) != 0) {
        perror("Error deleting original index file");
        return 1;
    }
    if (rename(temp_filename, idx_filename) != 0) {
        perror("Error renaming temporary index file");
        return 1;
    }
    return 0;
}

int rm_obj(char* db_path, DBIndex db_index, int obj_id, int obj_format_id) {
    UT_array* index_table_array = db_index.index_table_array;
    UT_array* table_obj_array = (UT_array*) utarray_eltptr(index_table_array, obj_format_id);
    IndexArrayEntry* entry = (IndexArrayEntry*) utarray_eltptr(table_obj_array, obj_id);
    IndexArrayEntry* next_entry = (IndexArrayEntry*) utarray_eltptr(table_obj_array, obj_id + 1);
    if (!entry || !next_entry) {
        fprintf(stderr, "Invalid object id or format id\n");
        return 1;
    }
    uint32_t obj_location = entry->obj_loc;
    uint32_t idx_location = entry->idx_loc;
    uint32_t next_obj_location = next_entry->obj_loc;

    FILE* temp_file = fopen(strcat(db_path, "/db/temp.obj"), "wb");

    if (temp_file == NULL) {
        perror("Error creating temp.obj file");
        fclose(temp_file);
        return 1;
    }

    char* filename = strcat(db_path, "/db/db.obj");

    FILE* obj_file = fopen(filename, "ab");

    if (obj_file == NULL) {
        perror("Error creating temp.obj file");
        fclose(obj_file);
        return 1;
    }

    long current_pos = 0;
    char ch;

    // Copy content before the removed object
    while ((ch = fgetc(obj_file)) != EOF && current_pos < obj_location) {
        fputc(ch, temp_file);
        current_pos++;
    }

    // Skip the removed object
    fseek(obj_file, next_obj_location, SEEK_SET);

    // Copy content after the removed object
    while ((ch = fgetc(obj_file)) != EOF) {
        fputc(ch, temp_file);
    }

    fclose(temp_file);
    fclose(obj_file);

    // Replace the original file with the modified temporary file
    if (remove(filename) != 0) {
        perror("Error deleting original file");
        return;
    }
    if (rename("temp.txt", filename) != 0) {
        perror("Error renaming temporary file");
    }

    remove_index_entry(db_path, idx_location);

    return 0;
}