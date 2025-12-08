#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int popularity;
    long lastAccessed;  
} Book;

int find_book_index(Book *shelf, int size, int id) {
    for (int i = 0; i < size; i++) {
        if (shelf[i].id == id) {
            return i;
        }
    }
    return -1;
}

void add_book(Book *shelf, int capacity, int *sizeofPtr, int id, int popularity, long currentTime) {

    int size = *sizeofPtr;
    int index = find_book_index(shelf, size, id);

    // if book already exists then update
    if (index != -1) {
        shelf[index].popularity = popularity;
        shelf[index].lastAccessed = currentTime;
        return;
    }

    // if there is space present, simply insert the book
    if (size < capacity) {
        shelf[size].id = id;
        shelf[size].popularity = popularity;
        shelf[size].lastAccessed = currentTime;
        *sizeofPtr = size + 1;
        return;
    }

    // if space is full, remove least recently accessed book
    int minIndex = 0;
    long minTime = shelf[0].lastAccessed;

    for (int i = 1; i < size; i++) {
        if (shelf[i].lastAccessed < minTime) {
            minTime = shelf[i].lastAccessed;
            minIndex = i;
        }
    }

    shelf[minIndex].id = id;
    shelf[minIndex].popularity = popularity;
    shelf[minIndex].lastAccessed = currentTime;
}

int access_book(Book *shelf, int size, int id, long currentTime) {
    int index = find_book_index(shelf, size, id);

    if (index == -1) return -1;

    shelf[index].lastAccessed = currentTime;
    return shelf[index].popularity;
}

int main() {

    int capacity, Q;

    if (scanf("%d %d", &capacity, &Q) != 2) {
        fprintf(stderr, "Error: Invalid input format!\n");
        return 1;
    }

    Book *shelf = (Book *)malloc(capacity * sizeof(Book));
    if (shelf == NULL) {
        fprintf(stderr, "Not enough memory available! Exiting the program\n");
        return 1;
    }

    int currentSize = 0; 
    long timeCounter = 0;
    char op[20];


    for (int i = 0; i < Q; i++) {

        if (scanf("%s", op) != 1) {
            fprintf(stderr, "Error reading operation!\n");
            break;
        }

        if (strcmp(op, "ADD") == 0) {

            int id, pop;
            if (scanf("%d %d", &id, &pop) != 2) {
                fprintf(stderr, "ADD format is incorrect!\n");
                continue;
            }

            timeCounter++;
            add_book(shelf, capacity, &currentSize, id, pop, timeCounter);
        }

        else if (strcmp(op, "ACCESS") == 0) {

            int id;
            if (scanf("%d", &id) != 1) {
                fprintf(stderr, "ACCESS format is incorrect!\n");
                continue;
            }

            timeCounter++;
            printf("%d\n", access_book(shelf, currentSize, id, timeCounter));
        }

        else {
            fprintf(stderr, "Unknown command: %s\n", op);
        }
    }

    free(shelf);
    return 0;
}
