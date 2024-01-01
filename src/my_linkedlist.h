
#ifndef MY_LINKEDLIST_H
#define MY_LINKEDLIST_H

#include "my_hashtable.h"
#include <stdlib.h>

// Forward declaration of hashtable_item
typedef struct hashtable_item hashtable_item;

typedef struct Node {
    hashtable_item *item;
    struct Node *next;
} Node;

Node* create_node();
Node* insert_node(Node *head, hashtable_item *item);
hashtable_item* remove_node (Node** head, const hashtable_item *item);
hashtable_item* find_item(const Node* head, const hashtable_item *item);
void print_list(const Node* head);
void free_list(Node *head);

#endif //MY_LINKEDLIST_H
