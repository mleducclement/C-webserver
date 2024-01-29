
#include "my_linkedlist.h"

Node* create_node() {
    Node *node = malloc(sizeof(Node));

    if (node == NULL) {
        printf("Failed to allocate memory for node\n");
        return NULL;
    }
    node->item = NULL;
    node->next = NULL;
    return node;
}

Node* insert_node(Node *head, hashtable_item *item) {
    Node *new_node = create_node();
    new_node->item = item;

    if (head == NULL) {
        return new_node;
    }

    Node* current = head;

    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
    return head;
}

hashtable_item* remove_node(Node** head, const hashtable_item *item) {
    if (*head == NULL) {
        return NULL;
    }
    Node* current = *head;
    Node* previous = NULL;

    while(current != NULL) {
        if (current->item == item) {
            if (previous == NULL) {
                *head = current->next;
            }
            else {
                previous->next = current->next;
            }
            hashtable_item *removed_item = current->item;
            free(current);
            return removed_item;
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

hashtable_item* find_item(const Node* head, const hashtable_item *item) {
    if (head == NULL) {
        return NULL;
    }

    const Node* current = head;

    while(current != NULL) {
        if (current->item == item) {
            return current->item;
        }
        current = current->next;
    }
    return NULL;
}

void print_list(const Node* head) {
    if (head == NULL) {
        printf("List is empty\n");
        return;
    }

    const Node* current = head;

    while (current != NULL) {
        current = current->next;
    }
}

void print_node(const Node* node) {
    printf("NODE ADDRESS %p | NODE KEY: %s | NODE VALUE: ", node, node->item->key);

    switch (node->item->type) {
        case STRING:
            printf("%s\n", (char*)node->item->value);
            break;
        case INT:
            printf("%d\n", *(int*)node->item->value);
            break;
        case FLOAT:
            printf("%f\n", *(double*)node->item->value);
            break;
        case BOOL:
            printf("%s\n", *(bool*)node->item->value ? "true" : "false");
            break;
        case NULL_TYPE:
            printf("(NULL)\n");
            break;
        default:
            printf("ERROR : Invalid type\n");
            break;
    }

    if (node->next != NULL) {
        printf("NEXT NODE ADDRESS: %p\n", node->next);
    }
}

void free_list(Node *head) {
    if (head == NULL) {
        return;
    }
    Node *current = head;
    Node *next = NULL;

    while (current != NULL) {
        next = current->next;
        free(current->item);
        free(current);
        current = next;
    }
}