#include "mtll.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char** argv) {
    struct mtll** lists = NULL; // Start with NULL list
    size_t listsCapacity = 0;
    size_t listCount = 0; // Counter for the number of lists

    char line[129]; // Buffer to store input commands
    while (1) {
        if (!fgets(line, sizeof(line), stdin)) {
            break; // Break the loop if reading input error
        }
    
        if (line[strlen(line) - 1] != '\n') {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }

        line[strcspn(line, "\n")] = 0; // Remove newline character

        // NEW Command
        if (strncmp(line, "NEW", 3) == 0) {
            char* endPtr;
            long numElements = strtol(line + 4, &endPtr, 10);
            if(endPtr == line + 4 || *endPtr != '\0' || numElements < 0) {
                printf("INVALID COMMAND: NEW\n");
                continue;
            }

            if (listCount == listsCapacity) {
                // Double capacity or initialize to 10 if capacity is 0
                size_t newCapacity = listsCapacity == 0 ? 10 : listsCapacity * 2;
                struct mtll** tempList = realloc(lists, newCapacity * sizeof(struct mtll*));
                if (!tempList) {
                    perror("INVALID COMMAND: NEW\n");
                    continue;
                }
                lists = tempList;
                listsCapacity = newCapacity;
            }

            struct mtll* newList = mtll_create();
            char elementBuffer[129]; // Buffer to hold each element input
            int isNestedList = 0;
            int eofEncountered = 0;

            for (long i = 0; i < numElements; i++) {
                if (!fgets(elementBuffer, sizeof(elementBuffer), stdin)) {
                    if (feof(stdin)) {
                        eofEncountered = 1;
                    } else {
                        printf("INVALID COMMAND: NEW\n");
                    }
                    break;
                }

                // Input longer than buffer
                if (elementBuffer[strlen(elementBuffer) - 1] != '\n') {
                    int ch;
                    // Discard excess characters
                    while ((ch = getchar()) != '\n' && ch != EOF);
                }
                elementBuffer[strcspn(elementBuffer, "\n")] = '\0';

                // Check if element indicates nested list
                if (elementBuffer[0] == '{' && elementBuffer[strlen(elementBuffer) - 1] == '}') {
                    isNestedList = 1;
                }
                mtll_add_element(newList, elementBuffer);
            }

            // Proceed if above check complete
            if (!eofEncountered) {
                newList->isNested = isNestedList;

                if (newList->isNested) {
                    printf("Nested %zu: ", newList->id);
                } else {
                    printf("List %zu: ", newList->id);
                }
                mtll_view(newList);
                lists[listCount++] = newList;
            } else {
                mtll_free(newList);
            }
        } else if (strncmp(line, "VIEW-NESTED", 11) == 0) {
            size_t id;
            if (sscanf(line + 12, "%zu", &id) == 1) {
                struct mtll* targetList = NULL;
                // Search for list with given id
                for (size_t i = 0; i < listCount; i++) {
                    if (lists[i] != NULL && lists[i]->id == id) {
                        targetList = lists[i];
                        break;
                    }
                }
                // If list found, view
                if (targetList != NULL) {
                    mtll_view_nested(targetList, lists, listCount);
                } else {
                    printf("INVALID COMMAND: VIEW-NESTED\n");
                }
            } else {
                printf("INVALID COMMAD: VIEW-NESTED\n");
            }
        } else if (strncmp(line, "VIEW ALL", 8) == 0) {
            mtll_view_all(lists, listCount);

        } else if (strncmp(line, "VIEW", 4) == 0) {
            size_t id;
            if(sscanf(line + 5, "%zu", &id) == 1) {
                int found = 0;
                for(size_t i = 0; i < listCount; i++) {
                    if (lists[i] != NULL && lists[i]->id == id) {
                        mtll_view(lists[i]);
                        found = 1;
                        break;
                    }
                }
                if(!found) {
                    printf("INVALID COMMAND: VIEW\n");
                }
            } else {
                printf("INVALID COMMAND: VIEW\n");
            }

        } else if (strncmp(line, "TYPE", 4) == 0) {
            size_t id;
            if (sscanf(line + 5, "%zu", &id) == 1) {
                int found = 0;
                for(size_t i = 0; i < listCount; i++) {
                    if (lists[i] != NULL && lists[i]->id == id) {
                        mtll_view_types(lists[i]);
                        found = 1;
                        break;
                    }
                } 
                if (!found) {
                    printf("INVALID COMMAND: TYPE\n");
                }
            } else {
                printf("INVALID COMMAND: TYPE\n");
            }

        } else if (strncmp(line, "REMOVE", 6) == 0) {
            size_t id;
            char idStr[32]; // Buffer to temporary store id
            char extraCheck; // Check for extra input after id
            char* endPtr;
            int listExists = 0;
            
            // Check for extra input
            if (sscanf(line + 7, "%31s %c", idStr, &extraCheck) == 2) {
                printf("INVALID COMMAND: REMOVE\n");
                continue;
            } 
            // If id present, parse and validate
            else if (sscanf(line + 7, "%31s", idStr) == 1) {
                long parsedID = strtol(idStr, &endPtr, 10);
                if (*endPtr != '\0' || parsedID < 0) {
                    printf("INVALID COMMAND: REMOVE\n");
                    continue;
                }
                id = (size_t)parsedID;
                // Check if id exists
                for(size_t i = 0; i < listCount; i++) {
                    if (lists[i] != NULL && lists[i]->id == id) {
                        listExists = 1;
                        break;
                    }
                }

                if (!listExists){
                    printf("INVALID COMMAND: REMOVE\n");
                    continue;
                }

                // Check if list is referenced in any other list
                int skipRemoval = 0;  // Whether the list is referenced flag
                for (size_t i = 0; i < listCount && !skipRemoval; i++) {
                    struct Node* current = lists[i]->head;
                    while (current) {
                        if (current->type == REFERENCE && current->refListId == id) {
                            printf("INVALID COMMAND: REMOVE\n");
                            skipRemoval = 1;
                            break;
                        }
                        current = current->next;
                    }
                }
                // If list not referenced, remove
                if (!skipRemoval) { // Proceed with removal if the list is not referenced
                    mtll_remove(lists, &listCount, id);
                    printf("List %zu has been removed.\n", id);
                    printf("\n");
                    mtll_view_all(lists, listCount);
                }
            } else {
                printf("INVALID COMMAND: REMOVE\n");
                continue;
            }
        } else if (strncmp(line, "INSERT", 6) == 0) {
            size_t id;
            long index;
            char indexStr[32]; // Buffer for index string
            char value[129];

            // Parse the command input
            int scanCount = sscanf(line + 7, "%zu %31s %[^\n]", &id, indexStr, value);
            // Must include id and index
            if (scanCount < 2) {
                printf("INVALID COMMAND: INSERT\n");
                continue;
            }

            // Checking third argument from spaces and non-spaces
            char* restOfLine = line + 7;
            for (int i = 0; i < 2; ++i) {
                while (*restOfLine && !isspace((unsigned char)*restOfLine)) restOfLine++; // Skip non-space
                while (*restOfLine && isspace((unsigned char)*restOfLine)) restOfLine++; // Skip space
            }

            if (*restOfLine) { // Have something after index
                strncpy(value, restOfLine, sizeof(value) - 1); // Take the rest as value
            } else {
                value[0] = '\0'; // Explicitly noting there's no value
            }

            char* endPtr;
            index = strtol(indexStr, &endPtr, 10);
            if (*endPtr != '\0') {
                printf("INVALID COMMAND: INSERT\n");
                continue;
            }

            if (scanCount == 2) {
                value[0] = '\0';
            }

            struct mtll* targetList = NULL;
            for (size_t i = 0; i < listCount; i++) {
                if (lists[i] != NULL && lists[i]->id == id) {
                    targetList = lists[i];
                    break;
                }
            }

            if (targetList == NULL) {
                printf("INVALID COMMAND: INSERT\n");
                continue;
            }

            // Adjust for negative index
            if (index < 0) {
                index += targetList->size + 1;
            }
            
            // Validate index range and insert
            if (index >= 0 && (size_t)index <= targetList->size) {
                if (value[0] == '{' && value[strlen(value) - 1] == '}') {
                    if (targetList->size == 0) {
                        printf("INVALID COMMAND: INSERT\n");
                        continue;
                    }
                    value[strlen(value) - 1] = '\0';
                    size_t refListId = strtoul(value + 1, &endPtr, 10);
                    if (*endPtr == '\0' && refListId != id) {
                        struct mtll* refList = NULL;
                        for (size_t j = 0; j < listCount; j++) {
                            if (lists[j] != NULL && lists[j]->id == refListId) {
                                refList = lists[j];
                                break;
                            }
                        }
                        if (refList == NULL) {
                            printf("INVALID COMMAND: INSERT\n");
                            continue;
                        }
                        mtll_insert_reference(targetList, index, refListId);
                    } else {
                        printf("INVALID COMMAND: INSERT\n");
                        continue;
                    }
                } else {
                    mtll_insert(targetList, index, value);
                }

                if (value[0] == '{') {
                    targetList->isNested = 1;
                }

                if (targetList->isNested) {
                    printf("Nested %zu: ", targetList->id);
                } else {
                    printf("List %zu: ", targetList->id);
                }
                mtll_view(targetList);
            } else {
                printf("INVALID COMMAND: INSERT\n");
            }
        } else if (strncmp(line, "DELETE", 6) == 0) {
            size_t id;
            char indexStr[32];
            char extra[2];
            char *endPtr;
            // Check for extra input
            if (sscanf(line + 7, "%zu %32s %1s", &id, indexStr, extra) == 3) {
                printf("INVALID COMMAND: DELETE\n");
            } else if (sscanf(line + 7, "%zu %32s", &id, indexStr) == 2) {
                long index = strtol(indexStr, &endPtr, 10);
                if (*endPtr != '\0') {
                    printf("INVALID COMMAND: DELETE\n");
                } 
                // Check if id within bound and list exists
                else if (id < listCount && lists[id] != NULL){ 
                    if (index < 0) {
                        index += lists[id]->size;
                    }
                    if (index >= 0 && index < lists[id]->size) {
                        mtll_delete(lists[id], index);
                        printf("List %zu: ", id);
                        mtll_view(lists[id]);
                    } else {
                        printf("INVALID COMMAND: DELETE\n");
                    }
                } else {
                    printf("INVALID COMMAND: DELETE\n");
                }
            } else {
                printf("INVALID COMMAND: DELETE\n");
            }
        } else {
            printf("INVALID COMMAND: INPUT\n");
        }
    }

    // Free any remaining lists
    for(size_t i = 0; i < listCount; i++) {
        if (lists[i] != NULL) {
            mtll_free(lists[i]);
        }
    }
    free(lists);

    return 0;
}