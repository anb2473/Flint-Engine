#ifndef SCHEMA_SCANNER_STATE_H
#define SCHEMA_SCANNER_STATE_H

// Define an enum to represent the state in the schema scanner
typedef enum {
    OUTSIDE_STRUCTURE,
    INSIDE_TYPE,
    INSIDE_NAME,
    INSIDE_PROPERTY,
} SchemaScannerState;

#endif