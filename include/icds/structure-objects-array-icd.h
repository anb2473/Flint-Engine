#ifndef STRUCTURE_OBJECTS_ARRAY_ICD_H
#define STRUCTURE_OBJECTS_ARRAY_ICD_H

#include "../utils/utarray.h"
#include "../structs/structure-objects-array.h"

// Custom destructor for StructureObjectsArray
void structure_objects_array_dtor(void* elt);

// UT_icd for StructureObjectsArray
extern UT_icd structure_objects_array_icd;

#endif