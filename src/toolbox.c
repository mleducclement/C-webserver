//
// Created by pres-mleducclement on 2024-01-25.
//

#include "toolbox.h"

int getValueType(const char* value) {
    // Checks if value is NULL
    if (strcmp(value, "null") == 0) {
        return NULL_TYPE;
    }

    // Checks if value is a number by checking if ALL the characters in value are digits or a period
    if (strspn(value, "0123456789.") == strlen(value)) {
        // Checks if value is a float
        if (strchr(value, '.') != NULL) {
            return FLOAT;
        }
        return INT;
    }

    // TODO : Handle case where value is capitalized
    // Checks if value is a boolean
    if (strcmp(value, "true") == 0 || strcmp(value, "false") == 0) {
        return BOOL;
    }

    // Checks if value is an hashtable
    if (value[0] == '{' &&  value[strlen(value) - 1] == '}') {
        return HASHTABLE;
    }

    return STRING;
}
