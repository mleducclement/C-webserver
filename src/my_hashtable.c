
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

    // calloc is contiguous memory allocation
    table->items = (hashtable_item **) calloc(table->size, sizeof(hashtable_item *));

    if (table->items == NULL) {
        printf("Error creating hashtable items\n");
        return NULL;
    }

    for (int i = 0; i < table->size; i++) {
        table->items[i] = NULL;
    }

    table->overflow_buckets = create_overflow_buckets(table);
    return table;
}

hashtable_item* create_item(const char *key, void *value, enum ValueType type) {
    // The hashtable size is 16 bytes (on the current machine),
    // BUT the key and value are pointers so the ACTUAL value pointed to can be MUCH larger

    // Creates a pointer to a hashtable_item
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
    // Frees the item
    free(item->key);
    free(item->value);
    free(item);
}

void free_table(hashtable *table) {
    // Frees the table
    for (int i = 0; i < table->size; i++) {
        hashtable_item *item = table->items[i];
        if (item != NULL) {
            free_item(item);
        }
    }
    free_overflow_buckets(table);
    free(table->items);
    free(table);
}

void handle_collisions(const hashtable *table, const unsigned long index, hashtable_item *item) {
    Node *head = table->overflow_buckets[index];

    if (head == NULL) {
        // Creates the list
        head = create_node();
        head->item = item;
        table->overflow_buckets[index] = head;
        return;
    }
    // Insert to the list
    table->overflow_buckets[index] = insert_node(head, item);
}

void insert_into_table(hashtable *table, hashtable_item *item) {
    const int index = hash_function(item->key);

    const hashtable_item *current_item = table->items[index];

    if (current_item == NULL) {
        // KEY does not exist
        if (table->count == table->size) {
            printf("Insert error: Hash table is full\n");
            return;
        }

        // Insert directly
        table->items[index] = item;
        table->count++;
    }
    else {
        // Update value if *key already exists
        if (strcmp(current_item->key, item->key) == 0) {
            strcpy(table->items[index]->value, item->value);
        }
        else {
            handle_collisions(table, index, item);
        }
    }
}

void hashtable_insert(hashtable *table, const char *key, void *value, enum ValueType type) {
    hashtable_item *item = create_item(key, value, type);
    hashtable_insert_with_item(table, item);

    if (table->count == table->size) {
        free_item(item);
    }
}

void hashtable_insert_with_item(hashtable *table, hashtable_item *item) {
    insert_into_table(table, item);
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

char* hashtable_search(const hashtable *table, const char *key) {
    const int index = hash_function(key);
    const hashtable_item *item = table->items[index];
    const Node *head = table->overflow_buckets[index];

    // Checks the hashtable for the item
    if (item != NULL && strcmp(item->key, key) == 0) {
            if (item->value == NULL) {
                return "(NULL)";
            }
            return item->value;
    }
    // If not found in the hastable, check the overflow bucket
    const Node *current = head;
    while (current != NULL) {
        if (strcmp(current->item->key, key) == 0) {
            return current->item->value;
        }
        current = current->next;
    }
    return NULL;
}

void print_table(const hashtable *table) {

    printf("\nHASH TABLE\n");
    printf("=====================================\n");
    for (int i = 0; i < table->size; i++) {
        if (table->items[i]) {
            print_value(i, table->items[i]);
        }
    }
    printf("=====================================\n");
}

void print_value(const int index, const hashtable_item *item) {
    printf("INDEX: %d, KEY: %s, VALUE: ", index, item->key);
    switch (item->type) {
        case STRING:
            printf("%s\n", (char*)item->value);
        break;
        case INT:
            printf("%d\n", *(int*)item->value);
        break;
        case FLOAT:
            printf("%f\n", *(float*)item->value);
        break;
        case BOOL:
            printf("Index: %d, Key: %s, Value: %s\n", index, item->key, *(bool*)item->value ? "true" : "false");
        break;
        case HASHTABLE:
            print_table(item->value);
        break;
        case default:
            stderr("Invalid type\n");
        break;
    }
    printf("\n");
}

void print_search(const hashtable *table, const char *key) {
    void *result = hashtable_search(table, key);

    if (result == NULL) {
        printf("Key: %s does not exist\n", key);
    }
    else {
        printf("Key: %s, Value: %s\n", key, result);
    }
}