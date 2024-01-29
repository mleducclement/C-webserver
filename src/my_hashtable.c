
#include "my_hashtable.h"

unsigned long hash_function(const char *str) {
    unsigned long hash = 0;

    for (int j = 0; str[j]; j++) {
        hash += str[j];
    }
    return hash % CAPACITY;
}

Node** create_overflow_buckets(const hashtable *table) {

    // Create the overflow buckets; an array of linkedlists
    Node **buckets = calloc(table->size, sizeof(Node *));

    if (buckets == NULL) {
        printf("Error creating overflow buckets\n");
        return NULL;
    }

    for (int i = 0; i < table->size; i++) {
        buckets[i] = NULL;
    }
    return buckets;
}

void free_overflow_buckets(const hashtable *table) {
    Node **buckets = table->overflow_buckets;

    for (int i = 0; i < table->size; i++) {
        free_list(buckets[i]);
    }
    free(buckets);
}

hashtable* create_table() {
    // Creates a pointer to a hashtable
    hashtable *table = malloc(sizeof(hashtable));

    if (table == NULL) {
        printf("Error creating hashtable\n");
        return NULL;
    }

    table->size = CAPACITY;
    table->count = 0;
    table->items = calloc(table->size, sizeof(hashtable_item *));

    if (table->items == NULL) {
        printf("Error creating hashtable items\n");
        free(table);
        return NULL;
    }

    table->overflow_buckets = create_overflow_buckets(table);
    if (table->overflow_buckets == NULL) {
        free(table->items);
        free(table);
        return NULL;
    }
    return table;
}

hashtable_item* create_item(const char *key, void *value, const ValueType type) {
    hashtable_item *item = malloc(sizeof(hashtable_item));

    if (item == NULL) {
        printf("Error creating hashtable item\n");
        return NULL;
    }

    item->key = strdup(key);
    item->value = value;
    item->type = type;
    return item;
}

void free_item(hashtable_item *item) {
    free(item->key);

    if (item->value != NULL) {
        item->type == HASHTABLE ? free_table(item->value) : free(item->value);
    }
    free(item);
}

void free_table(hashtable *table) {
    for (int i = 0; i < table->size; i++) {
        if (table->items[i] != NULL) {
            free_item(table->items[i]);
        }
    }
    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collisions(const hashtable *table, const unsigned long index, hashtable_item *item) {
    Node *head = table->overflow_buckets[index];

    if (head == NULL) {
        head = create_node();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    insert_node(head, item);
}

void hashtable_insert(hashtable *table, const char *key, void *value, const ValueType type) {
    void *value_to_insert = value;

    // If the value is a int, alloc memory for it and store the pointer
    if (type == INT) {
        int *int_ptr = malloc(sizeof(int));
        if (int_ptr == NULL) {
            printf("Error allocating memory for value\n");
            return;
        }
        // atoi() converts a string to an int
        *int_ptr = atoi(value);
        value_to_insert = int_ptr;
    }

    // TODO : doesn't work for some reason, getting 0.000000 in hashtable
    // If the value is a int, alloc memory for it and store the pointer
    if (type == FLOAT) {
        int *float_ptr = malloc(sizeof(int));
        if (float_ptr == NULL) {
            printf("Error allocating memory for value\n");
            return;
        }
        // atoi() converts a string to an int
        *float_ptr = atoi(value);
        value_to_insert = float_ptr;
    }

    // If the value is a bool, alloc memory for it and store the pointer
    if (type == BOOL) {
        bool *bool_ptr = malloc(sizeof(bool));
        if (bool_ptr == NULL) {
            printf("Error allocating memory for value\n");
            return;
        }
        *bool_ptr = strcmp(value, "true") == 0 ? true : false;
        value_to_insert = bool_ptr;
    }
    hashtable_item *item = create_item(key, value_to_insert, type);

    if (item == NULL) {
        printf("Error creating item\n");
        return;
    }

    if (table->count >= table->size) {
        free_item(item);
        printf("Insert error: Hash table is full\n");
        return;
    }
    insert_into_table(table, item);
}

void insert_into_table(hashtable *table, hashtable_item *item) {
    const int index = hash_function(item->key);
    const hashtable_item *current_item = table->items[index];

    if (current_item == NULL) {
        table->items[index] = item;
        table->count++;
    } else {
        if (strcmp(current_item->key, item->key) == 0) {
            free_item(table->items[index]);
            table->items[index] = item;
        }
        else {
            handle_collisions(table, index, item);
        }
    }
}

void hashtable_delete(hashtable *table, const char *key) {
    // Deletes an item from the table
    const int index = hash_function(key);
    hashtable_item *item = table->items[index];
    Node *head = table->overflow_buckets[index];

    // Check hashtable for item
    if (item == NULL) {
        // Doesn't exist
        printf("Key: %s does not exist\n", key);
        return;
    }

    if (head == NULL && strcmp(item->key, key) == 0) {
        // Collision chain is empty
        table->items[index] = NULL;
        free_item(item);
        table->count--;
        return;
    }

    if (head != NULL) {
        // Collision chain exists
        if (strcmp(item->key, key) == 0) {
            // Remove this item and set the next node as the head
            free_item(item);
            Node *node = head;
            head = head->next;
            node->next = NULL;
            table->items[index] = create_item(node->item->key, node->item->value, node->item->type);
            table->count++;
            free_list(node);
            table->overflow_buckets[index] = head;
            return;
        }
        Node *current = head;
        Node *previous = NULL;

        while (current) {
            if (strcmp(current->item->key, key) == 0) {
                if (previous == NULL) {
                    // First element of the chain. Remove the chain
                    free_list(head);
                    table->overflow_buckets[index] = NULL;
                    return;
                }
                // This is somewhere in the chain
                previous->next = current->next;
                current->next = NULL;
                free_list(current);
                table->overflow_buckets[index] = head;
                return;
            }
            previous = current;
            current = current->next;
        }
    }
}
hashtable_item* hashtable_search(const hashtable *table, const char *key) {
    const int index = hash_function(key);
    hashtable_item *item = table->items[index];
    const Node *head = table->overflow_buckets[index];

    // Checks the hashtable for the item
    if (item != NULL && strcmp(item->key, key) == 0) {
            return item;
    }
    // If not found in the hastable, check the overflow bucket
    const Node *current = head;

    while (current != NULL) {
        if (strcmp(current->item->key, key) == 0) {
            return current->item;
        }
        current = current->next;
    }
    return NULL;
}

void print_search(const hashtable *table, const char *key) {
    const hashtable_item *result = hashtable_search(table, key);
    result == NULL ? printf("Key: %s does not exist\n", key) :
        result->value == NULL ? printf("Key: %s exists but has no value\n", key) :
        print_value(result);
}

void print_table(const hashtable *table, const char *key) {
    printf("HASHTABLE START ====================\n");
    printf("%s\n", key);
    for (int i = 0; i < table->size; i++) {
        if (table->items[i]) {
            print_value(table->items[i]);
        }
    }
    printf("END TABLE ====================\n");
}

void print_value(const hashtable_item *item) {
    printf(" KEY: %s, VALUE: ", item->key);

    switch (item->type) {
        case STRING:
            printf("%s", (char*)item->value);
            break;
        case INT:
            printf("%d", *(int*)item->value);
            break;
        case FLOAT:
            printf("%f", *(float*)item->value);
            break;
        case BOOL:
            printf("%s", *(bool*)item->value ? "true" : "false");
            break;
        case HASHTABLE:
            printf("NESTED\n");
            print_table(item->value,item->key);
            break;
        default:
            printf("Invalid type");
    }
    printf("\n");
}