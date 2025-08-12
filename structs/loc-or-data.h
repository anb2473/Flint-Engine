#ifndef LOC_OR_DATA_H
#define LOC_OR_DATA_H

#include "../states/loc-or-data-state.h"

typedef struct {
    LocOrDataState type;
    union {
        IndexArrayEntry* idx_entry;
        DataMap *map;
    };
} LocOrData;

#endif