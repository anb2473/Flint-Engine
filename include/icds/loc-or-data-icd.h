#ifndef LOC_OR_DATA_ICD_H
#define LOC_OR_DATA_ICD_H

#include "../structs/loc-or-data.h"
#include "../structs/data-map.h"

void loc_or_data_init(void *elt);

// Deep copy element
void loc_or_data_copy(void *dst, const void *src);

// Destroy element
void loc_or_data_dtor(void *elt);

extern UT_icd loc_or_data_icd;

#endif