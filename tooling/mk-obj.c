// Create a new object

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/utarray.h"
#include "../utils/uthash.h"
#include <stdio.h>

typedef struct
{
    char *key;
    char *value;
    UT_hash_handle hh;
} HashItem;

typedef enum
{
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

typedef struct {
    char* name;
    UT_array* attributes;
} TableStructure;

typedef struct
{
    char *name;
    AttributeType type;
    UT_array *properties; // Attributes for the property
} Attribute;

typedef struct {
    uint32_t obj_loc;
    uint16_t obj_size;
    uint32_t idx_loc;
} IndexArrayEntry;

// New struct to handle mixed data types in the inner arrays
typedef struct
{
    uint32_t reserved_int; // Reserved integer at index 0
    UT_array *objects;     // Array of IndexArrayEntry objects
} StructureObjectsArray;

// UT_icd for IndexArrayEntry
static UT_icd index_array_entry_icd = {
    sizeof(IndexArrayEntry), // Size of each element
    NULL,                    // No special init
    NULL,                    // No special copy
    NULL                     // No special destructor
};

typedef struct
{
    UT_array *schema_table_array;
    UT_array *index_table_array;
    UT_array *empty_indexes;
} DBIndex;

char *attr_type_to_str(AttributeType type)
{
    switch (type)
    {
    case TYPE_INT:
        return "int";
    case TYPE_STR:
        return "str";
    case TYPE_FLOAT:
        return "float";
    default:
        return "unknown";
    }
}

bool is_property(char *property_name, UT_array *properties)
{
    for (int i = 0; i < utarray_len(properties); i++)
    {
        if (utarray_eltptr(properties, i) == property_name)
        {
            return true;
        }
    }
    return false;
}

typedef struct
{
    uint32_t object_id;
    uint8_t table_id; // The id of the table structure (maximum 256 tables)
    uint16_t size;    // The size of the object in bytes (maximum 65,535 bytes in a single object)
} IndexEntry;

typedef struct {
    uint32_t obj_loc;
    uint16_t obj_size;
    uint32_t idx_loc;
} IndexArrayEntry;

typedef struct
{
    uint32_t obj_id;
    uint32_t obj_format_id;
} ObjLocation;

int mk_obj(const char *db_path, DBIndex *db_index, uint32_t structure_id, HashItem *data)
{
    // Data will be given as a hashmap
    // Data must be stored as an array
    // Cross check each value in the hashmap with the schema to find the correct position

    // Create a char representation of the object, find its length,
    // and then write its metadata to the db.idx and its raw data to the db.obj

    // Char representation format should look like:
    // sHello World\x02i0

    char *obj_data = "";

    UT_array *attributes = ((TableStructure *)utarray_eltptr(db_index->schema_table_array, structure_id))->attributes;
    size_t attr_len = utarray_len(attributes);

    for (int i = 0; i < attr_len; i++)
    {
        Attribute *attr = (Attribute *)utarray_eltptr(attributes, i);
        HashItem *item;
        HASH_FIND_STR(data, attr->name, item);
        if (item)
        {
            obj_data = strcat(obj_data, item->value);
            if (i != attr_len - 1)
            { // if not last attr, add a seperator
                obj_data = strcat(obj_data, "\x01");
            }
        }
        else
        {
            if (is_property("nullable", attr->properties))
            { // Check if property is nullable
                obj_data = strcat(obj_data, "\x02");
            }
            else
            {
                perror("Nulled non nullable value");
                return 1;
            }
        }
    }

    FILE* obj_file = fopen(strcat(db_path, "/db/db.obj"), "ab");
    fwrite(obj_data, strlen(obj_data), 1, obj_file);
    
    // Get file length
    fseek(obj_file, 0, SEEK_END); // Move file pointer to the end
    long obj_file_len = ftell(obj_file);  // Get current position (which is the size)

    fclose(obj_file);
    free(obj_file);

    FILE *idx_file = fopen(strcat(db_path, "/db/db.idx"), "ab");

    // Number of items in the structure array + 1 is the id of the new object
    StructureObjectsArray *soa = (StructureObjectsArray *)utarray_eltptr(db_index->index_table_array, structure_id);

    // If the StructureObjectsArray doesn't exist, create it
    if (!soa)
    {
        // Ensure the outer array is big enough
        while (utarray_len(db_index->index_table_array) <= structure_id)
        {
            StructureObjectsArray empty_soa;
            empty_soa.reserved_int = 0;
            utarray_new(empty_soa.objects, &index_array_entry_icd);
            utarray_push_back(db_index->index_table_array, &empty_soa);
        }
        soa = (StructureObjectsArray *)utarray_eltptr(db_index->index_table_array, structure_id);
    }

    int new_object_id = soa->reserved_int + 1;

    IndexEntry entry = {
        .object_id = new_object_id,
        .table_id = structure_id,
        .size = strlen(obj_data),
    };

    fwrite(&entry, sizeof(IndexEntry), 1, idx_file);
    fclose(idx_file);
    free(idx_file);

    // Ensure the objects array is big enough for the new object_id
    size_t required_size = new_object_id + 1;
    size_t current_size = utarray_len(soa->objects);

    if (required_size > current_size)
    {
        // Expand array with UINT32_MAX empty slots
        size_t new_size = current_size == 0 ? 64 : current_size;
        while (new_size <= new_object_id)
        {
            new_size *= 2; // double size each time until big enough
        }

        // Add empty slots with UINT32_MAX values
        IndexArrayEntry empty_entry = {UINT32_MAX, UINT16_MAX, UINT32_MAX};
        for (size_t i = current_size; i < new_size; i++)
        {
            utarray_push_back(soa->objects, &empty_entry);
        }
    }

    // Check if we can reuse an empty slot
    if (utarray_len(db_index->empty_indexes) == 0)
    {
        // Create new object in array at the specific position
        IndexArrayEntry *entry_ptr = (IndexArrayEntry *)utarray_eltptr(soa->objects, new_object_id);
        if (entry_ptr)
        {
            entry_ptr->obj_loc = obj_file_len;
            entry_ptr->idx_loc = new_object_id;
        }

        soa->reserved_int = new_object_id; // Update the reserved integer
        return 0;
    }

    // Try to reuse an empty slot
    size_t num_of_empty_indexes = utarray_len(db_index->empty_indexes);
    if (num_of_empty_indexes > 0)
    {
        ObjLocation *empty_slot_location_ptr = (ObjLocation *)utarray_eltptr(db_index->empty_indexes, num_of_empty_indexes - 1);
        ObjLocation empty_slot_location = *empty_slot_location_ptr;
        utarray_pop_back(db_index->empty_indexes);

        StructureObjectsArray *target_soa = (StructureObjectsArray *)utarray_eltptr(db_index->index_table_array, empty_slot_location.obj_format_id);

        if (target_soa && target_soa->objects)
        {
            // Ensure the target array is big enough
            size_t target_required_size = empty_slot_location.obj_id + 1;
            size_t target_current_size = utarray_len(target_soa->objects);

            if (target_required_size > target_current_size)
            {
                size_t target_new_size = target_current_size == 0 ? 64 : target_current_size;
                while (target_new_size <= empty_slot_location.obj_id)
                {
                    target_new_size *= 2;
                }

                IndexArrayEntry empty_entry = {UINT32_MAX, UINT32_MAX};
                for (size_t i = target_current_size; i < target_new_size; i++)
                {
                    utarray_push_back(target_soa->objects, &empty_entry);
                }
            }

            // Set the object at the empty slot position
            IndexArrayEntry *target_entry_ptr = (IndexArrayEntry *)utarray_eltptr(target_soa->objects, empty_slot_location.obj_id);
            if (target_entry_ptr)
            {
                target_entry_ptr->obj_loc = obj_file_len;
                target_entry_ptr->idx_loc = new_object_id;
            }

            return 0;
        }
    }

    // Fallback: create new object in the original structure
    IndexArrayEntry *entry_ptr = (IndexArrayEntry *)utarray_eltptr(soa->objects, new_object_id);
    if (entry_ptr)
    {
        entry_ptr->obj_loc = obj_file_len;
        entry_ptr->obj_size = strlen(obj_data);
        entry_ptr->idx_loc = new_object_id;
    }

    soa->reserved_int = new_object_id; // Update the reserved integer
    return 0;
}