// Index database

#include <stdio.h>
#include <stdint.h>
#include <ctype.h> 
#include "../utils/utarray.h"
#include "../utils/uthash.h"

// Go through every schema in the schema.flint file and every object in the db.idx file, and store their data for future use
// Whenever a request for an object is made, load its contents from its location and then add it to the FILO cache

typedef struct {
    uint32_t object_id;
    uint8_t  table_id; // The id of the table structure (maximum 256 tables)
    uint16_t size; // The size of the object in bytes (maximum 65,535 bytes in a single object)
} IndexEntry;

// define the datatype for the inner utarray (uint32)
// No special init, copy, or dtor functions are needed for scalar types like uint32_t.
// Define the UT_icd for IndexArrayEntry for use in inner UT_arrays
static UT_icd index_array_entry_icd = {
    sizeof(IndexArrayEntry), // Size of each element
    NULL,                    // No special init
    NULL,                    // No special copy
    NULL                     // No special destructor
};

// Function to initialize a new slot in the outer array.
// It sets the UT_array* pointer to NULL.
static void inner_utarray_ptr_init(void *elt) {
    UT_array **ptr = (UT_array**)elt;
    *ptr = NULL;
}

// Function to destroy an element in the outer array.
// If the slot holds a pointer to an inner UT_array, free that inner UT_array.
static void inner_utarray_ptr_dtor(void *elt) {
    UT_array **ptr = (UT_array**)elt;
    if (*ptr != NULL) {
        utarray_free(*ptr); // Free the inner UT_array
        *ptr = NULL;        // Set the pointer to NULL for safety
    }
}

// The UT_icd for the outer array. Its elements are UT_array* (pointers).
static UT_icd structure_utarry_icd = {
    sizeof(UT_array*),      // Size of each element is the size of a pointer to UT_array
    inner_utarray_ptr_init, // Custom init function
    NULL,                   // Default copy (bitwise copy of pointer) is okay if not deep copying
    inner_utarray_ptr_dtor  // Custom dtor function
};

// Define an enum to represent the state in the schema scanner
typedef enum {
    OUTSIDE_STRUCTURE,
    INSIDE_TYPE,
    INSIDE_NAME,
    INSIDE_ATTR,
} SchemaScannerState;

typedef enum {
    TYPE_INT,
    TYPE_STR,
    TYPE_FLOAT,
} AttributeType;

typedef struct {
    char* name;
    UT_array* attributes
} TableStructure;

typedef struct {
    char* name;
    AttributeType type;
    UT_array* properties; // Attributes for the property
} Attribute;

AttributeType parse_attribute_type(const char* type_str) {
    if (strcmp(type_str, "int") == 0) {
        return TYPE_INT;
    } else if (strcmp(type_str, "str") == 0) {
        return TYPE_STR;
    } else if (strcmp(type_str, "float") == 0) {
        return TYPE_FLOAT;
    }
    return -1; // Invalid type
}

static UT_icd char_icd = {sizeof(char), NULL, NULL, NULL};

// Custom destructor for Attribute
void attribute_free(Attribute* attr) {
    if (!attr) return;
    free(attr->name);

    if (attr->properties) {
        Attribute* prop = NULL;
        while ((prop = (Attribute*)utarray_next(attr->properties, prop))) {
            attribute_free(prop);
        }
        utarray_free(attr->properties);
    }

    free(attr);
}

