// Index database

#include <stdio.h>
#include <stdint.h>
#include <ctype.h> 
#include "../utils/utarray.h"

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
static UT_icd outer_utarray_icd = {
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

void ensure_inner_array_size(UT_array* inner_array, size_t pos) {
    size_t len = utarray_len(inner_array);
    if (pos < len) return;  // already big enough

    size_t new_len = len == 0 ? 64 : len;
    while (new_len <= pos) {
        new_len *= 2;  // double size each time until big enough
    }

    uint32_t default_val = UINT32_MAX;

    // Grow in one go:
    for (size_t i = len; i < new_len; i++) {
        utarray_push_back(inner_array, &default_val);
    }
}

void set_inner_array_value(UT_array* inner_array, size_t pos, uint32_t obj_loc, uint32_t idx_loc) {
    ensure_inner_array_size(inner_array, pos);
    IndexArrayEntry* ptr = (IndexArrayEntry*)utarray_eltptr(inner_array, pos);
    *ptr = (IndexArrayEntry){
        .obj_loc = obj_loc,
        .idx_loc = idx_loc
    };
}

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
} DBIndex;

DBIndex make_db_index(UT_array* schema_table_array, UT_array* index_table_array) {
    DBIndex result;
    result.schema_table_array = schema_table_array;
    result.index_table_array = index_table_array;
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
    //    UT_ARRAY [
    //      int location_1,
    //      int location_2,
    //    ]
    // ]

    UT_array *index_table_array;

    utarray_new(index_table_array, &outer_utarray_icd);
    for (int i = 0; i < schema_table_array_len; i++) {
        UT_array* inner_array;
        utarray_new(inner_array, &index_array_entry_icd);

        utarray_push_back(index_table_array, &inner_array);
    }

    uint32_t object_loc = 0;

    int i = 0;

    while (fread(&buffer, sizeof(IndexEntry), 1, idx_file) == 1) {  // Read every index entry in the file
        UT_array* objects_array = *(UT_array**)utarray_eltptr(index_table_array, buffer.table_id);

        set_inner_array_value(objects_array, buffer.object_id, object_loc, i);
        object_loc += buffer.size; // Increment the object location by the size of the object

        i = i++;
    }
    
    return make_db_index(schema_table_array, index_table_array);
}