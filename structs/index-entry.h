#ifndef INDEX_ENTRY_H
#define INDEX_ENTRY_H

#include <stdint.h>

typedef struct {
    uint32_t object_id;
    uint8_t  table_id; // The id of the table structure (maximum 256 tables)
    uint16_t size; // The size of the object in bytes (maximum 65,535 bytes in a single object)
} IndexEntry;

#endif