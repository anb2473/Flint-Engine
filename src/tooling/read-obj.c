// Read object from index array
// If only a location is provided, load data from obj file to array
// Otherwise load data directly from array

// \x01 is a seperator, \x02 is a null value, \x03 is a marker that that data region is empty

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/utils/utarray.h"
#include "../../include/utils/uthash.h"

// Include structs
#include "../../include/structs/db-index.h"
#include "../../include/structs/index-array-entry.h"
#include "../../include/structs/structure-objects-array.h"
#include "../../include/structs/data-map.h"
#include "../../include/structs/attribute.h"
#include "../../include/structs/table-structure.h"
#include "../../include/structs/loc-or-data.h"

// Include icds
#include "../../include/icds/attribute-icd.h"

DataMap* read_obj(const char* db_path, DBIndex* db_index, uint32_t obj_id, uint32_t table_id) {
    StructureObjectsArray* soa = utarray_eltptr(db_index->index_table_array, table_id);
    if (!soa) {
        fprintf(stderr, "Table not found\n");
        return NULL;
    }

    LocOrData* entry = utarray_eltptr(soa->objects, obj_id);
    if (!entry) {
        fprintf(stderr, "Object not found\n");
        return NULL;
    }

    if (entry->type == TYPE_LOC) {
        IndexArrayEntry* entry_loc = entry->idx_entry;
        if (entry_loc->obj_loc == UINT32_MAX && entry_loc->idx_loc == UINT32_MAX) {
            fprintf(stderr, "Object location is empty\n");
            return NULL;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/db/db.obj", db_path);
        FILE* obj_file = fopen(path, "rb");
        if (!obj_file) {
            perror("Failed to open object file");
            return NULL;
        }

        if (fseek(obj_file, entry_loc->obj_loc, SEEK_SET) != 0) {
            perror("Seek failed");
            fclose(obj_file);
            return NULL;
        }

        DataMap* data = NULL; // empty hash table

        TableStructure* table_schema = utarray_eltptr(db_index->schema_table_array, table_id);
        UT_array* attributes = table_schema->attributes;

        char buffer[256];
        size_t buf_len = 0;
        uint32_t current_attr_index = 0;

        for (uint32_t pos = 0; pos < entry_loc->obj_size; pos++) {
            int ch = fgetc(obj_file);
            if (ch == EOF) break;

            if (ch == '\x01') { // separator
                buffer[buf_len] = '\0'; // Terminate string safely
                Attribute* current_attribute = utarray_eltptr(attributes, current_attr_index);
                AttributeValue val = parse_str_to_val(buffer, current_attribute->type);
                insert_data(&data, current_attribute->name, &val);
                if (val.type == TYPE_STR) free(val.value.s); // cleanup after copy
                buf_len = 0;
                current_attr_index++;
            } else {
                if (buf_len < sizeof(buffer) - 1) {
                    buffer[buf_len++] = (char)ch;
                }
            }
        }

        fclose(obj_file);
        return data;
    }
    else if (entry->type == TYPE_MAP) {
        return entry->map;
    }

    return NULL;
}
