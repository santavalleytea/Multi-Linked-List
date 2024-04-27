#include "mtll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

// Function to create list
struct mtll* mtll_create() {
    static size_t nextId = 0;
    struct mtll* list = malloc(sizeof(struct mtll));
    // Ensure successful memory allocation
    if (!list) {
        perror("Failed to allocate memory for the list");
        exit(EXIT_FAILURE);
    }
    list->head = NULL; // Initialize head pointer to NULL
    list->size = 0; // Initial size of list 0
    list->id = nextId++;
    list->isNested = 0;
    return list;
}

// Function for freeing memory in mtll
void mtll_free(struct mtll* list) {
    struct Node* current = list->head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
        if (temp->type == STRING) {
            free(temp->value.stringValue);
        }

        free(temp);
    }
    free(list);
}

// Function for viewing mtll
void mtll_view(const struct mtll* list) {
    if (!list->head) {
        printf("\n");
        return;
    }

    struct Node* current = list->head;
    while (current) {
        switch (current->type) {
            case INT:
                printf("%d", current->value.intValue);
                break;
            case FLOAT:
                printf("%.2f", current->value.floatValue);
                break;
            case CHAR:
                printf("%c", current->value.charValue);
                break;
            case STRING:
                printf("%s", current->value.stringValue);
                break;
            case REFERENCE:
                printf("{List %zu}", current->refListId);
                break;
        }
        if (current->next) printf(" -> ");
        current = current->next;
    }
    printf("\n");
}

// Function for viewing types in mtll
void mtll_view_types(const struct mtll* list) {
    struct Node* current = list->head;
    if (!current) {
        printf("INVALID COMMAND: TYPE\n");
        return;
    }

    while (current) {
        switch (current->type) {
            case INT: 
                printf("int");
                break;
            case FLOAT:
                printf("float");
                break;
            case CHAR: 
                printf("char");
                break;
            case STRING:
                printf("string");
                break;
            case REFERENCE:
                printf("reference");
                break;
        }
        current = current->next;

        if (current) {
            printf(" -> ");
        }
    }
    printf("\n");
}

// Function to add element when using NEW command
void mtll_add_element(struct mtll* list, const char* value) {
    if (!value || !list) return;

    struct Node* newNode = malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Failed to allocate memory for newNode");
        exit(EXIT_FAILURE);
    }
    newNode->next = NULL;

    if (value[0] == '{' && value[strlen(value) - 1] == '}') {
        list->isNested = 1;
        char* endPtr;
        long refListId = strtol(value + 1, &endPtr, 10);
        if (value + 1 != endPtr && *endPtr == '}') {
            newNode->type = REFERENCE;
            newNode->refListId = refListId;
        } else {
            printf ("Error: Invalid reference format.\n");
            free(newNode);
            return;
        }
    } else {
        char* endPtr;
        errno = 0;
        newNode->value.intValue = strtol(value, &endPtr, 10);
        if(endPtr != value && *endPtr == '\0' && errno == 0) {
            newNode->type = INT;
        } else {
            errno = 0;
            newNode->value.floatValue = strtof(value, &endPtr);
            if (endPtr != value && *endPtr == '\0' && errno == 0) {
                newNode->type = FLOAT;
            } else if (strlen(value) == 1 && isprint((unsigned char)value[0])) { 
                newNode->type = CHAR;
                newNode->value.charValue = *value;
            } else {
                newNode->type = STRING;
                newNode->value.stringValue = strdup(value);
            }
        }
    }

    // Append newNode to the list
    if (list->head == NULL) {
        list->head = newNode;
    } else {
        struct Node* current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    list->size++;
}

// Function to view all in mtll
void mtll_view_all(struct mtll** lists, size_t count) {
    printf("Number of lists: %zu\n", count);
    for(size_t i = 0; i < count; i++) {
        if (lists[i] != NULL) {
            int containsReference = 0;
            struct Node* current = lists[i]->head;
            while (current) {
                if (current->type == REFERENCE) {
                    containsReference = 1;
                    break;
                }
                current = current->next;
            }

            if (containsReference) {
                printf("Nested %zu\n", lists[i]->id);
            } else {
                printf("List %zu\n", lists[i]->id);
            }
        }
    }
}

// Function to remove in mtll
void mtll_remove (struct mtll** lists, size_t* count, size_t id) {
    for(size_t i = 0; i < *count; i++) {
        if(lists[i] != NULL && lists[i]->id == id) {
            mtll_free(lists[i]);
            lists[i] = NULL;
            for(size_t j = i; j < (*count) - 1; j++) {
                lists[j] = lists[j + 1];
            }
            lists[(*count) - 1] = NULL;
            (*count)--;
            return;
        }
    }
    printf("INVALID COMMAND: REMOVE\n");
}

