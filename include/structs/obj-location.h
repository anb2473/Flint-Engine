#ifndef OBJ_LOCATION_H
#define OBJ_LOCATION_H

#include <stdint.h>

typedef struct {
    uint32_t obj_id;
    uint32_t obj_format_id;
} ObjLocation;

#endif