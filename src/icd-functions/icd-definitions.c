// This file contains the actual definitions of all ICD structures
// to prevent multiple definition errors during linking

#include "../../include/icds/obj-location-icd.h"
#include "../../include/icds/attribute-icd.h"
#include "../../include/icds/structure-objects-array-icd.h"
#include "../../include/icds/loc-or-data-icd.h"
#include "../../include/icds/table-structure-icd.h"
#include "../../include/icds/index-array-entry-icd.h"

// Define obj_location_icd
UT_icd obj_location_icd = {
    sizeof(ObjLocation), // size of each element
    NULL,                 // init   (no special init needed)
    NULL,                 // copy   (bitwise copy is fine)
    NULL                  // dtor   (no special cleanup needed)
};

// Define attribute_icd
UT_icd attribute_icd = {
    sizeof(Attribute),
    NULL,
    attribute_copy,
    attribute_dtor
};

// Define structure_objects_array_icd
UT_icd structure_objects_array_icd = {
    sizeof(StructureObjectsArray),
    NULL,  // No special init needed
    NULL,  // Default copy is fine
    structure_objects_array_dtor
};

// Define loc_or_data_icd
UT_icd loc_or_data_icd = {
    sizeof(LocOrData),
    loc_or_data_init,
    loc_or_data_copy,
    loc_or_data_dtor
};

// Define table_structure_icd
UT_icd table_structure_icd = {
    sizeof(TableStructure),
    table_structure_init,
    table_structure_copy,
    table_structure_dtor
};

// Define index_array_entry_icd
UT_icd index_array_entry_icd = {
    sizeof(IndexArrayEntry),
    NULL,  // No special init needed
    NULL,  // Default copy is fine
    NULL   // No special cleanup needed
};
