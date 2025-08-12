#ifndef SCHEMA_PARSER_RESULT_H
#define SCHEMA_PARSER_RESULT_H

#include "../utils/utarray.h"
#include <stdint.h>

typedef struct {
    UT_array* schema_table_array;
    uint32_t length;
} SchemaParserResult;

#endif