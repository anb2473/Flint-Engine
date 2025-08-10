// Create a new table

#include <stdint.h>
#include "../utils/utarray.h"
#include <stdio.h>

typedef struct {
    UT_array* schema_table_array;
    UT_array* index_table_array;
    UT_array* empty_indexes;
} DBIndex;

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

TableStructure* alloc_table_structure() {
    TableStructure *table_structure = malloc(sizeof(TableStructure));
    utarray_new(table_structure->attributes, &attribute_icd);
    return table_structure;
}

char* attr_type_to_str(AttributeType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_STR: return "str";
        case TYPE_FLOAT: return "float";
        default: return "unknown";
    }
}

int mk_table(const char* db_path, DBIndex* db_index, char* table_name, UT_array* attributes) {
    TableStructure* table_structure = alloc_table_structure(); // Allocate memory for the table structure

    table_structure->name = strdup(table_name); // Allocate memory for the table name
    table_structure->attributes = attributes; // Assign the attributes to the table structure

    utarray_push_back(db_index->schema_table_array, table_structure);   // Add the table structure to the schema array

    FILE* schema_file = fopen(strcat(db_path, "/db/schema.flint"), "ab"); // Open schema file in append mode
    if (schema_file == NULL) {
        perror("Error opening schema.flint file");
        return -1; // Return error code
    }

    char* attribute_str = "";
    for (int i = 0; i < utarray_len(attributes); i++) {
        Attribute* attr = (Attribute*)utarray_eltptr(attributes, i);
        if (i > 0) {
            strcat(attribute_str, ", ");
        }
        char* property_str = "";
        for (int i = 0; i < attr->properties->n; i++) {
            property_str = strcat(property_str, " @", utarray_eltptr(attr->properties, i));
        }
        strcat(attribute_str, attr_type_to_str(attr->type), " *", attr->name, property_str);
    }

    fprintf(schema_file, 
        strcat(
            "%s {\n", attribute_str,"}\n", table_structure->name
        )
    ); // Write the table name to the schema file

    return 0;
}