// Remove an object

#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"
#include <stdint.h>
#include <stdio.h>

// Include structs
#include "../structs/db-index.h"
#include "../structs/index-array-entry.h"
#include "../structs/structure-objects-array.h"
#include "../structs/index-entry.h"
#include "../structs/obj-location.h"

// Include icds
#include "../icds/index-array-entry-icd.h"

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
    StructureObjectsArray* soa = (StructureObjectsArray*)utarray_eltptr(index_table_array, obj_format_id);
    
    if (!soa || !soa->objects) {
        fprintf(stderr, "Invalid object id or format id\n");
        return 1;
    }
    
    IndexArrayEntry* entry = (IndexArrayEntry*)utarray_eltptr(soa->objects, obj_id);
    IndexArrayEntry* next_entry = (IndexArrayEntry*)utarray_eltptr(soa->objects, obj_id + 1);
    // Check if a next entry exists, if it does not we can directly remove the object,
    // otherwise we need to replace the object with empty characters
    
    if (!entry) {
        fprintf(stderr, "Invalid object id or format id\n");
        return 1;
    }
    
    if (!next_entry) {  // Entry is the last in the file, no next entry
        // We can directly remove the object as it won't cause any conflicts with other object locations
        uint32_t obj_location = entry->obj_loc;
        uint32_t idx_location = entry->idx_loc;
        ObjLocation loc = {
            .obj_id = obj_id,
            .obj_format_id = obj_format_id
        };
        utarray_push_back(db_index.empty_indexes, &loc);

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

        // No content after removed object (object at end of file)

        fclose(temp_file);
        fclose(obj_file);

        // Replace the original file with the modified temporary file
        if (remove(filename) != 0) {
            perror("Error deleting original file");
            return 1;
        }
        if (rename("temp.txt", filename) != 0) {
            perror("Error renaming temporary file");
            return 1;
        }

        remove_index_entry(db_path, idx_location);

        return 0;
    }
    // If the object is not the last in the file we cannot directly remove it, instead we need to clear the data at it's location
    uint32_t obj_location = entry->obj_loc;
    uint16_t obj_size = entry->obj_size;
    uint32_t idx_location = entry->idx_loc;
    ObjLocation loc = {
        .obj_id = obj_id,
        .obj_format_id = obj_format_id
    };
    utarray_push_back(db_index.empty_indexes, &loc);

    char* filename = strcat(db_path, "/db/db.obj");

    FILE* obj_file = fopen(filename, "ab");

    if (obj_file == NULL) {
        perror("Error creating temp.obj file");
        fclose(obj_file);
        return 1;
    }

    long current_pos = 0;
    char ch;

    fseek(obj_file, obj_location, SEEK_SET);

    // Null the removed object by replacing it with \x03 empty characters
    while (current_pos < obj_size) {
        fputc("\x03", obj_file);
        current_pos++;
    }

    fclose(obj_file);

    remove_index_entry(db_path, idx_location);

    return 0;
}