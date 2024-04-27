#ifndef MTLL_H
#define MTLL_H

#include <stddef.h> // For size_t

// Represent type of each element in the list
enum ElementType {
    INT,
    FLOAT,
    CHAR,
    STRING,
    REFERENCE // Represents references to other lists
};

// Node properties
struct Node {
    // Union to hold the value of an element
    union ElementValue {
        int intValue;
        float floatValue;
        char charValue;
        char* stringValue;
        struct mtll* listValue; // Pointer to another list; Reference type
    } value;
    enum ElementType type;
    struct Node* next;
    size_t refListId;
};

struct mtll {
    struct Node *head; // Pointer to the first node
    size_t size; // Number of elements in the list
    size_t id;
    int isNested;
    int isReferenced; // Added file to track reference count
};

extern struct mtll *mtll_create();

extern void mtll_free(struct mtll* list);

extern void mtll_view_types(const struct mtll* list);

extern void mtll_view(const struct mtll* list);

extern void mtll_view_all(struct mtll** lists, size_t count);

extern void mtll_remove(struct mtll** lists, size_t* count, size_t index);

extern void mtll_add_element(struct mtll* list, const char* value);

extern void mtll_insert(struct mtll* list, long index, const char* value);

extern void mtll_delete(struct mtll* list, long index);

extern void mtll_view_nested(const struct mtll* list, struct mtll** lists, size_t listsCount);

extern void mtll_view_nested_internal(const struct mtll* list, struct mtll** lists, size_t listsCount, int depth);

extern void mtll_insert_reference(struct mtll* list, long index, size_t refListId);

#endif