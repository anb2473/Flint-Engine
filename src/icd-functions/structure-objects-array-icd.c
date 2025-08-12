#include "../../include/icds/structure-objects-array-icd.h"

// Custom destructor for StructureObjectsArray
void structure_objects_array_dtor(void* elt) {
    StructureObjectsArray* soa = (StructureObjectsArray*)elt;
    if (soa->objects) {
        utarray_free(soa->objects);
        soa->objects = NULL;
    }
}