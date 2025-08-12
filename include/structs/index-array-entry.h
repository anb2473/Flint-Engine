#ifndef INDEX_ARRAY_ENTRY_H
#define INDEX_ARRAY_ENTRY_H

#include <stdint.h>

typedef struct {
    uint32_t obj_loc;
    uint16_t obj_size;
    uint32_t idx_loc;
} IndexArrayEntry;

#endif