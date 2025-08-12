#ifndef INDEX_ARRAY_ENTRY_ICD_H
#define INDEX_ARRAY_ENTRY_ICD_H

#include "../structs/index-array-entry.h"
#include "../utils/utarray.h"

// UT_icd for IndexArrayEntry
static UT_icd index_array_entry_icd = {
    sizeof(IndexArrayEntry), // Size of each element
    NULL,                    // No special init
    NULL,                    // No special copy
    NULL                     // No special destructor
};

#endif