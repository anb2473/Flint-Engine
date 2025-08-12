#ifndef OBJ_LOCATION_ICD_H
#define OBJ_LOCATION_ICD_H

#include "../utils/utarray.h"
#include "../structs/obj-location.h"

UT_icd obj_location_icd = {
    sizeof(ObjLocation), // size of each element
    NULL,                 // init   (no special init needed)
    NULL,                 // copy   (bitwise copy is fine)
    NULL                  // dtor   (no special cleanup needed)
};

#endif