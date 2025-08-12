#ifndef ATTRIBUTE_ICD_H
#define ATTRIBUTE_ICD_H

#include "../structs/attribute.h"

// Free function for Attribute
void attribute_free(Attribute* attr);

// Copy function for UT_array (deep copy of nested UT_arrays)
void attribute_copy(void* dst, const void* src);

void attribute_dtor(void* attr);

extern UT_icd attribute_icd;

#endif