// Function to insert
void mtll_insert(struct mtll* list, long index, const char* value) {
    if (!list) {
        printf("INVALID COMMAND: INSERT\n");
        return;
    }

    size_t len = list->size;
    if (index < 0) {
        index += len + 1;
    }

    if (index < 0 || index > len) {
        printf("INVALID COMMAND: INSERT\n");
        return;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Failed to allocate memory for newNode");
        exit(EXIT_FAILURE);
    }

    char* endPtr;
    newNode->value.intValue = strtol(value, &endPtr, 10);
    if (endPtr != value && *endPtr == '\0') {
        newNode->type = INT;
    } else {
        errno = 0;
        newNode->value.floatValue = strtof(value, &endPtr);
        if (endPtr != value && *endPtr == '\0' && errno == 0) {
            newNode->type = FLOAT;
        } else if (strlen(value) == 1 && isprint((unsigned char)value[0])){
            newNode->type = CHAR;
            newNode->value.charValue = value[0];
        } else {
            newNode->type = STRING;
            newNode->value.stringValue = strdup(value);
        }
    }

    if (index == 0) {
        newNode->next = list->head;
        list->head = newNode;
    } else {
        struct Node* current = list->head;
        for (long i = 0; i < index - 1; i++) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
    list->size++;
}

// Function to delete
void mtll_delete(struct mtll* list, long index) {
    if (!list || !list->head || list->size == 0) {
        return;
    }

    if (index < 0) {
        index += list->size;
    }

    if (index < 0 || index >= list->size) {
        return;
    }

    struct Node *toDelete = NULL, *current = list->head;
    if (index == 0) {
        toDelete = list->head;
        list->head = list->head->next;
    } else {
        for (long i = 0; i < index - 1; ++i) {
            current = current->next;
        }
        toDelete = current->next;
        current->next = current->next->next;
    }

    if (toDelete) {
        if (toDelete->type == STRING) {
            free(toDelete->value.stringValue);
        }
    free(toDelete);
    list->size--;
    }
}

// Function when inserting reference
void mtll_insert_reference(struct mtll* list, long index, size_t refListId) {
    if (!list) {
        printf("INVALID COMMAND: INSERT\n");
        return;
    }

    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (!newNode) {
        perror("Failed to allocate memory for newNode");
        exit(EXIT_FAILURE);
    }

    newNode->type = REFERENCE;
    newNode->refListId = refListId;
    newNode->next = NULL;

    if (index < 0) {
        index += list->size;
    }

    if (index < 0 || (size_t)index > list->size) {
        printf("INVALID COMMAND: INSERT\n");
        free(newNode);
        return;
    }

    if (index == 0) {
        newNode->next = list->head;
        list->head = newNode;
    } else {
        struct Node* current = list->head;
        for (long i = 0; i < index - 1; i++) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }

    list->size++;
}

// Function for handling nested lists
void mtll_view_nested_internal(const struct mtll* list, struct mtll** lists, size_t listsCount, int depth) {
    if (!list || !list->head) {
        if (depth == 0) {
            printf("\n");
        }
        return;
    }

    struct Node* current = list->head;
    while(current) {
            switch (current->type) {
                case INT:
                    printf("%d", current->value.intValue);
                    break;
                case FLOAT:
                    printf("%.2f", current->value.floatValue);
                    break;
                case CHAR:
                    printf("%c", current->value.charValue);
                    break;
                case STRING:
                    printf("%s", current->value.stringValue);
                    break;
                case REFERENCE: {
                    size_t refListId = current->refListId;
                    struct mtll* refList = NULL;
                    for (size_t i = 0; i < listsCount; i++) {
                        if (lists[i] != NULL && lists[i]->id == refListId) {
                            refList = lists[i];
                            break;
                        }
                    }
                    if (refList != NULL) {
                        printf("{");
                        mtll_view_nested_internal(refList, lists, listsCount, depth + 1);
                        printf("}");
                    } else {
                        printf("{Invalid List Reference}");
                    }
                    break;
                }
            }
            if (current->next) {
                printf(" -> ");
            }
            current = current->next;
    }
    if (depth == 0) {
        printf("\n");
    }
}

// Function to handle nested list, and handling the recursion in previous function
void mtll_view_nested(const struct mtll* list, struct mtll** lists, size_t listsCount) {
    mtll_view_nested_internal(list, lists, listsCount, 0);
}