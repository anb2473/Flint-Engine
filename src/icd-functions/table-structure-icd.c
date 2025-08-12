#include "../../include/icds/table-structure-icd.h"

// Init function for TableStructure
void table_structure_init(void *p) {
    TableStructure *ts = (TableStructure*)p;
    ts->name = NULL;
    ts->attributes = NULL;
}

// Copy function (deep copy of name and attributes)
void table_structure_copy(void *dst, const void *src) {
    TableStructure *d = (TableStructure*)dst;
    const TableStructure *s = (const TableStructure*)src;

    d->name = s->name ? strdup(s->name) : NULL;

    if (s->attributes) {
        utarray_new(d->attributes, &attribute_icd);
        Attribute *a = NULL;
        while ((a = (Attribute*)utarray_next(s->attributes, a))) {
            Attribute copy = {0};
            attribute_copy(&copy, a);
            utarray_push_back(d->attributes, &copy);
        }
    } else {
        d->attributes = NULL;
    }
}

// Destructor (free name and attributes)
void table_structure_dtor(void *p) {
    TableStructure *ts = (TableStructure*)p;
    if (ts->name) free(ts->name);
    if (ts->attributes) {
        utarray_free(ts->attributes);
        ts->attributes = NULL;
    }
}