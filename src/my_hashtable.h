
#ifndef WEBSERVER_MY_HASHTABLE_H
#define WEBSERVER_MY_HASHTABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "my_linkedlist.h"
#include "constants.h"

#define CAPACITY 50000 // size of hashtable

// Forward declaration of Node
typedef struct Node Node;



// Defines a struct for a hashtable_item
typedef struct hashtable_item {
    char *key;
    void *value;
    ValueType type;
} hashtable_item;

// Defines a struct for a hashtable
typedef struct hashtable {
    int size;
    int count;
    hashtable_item **items; // Pointer to a pointer to an hashtable_item (in this case, pointer to the first element of an array of hashtable_items)
    Node **overflow_buckets;
} hashtable;

unsigned long hash_function(const char *str);
hashtable* create_table();
hashtable_item* create_item(const char *key, void *value, ValueType type);
void free_item(hashtable_item *item);
void free_table(hashtable *table);
void handle_collisions(const hashtable *table, unsigned long index, hashtable_item *item);
void hashtable_insert(hashtable *table, const char *key, void *value, ValueType type);
void hashtable_insert_with_item(hashtable *table, hashtable_item *item);
void hashtable_delete(hashtable *table, const char *key);
hashtable_item* hashtable_search(const hashtable *table, const char *key);
void print_table(const hashtable *table);
void print_value(const hashtable_item *item);
void print_search(const hashtable *table, const char *key);

#endif //WEBSERVER_MY_HASHTABLE_H
