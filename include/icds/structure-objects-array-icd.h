#ifndef STRUCTURE_OBJECTS_ARRAY_ICD_H
#define STRUCTURE_OBJECTS_ARRAY_ICD_H

#include "../utils/utarray.h"
#include "../structs/structure-objects-array.h"

// UT_icd for StructureObjectsArray
UT_icd structure_objects_array_icd = {
    sizeof(StructureObjectsArray),
    NULL,  // No special init needed
    NULL,  // Default copy is fine
    structure_objects_array_dtor
};

// Custom destructor for StructureObjectsArray
void structure_objects_array_dtor(void* elt) {
    StructureObjectsArray* soa = (StructureObjectsArray*)elt;
    if (soa->objects) {
        utarray_free(soa->objects);
        soa->objects = NULL;
    }
}

#endif