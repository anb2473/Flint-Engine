#ifndef STRUCTURE_OBJECTS_ARRAY_H
#define STRUCTURE_OBJECTS_ARRAY_H

#include <stdint.h>
#include "../utils/utarray.h"

typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

#endif