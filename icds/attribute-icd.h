#ifndef ATTRIBUTE_ICD_H
#define ATTRIBUTE_ICD_H

#include "../structs/attribute.h"
#include "../utils/utarray.h"

// Free function for Attribute
void attribute_free(Attribute* attr) {
    if (!attr) return;
    if (attr->name) free(attr->name);
    if (attr->properties) {
        utarray_free(attr->properties);
    }
}

// Copy function for UT_array (deep copy of nested UT_arrays)
void attribute_copy(void* dst, const void* src) {
    Attribute* d = (Attribute*)dst;
    const Attribute* s = (const Attribute*)src;
    d->name = s->name ? strdup(s->name) : NULL;
    d->type = s->type;

    if (s->properties) {
        utarray_new(d->properties, &attribute_icd);
        Attribute *p = NULL;
        while ((p = (Attribute*)utarray_next(s->properties, p))) {
            Attribute copy = {0};
            attribute_copy(&copy, p);
            utarray_push_back(d->properties, &copy);
        }
    } else {
        d->properties = NULL;
    }
}

void attribute_dtor(void* attr) {
    attribute_free((Attribute*)attr);
}

UT_icd attribute_icd = {
    sizeof(Attribute),
    NULL,
    attribute_copy,
    attribute_dtor
};

#endif