//
// Created by pres-mleducclement on 2024-01-25.
//

#include "toolbox.h"

hashtable* parseJSON(const char *jsonString) {
    char *cleanedJSON = cleanJSON(jsonString);
    void *kvpArray[MAX_KVP];
    int index = 0;
    int start = 0;
    int count = 0;
    int nestingLevel = 0;

    if (cleanedJSON == NULL) {
        printf("Error cleaning JSON\n");
        return NULL;
    }
    printf("Now parsing JSON : %s\n", cleanedJSON);

    // Checks if JSON is malformed
    if (cleanedJSON[0] != '{' || cleanedJSON[strlen(cleanedJSON) - 1] != '}') {
        printf("Malformed JSON input : %s\n", cleanedJSON);
        return NULL;
    }

    // Iterate over the entire JSON string
    while (cleanedJSON[index] != '\0' && count < MAX_KVP) {
        if (cleanedJSON[index] == '{') {
            nestingLevel++;
            if (nestingLevel == 1) {
                start = index + 1;    // Start a new object
            }
        } else if (cleanedJSON[index] == '}') {
            nestingLevel--;
            if (nestingLevel == 0) {
                // End of the current object
                char *kvp = strndup(cleanedJSON + start, index - start);
                kvpArray[count++] = kvp;
                start = index + 1;
            }
        } else if (cleanedJSON[index] == ',' && nestingLevel == 1) {
            // Split on commas only at the top level
            char *kvp = strndup(cleanedJSON + start, index - start);
            kvpArray[count++] = kvp;
            start = index + 1;
        }
        index++;
    }

    hashtable *table = create_table();

    // Iterate over the kvpArray
    for (int i = 0; i < count; i++) {
        // Finds the string index of the first occurrence of ':'
        // colon is not alloc new memory it points to the first occurence of the ':' in the string
        const char *colon = strchr(kvpArray[i], ':');
        if (colon != NULL) {
            // Copies the string from kvpArray[i] to colon - kvpArray[i] into key
            char *key = strndup(kvpArray[i], colon - (char*)kvpArray[i]);
            void *value;
            const ValueType type = getValueType(colon + 1);   // Determine type directly from JSON

            if (type == STRING) {
                // Extract and clean string value
                value = strndup(colon + 2, strlen(kvpArray[i]) - strlen(key) - 3);
            } else if (type == HASHTABLE) {
                // Parse nested JSON to create hashtable
                value = parseJSON(colon + 1);    // Each nested object is a new JSON string
            } else {
                // Handle other types (INT, FLOAT, BOOL)
                value = strndup(colon + 1, strlen(kvpArray[i]) - strlen(key) - 1);
            }

            if (key != NULL) {
                hashtable_insert(table, key, value, type);
            }
            free(key);    // Free the key string after it's used
        }
        free(kvpArray[i]);
    }
    return table;
}

char* cleanJSON(const char *string) {
    if (string == NULL) return NULL;
    const int length = strlen(string);
    char *output = malloc(length + 1);

    if (output == NULL) {
        printf("Error allocating memory for output\n");
        free(output);
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < length; ++i) {
        if (!isspace((unsigned char)string[i])) {
            output[j++] = string[i];
        }
    }
    output[j] = '\0';

    return output;
}

int getValueType(const char* value) {
    // toLowerCase() needs a copy of value because it modifies it
    char *lowerValue = strdup(value);

    if (lowerValue == NULL) {
        printf("Error allocating memory to lowerValue\n");
        return -1;
    }
    toLowerCase(lowerValue);

    // Checks if value is NULL
    if (strcmp(lowerValue, "null") == 0) {
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
    if (strcmp(lowerValue, "true") == 0 || strcmp(lowerValue, "false") == 0) {
        return BOOL;
    }

    // Checks if value is an hashtable
    if (value[0] == '{' &&  value[strlen(value) - 1] == '}') {
        return HASHTABLE;
    }
    free(lowerValue);
    return STRING;
}

void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}
