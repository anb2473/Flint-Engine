#include "../../include/icds/loc-or-data-icd.h"
#include "../../include/structs/index-array-entry.h"

static void loc_or_data_init(void *elt) {
    LocOrData *lod = (LocOrData *)elt;
    lod->type = TYPE_LOC;
    IndexArrayEntry* idx_entry = (IndexArrayEntry*) malloc(sizeof(IndexArrayEntry));
    // Initialize as empty entry - doesn't point anywhere
    idx_entry->obj_loc = UINT32_MAX;  // Mark as empty object location
    idx_entry->obj_size = UINT16_MAX;   // Mark as empty object size
    idx_entry->idx_loc = UINT32_MAX;  // Mark as empty index location
    lod->idx_entry = idx_entry;
    lod->map = NULL; // union member
}

// Deep copy element
static void loc_or_data_copy(void *dst, const void *src) {
    const LocOrData *s = (const LocOrData *)src;
    LocOrData *d = (LocOrData *)dst;

    d->type = s->type;
    if (s->type == TYPE_LOC) {
        d->idx_entry = s->idx_entry;
    } else if (s->type == TYPE_MAP) {
        d->map = NULL;
        DataMap *curr, *tmp;
        HASH_ITER(hh, s->map, curr, tmp) {
            DataMap *new_entry = (DataMap*) malloc(sizeof(*new_entry));
            new_entry->name = strdup(curr->name);
            new_entry->data = curr->data;
            HASH_ADD_KEYPTR(hh, d->map, new_entry->name, strlen(new_entry->name), new_entry);
        }
    }
}

// Destroy element
static void loc_or_data_dtor(void *elt) {
    LocOrData *lod = (LocOrData *)elt;
    if (lod->type == TYPE_MAP) {
        DataMap *curr, *tmp;
        HASH_ITER(hh, lod->map, curr, tmp) {
            HASH_DEL(lod->map, curr);
            free(curr->name);
            free(curr->data);
            free(curr);
        }
        lod->map = NULL;
    }
}