// Copy function for UT_array (deep copy for nested UT_arrays)
void attribute_copy(void* dst, const void* src) {
    Attribute* d = (Attribute*)dst;
    const Attribute* s = (const Attribute*)src;
    d->name = strdup(s->name);
    d->type = s->type;

    if (s->properties) {
        utarray_new(d->properties, &attribute_icd);
        Attribute* p = NULL;
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

Attribute* create_attribute(const char* name, AttributeType type) {
    Attribute* attr = (Attribute*)malloc(sizeof(Attribute));
    attr->name = strdup(name);
    attr->type = type;
    attr->properties = NULL;
    return attr;
}

void print_attribute(const Attribute* attr, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Attribute: %s (type=%d)\n", attr->name, attr->type);
    if (attr->properties) {
        Attribute* sub = NULL;
        while ((sub = (Attribute*)utarray_next(attr->properties, sub))) {
            print_attribute(sub, indent + 1);
        }
    }
}

TableStructure* alloc_table_structure() {
    TableStructure *table_structure = malloc(sizeof(TableStructure));
    utarray_new(table_structure->attributes, &attribute_icd);
    return table_structure;
}

Attribute* alloc_attr() {
    Attribute *attr = malloc(sizeof(Attribute));
    utarray_new(attr->properties, &char_icd); // Load UT_array to properties
    return attr;
}

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

// UT_icd for TableStructure
UT_icd table_structure_icd = {
    sizeof(TableStructure),
    table_structure_init,
    table_structure_copy,
    table_structure_dtor
};

typedef struct {
    uint32_t obj_loc;
    uint32_t idx_loc;
} IndexArrayEntry;

// New struct to handle mixed data types in the inner arrays
typedef struct {
    uint32_t reserved_int;  // Reserved integer at index 0
    UT_array* objects;      // Array of IndexArrayEntry objects
} StructureObjectsArray;

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

typedef struct {
    uint32_t obj_id;
    uint32_t obj_format_id;
} ObjLocation;

typedef enum {
    TYPE_LOC,
    TYPE_MAP,
} LocOrDataType;

typedef struct {
    char* name;                 // key
    char* data;
    UT_hash_handle hh;      // makes this hashable
} Data;

typedef struct {
    LocOrDataType type;
    union {
        IndexArrayEntry* idx_entry;
        Data *map;
    };
} LocOrData;

static void loc_or_data_init(void *elt) {
    LocOrData *lod = (LocOrData *)elt;
    lod->type = TYPE_INT;
    IndexArrayEntry* idx_entry = malloc(sizeof(IndexArrayEntry));
    // Initialize as empty entry - doesn't point anywhere
    idx_entry->obj_loc = UINT32_MAX;  // Mark as empty/invalid object location
    idx_entry->idx_loc = UINT32_MAX;  // Mark as empty/invalid index location
    lod->idx_entry = idx_entry;
    lod->map = NULL; // union member
}

// Deep copy element
static void loc_or_data_copy(void *dst, const void *src) {
    const LocOrData *s = (const LocOrData *)src;
    LocOrData *d = (LocOrData *)dst;

    d->type = s->type;
    if (s->type == TYPE_INT) {
        d->idx_entry = s->idx_entry;
    } else if (s->type == TYPE_MAP) {
        d->map = NULL;
        Data *curr, *tmp;
        HASH_ITER(hh, s->map, curr, tmp) {
            Data *new_entry = malloc(sizeof(*new_entry));
            new_entry->name = strdup(curr->name);
            new_entry->data = strdup(curr->data);
            HASH_ADD_KEYPTR(hh, d->map, new_entry->name, strlen(new_entry->name), new_entry);
        }
    }
}

// Destroy element
static void loc_or_data_dtor(void *elt) {
    LocOrData *lod = (LocOrData *)elt;
    if (lod->type == TYPE_MAP) {
        Data *curr, *tmp;
        HASH_ITER(hh, lod->map, curr, tmp) {
            HASH_DEL(lod->map, curr);
            free(curr->name);
            free(curr->data);
            free(curr);
        }
        lod->map = NULL;
    }
}

/* --- The UT_icd definition --- */
UT_icd loc_or_data_icd = {
    sizeof(LocOrData),
    loc_or_data_init,
    loc_or_data_copy,
    loc_or_data_dtor
};

UT_icd ObjLocation_icd = {
    sizeof(ObjLocation), // size of each element
    NULL,                 // init   (no special init needed)
    NULL,                 // copy   (bitwise copy is fine)
    NULL                  // dtor   (no special cleanup needed)
};

DBIndex make_db_index(UT_array* schema_table_array, UT_array* index_table_array) {
    DBIndex result;
    result.schema_table_array = schema_table_array;
    result.index_table_array = index_table_array;
    utarray_new(result.empty_indexes, &ObjLocation_icd);
    return result;
}

DBIndex index_db(const char* db_path) {
    // Scan the schema.flint file and extract all schema information
    // use the number of tables in the schema to create a utarray of each table 
    // to a utarray of objects inside the table

    // This means that finding an object table by id is an O(1) operation,
    // finding an object by id is an O(1) operation,
    // adding a new object is an O(1) operation,
    // when we want to remove an id, we set the location at that index to UINT32_MAX, marking it as deleted,
    // and then we add that id to a registry of available ids for reuse when we need to add a new object
    // We also have a seperate check program which can be run and makes sure that all 
    // empty id's are marked and the system is running nominally

    // Open the db.idx file

    int read_byte_as_int; // Use 'int' for fgetc's return value to check for EOF
    unsigned char current_byte; // Use unsigned char for the actual byte value

    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "rb");

    // Each entry in the schema has a name and a structure
    // name {
    //    str *property_name @property_attr,
    //    int *property_name
    // }

    // Which would be stored as:
    // Structure [
    //     name, UT_ARRAY [
    //       Attribute [
    //          str, property_name, UT_ARRAY [ property_attr ]
    //       ] 
    //       Attribute [
    //          int, property_name, UT_ARRAY []
    //       ]
    //    ]
    // ]

    UT_array* schema_table_array;
    utarray_new(schema_table_array, &table_structure_icd); // Create a new outer UT_array for table structures

    TableStructure* table_structure = alloc_table_structure(); // Allocate memory for the table structure

    SchemaScannerState state = OUTSIDE_STRUCTURE; // Initial state

    char* name = ""; // Buffer to hold the name of the structure
    char* attribute_type = "";
    char* attribute_name = "";
    char* property = "";

    Attribute* current_attribute = NULL; // Pointer to the current attribute being processed

    uint32_t schema_table_array_len = 0;

    while ((read_byte_as_int = fgetc(schema_file)) != EOF) {    // Read until end of file
        // Cast the int result from fgetc to unsigned char for processing the byte value
        current_byte = (unsigned char)read_byte_as_int;

        if (isspace(current_byte) || iscntrl(current_byte)) {
            continue; // Skip whitespace characters or invisible control characters
        }

        switch (state) {
            case OUTSIDE_STRUCTURE:  
                // one case: name {
                if (current_byte == '{') {
                    table_structure->name = strdup(name); // Allocate memory for the name
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                name += current_byte;
            case INSIDE_TYPE:
                // one case: type *name
                if (current_byte == '*') {
                    current_attribute = alloc_attr(); // Allocate memory for the current attribute
                    current_attribute->type = parse_attribute_type(attribute_type); // Parse the attribute type
                    state = INSIDE_NAME; // Transition to inside name
                    continue;
                }
                attribute_type += current_byte;
            case INSIDE_NAME:
                // three cases, name @property, name, name {
                if (current_byte == '@') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    state = INSIDE_ATTR; // Transition to inside attribute
                    continue;
                }
                if (current_byte == ',') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    current_attribute->name = strdup(attribute_name); // Allocate memory for the attribute name
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_table_array_len += 1;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                attribute_name += current_byte;
            case INSIDE_ATTR:
                // three cases: @attr @attr, @attr {, @attr,
                if (current_byte == '@') {
                    create_attribute(property, current_attribute->properties);
                    state = INSIDE_ATTR; // Transition to inside attribute
                    continue;
                }
                if (current_byte == ',') {
                    create_attribute(property, current_attribute->properties);
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value                    
                    state = INSIDE_TYPE; // Transition to inside type
                    continue;
                }
                if (current_byte == '}') {
                    create_attribute(property, current_attribute->properties);
                    Attribute* prop_attr = create_attribute(property, TYPE_STR);  // Or infer type
                    utarray_push_back(current_attribute->properties, prop_attr);
                    free(prop_attr);  // Because utarray copies by value  
                    utarray_push_back(schema_table_array, table_structure);    // Add the table structure to the outer array
                    schema_table_array_len += 1;
                    state = OUTSIDE_STRUCTURE; // Transition to outside structure
                    continue;
                }
                property += current_byte;
        }
    }

    if (schema_file == NULL) {
        perror("Error opening schema.flint file");
        DBIndex error_result = {NULL, NULL};
        return error_result;
    }

    // Each entry in the db.obj has an id and structure
    // Hello World\x020
    // Which translates to:
    // str "Hello World",
    // int 0
    // Types are derived from the schema

    IndexEntry buffer;

    // Open the db.idx file
    FILE* idx_file = fopen(strcat(db_path, "/db/db.idx"), "rb");    // idx is binary

    if (idx_file == NULL) {
        perror("Error opening db.idx file");
        DBIndex error_result = {NULL, NULL};
        return error_result;
    }

    // Output format:
    // UT_ARRAY [
    //    StructureObjectsArray [
    //      reserved_int,
    //      UT_ARRAY [
    //        LocOrData { TYPE_LOC, IndexArrayEntry* { location_1, idx_1 } },
    //        LocOrData { TYPE_LOC, IndexArrayEntry* { location_2, idx_2 } },
    //      ]
    //    ]
    // ]

    UT_array *index_table_array;

    utarray_new(index_table_array, &structure_objects_array_icd);
    for (int i = 0; i < schema_table_array_len; i++) {
        StructureObjectsArray soa;
        soa.reserved_int = 0;  // Initialize reserved integer
        utarray_new(soa.objects, &loc_or_data_icd);  // Create array for LocOrData objects
        
        utarray_push_back(index_table_array, &soa);
    }

    uint32_t object_loc = 0;

    uint32_t idx_loc = 0;

    while (fread(&buffer, sizeof(IndexEntry), 1, idx_file) == 1) {  // Read every index entry in the file
        StructureObjectsArray* soa = (StructureObjectsArray*)utarray_eltptr(index_table_array, buffer.table_id);
        
        if (!soa) {
            // Create a new StructureObjectsArray if it doesn't exist
            StructureObjectsArray new_soa;
            new_soa.reserved_int = 0;
            utarray_new(new_soa.objects, &loc_or_data_icd);
            
            // Ensure the outer array is big enough
            while (utarray_len(index_table_array) <= buffer.table_id) {
                StructureObjectsArray empty_soa;
                empty_soa.reserved_int = 0;
                utarray_new(empty_soa.objects, &loc_or_data_icd);
                utarray_push_back(index_table_array, &empty_soa);
            }
            
            soa = (StructureObjectsArray*)utarray_eltptr(index_table_array, buffer.table_id);
        }

        // Ensure the objects array is big enough for the object_id
        size_t required_size = buffer.object_id + 1;
        size_t current_size = utarray_len(soa->objects);
        
        if (required_size > current_size) {
            // Expand array with empty LocOrData slots
            size_t new_size = current_size == 0 ? 64 : current_size;
            while (new_size <= buffer.object_id) {
                new_size *= 2;  // double size each time until big enough
            }
            
            // Add empty slots with initialized LocOrData objects
            for (size_t i = current_size; i < new_size; i++) {
                LocOrData empty_lod;
                loc_or_data_init(&empty_lod);
                utarray_push_back(soa->objects, &empty_lod);
            }
        }

        // Get the reserved integer from the StructureObjectsArray
        uint32_t id = soa->reserved_int;

        // Get the LocOrData object at the specific object_id position
        LocOrData* lod_ptr = (LocOrData*)utarray_eltptr(soa->objects, buffer.object_id);
        if (lod_ptr) {
            // Set the type to TYPE_LOC since we're storing an IndexArrayEntry
            lod_ptr->type = TYPE_LOC;
            
            // Allocate a new IndexArrayEntry and set its values
            IndexArrayEntry* new_entry = malloc(sizeof(IndexArrayEntry));
            new_entry->obj_loc = object_loc;
            new_entry->idx_loc = idx_loc;
            
            // Store the pointer in the union
            lod_ptr->idx_entry = new_entry;
        }

        // Increment the reserved integer for next object
        soa->reserved_int++;
        
        object_loc += buffer.size; // Increment the object location by the size of the object

        idx_loc++;
    }
    
    return make_db_index(schema_table_array, index_table_array);
}

// Custom destructor for StructureObjectsArray
void structure_objects_array_dtor(void* elt) {
    StructureObjectsArray* soa = (StructureObjectsArray*)elt;
    if (soa->objects) {
        utarray_free(soa->objects);
        soa->objects = NULL;
    }
}

// UT_icd for StructureObjectsArray
UT_icd structure_objects_array_icd = {
    sizeof(StructureObjectsArray),
    NULL,  // No special init needed
    NULL,  // Default copy is fine
    structure_objects_array_dtor
};