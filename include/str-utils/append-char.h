#ifndef CHAR_CAT_H
#define CHAR_CAT_H

#include <string.h>

void append_char(char* buffer, size_t buffer_size, char c) {
    size_t len = strlen(buffer);
    if (len + 1 < buffer_size) {  // +1 for null terminator
        buffer[len] = c;
        buffer[len + 1] = '\0';
    }
}

#endif