#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define INITIAL_CAPACITY 4

typedef struct {
    char **lines;
    size_t size;       
    size_t capacity;   
} Editor;

static void oom_exit(const char *msg)
{
    if (msg) perror(msg);
    else perror("Out of memory");
    exit(EXIT_FAILURE);
}
    
void initEditor(Editor *ed)
{
    ed->size = 0;
    ed->capacity = INITIAL_CAPACITY;
    ed->lines = malloc(ed->capacity * sizeof(char *));
    if (!ed->lines) oom_exit("malloc in initEditor"); 

    for (size_t i = 0; i < ed->capacity; ++i) ed->lines[i] = NULL;
}

void freeAll(Editor *ed)
{
    if (!ed) return;
    for (size_t i = 0; i < ed->size; ++i) {
        free(ed->lines[i]);
        ed->lines[i] = NULL;
    }
    free(ed->lines);
    ed->lines = NULL;
    ed->size = 0;
    ed->capacity = 0;
}
void ensureCapacity(Editor *ed, size_t minCap)
{
    if (ed->capacity >= minCap) return;

    size_t newCap = ed->capacity ? ed->capacity : 1;
    while (newCap < minCap) newCap <<= 1;

    char **tmp = realloc(ed->lines, newCap * sizeof(char *));
    if (!tmp) oom_exit("realloc");
    ed->lines = tmp;

    for (size_t i = ed->capacity; i < newCap; ++i) ed->lines[i] = NULL;
    ed->capacity = newCap;
}


void insertLine(Editor *ed, size_t index, const char *text)
{
    if (index > ed->size) {
        fprintf(stderr, "insertLine: invalid index %zu (size %zu)\n", index, ed->size);
        return;
    }
    ensureCapacity(ed, ed->size + 1);

    if (index < ed->size) {
        memmove(&ed->lines[index + 1], &ed->lines[index],
                (ed->size - index) * sizeof(char *));
    }
    size_t len = strlen(text);
    char *copy = malloc(len + 1);
    if (!copy) oom_exit("malloc");
    memcpy(copy, text, len + 1);

    ed->lines[index] = copy;
    ed->size++;
}

void deleteLine(Editor *ed, size_t index)
{
    if (index >= ed->size) {
        fprintf(stderr, "deleteLine: invalid index %zu (size %zu)\n", index, ed->size);
        return;
    }
    free(ed->lines[index]);
    if (index + 1 < ed->size) {
        memmove(&ed->lines[index], &ed->lines[index + 1],
                (ed->size - index - 1) * sizeof(char *));
    }
    ed->size--;
    ed->lines[ed->size] = NULL; 
}

void printAllLines(const Editor *ed)
{
    printf("--- Buffer Contents (%zu lines, capacity %zu) ---\n", ed->size, ed->capacity);
    for (size_t i = 0; i < ed->size; ++i) {
        printf("%zu: %s\n", i + 1, ed->lines[i]);
    }
    printf("-----------------------------------\n");
}
void shrinkToFit(Editor *ed)
{
    if (ed->size == ed->capacity) return;
    
    if (ed->size == 0) {
        char **tmp = realloc(ed->lines, INITIAL_CAPACITY * sizeof(char *));
        if (!tmp) oom_exit("realloc (shrink to fit zero)");
        ed->lines = tmp;
        for (size_t i = 0; i < INITIAL_CAPACITY; ++i) ed->lines[i] = NULL;
        ed->capacity = INITIAL_CAPACITY;
        return;
    }

    char **tmp = realloc(ed->lines, ed->size * sizeof(char *));
    if (!tmp) oom_exit("realloc");
    ed->lines = tmp;
    ed->capacity = ed->size;
}


int saveToFile(const Editor *ed, const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return -1;
    }
    for (size_t i = 0; i < ed->size; ++i) {
        if (fprintf(f, "%s\n", ed->lines[i]) < 0) {
            perror("fprintf");
            fclose(f);
            return -1;
        }
    }
    fclose(f);
    return 0;
}


int loadFromFile(Editor *ed, const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return -1;
    }

    for (size_t i = 0; i < ed->size; ++i) free(ed->lines[i]);
    ed->size = 0;
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread; 

    while ((nread = getline(&line, &len, f)) != -1){
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
            nread--;
        }
        ensureCapacity(ed, ed->size + 1);
        
        char *copy = malloc((size_t)nread + 1); 
        if (!copy) {
            free(line);
            fclose(f);
            oom_exit("malloc");
        }
        memcpy(copy, line, (size_t)nread + 1);
        ed->lines[ed->size++] = copy;
    }

    free(line);
    fclose(f);
    return 0;
}
void printHelp(void)
{
    puts("Commands:");
    puts("  a <index>     - insert (append if index == size+1). After pressing Enter the program will prompt for the text line.");
    puts("  d <index>     - delete line at index (1-based)");
    puts("  p             - print all lines (shows capacity)");
    puts("  s <filename>  - save to file");
    puts("  l <filename>  - load from file (replaces buffer)");
    puts("  r             - shrinkToFit (release unused memory)");
    puts("  q             - quit (frees memory)");
    puts("  h             - help");
}

char *readInputLine(const char *prompt)
{
    if (prompt) printf("%s", prompt);
    fflush(stdout);

    char *buf = NULL;
    size_t cap = 0;
    
    ssize_t nread = getline(&buf, &cap, stdin);
    if (nread == -1) {
        free(buf);
        return NULL;
    }
    if (nread > 0 && buf[nread - 1] == '\n') buf[nread - 1] = '\0';
    return buf;
}

int main(void)
{
    Editor ed;
    initEditor(&ed);

    printf("Lightweight command-line line editor\n");
    printHelp();

    char cmd[8];
    while (1) {
        printf("\n> ");
        if (scanf("%7s", cmd) != 1) break;

        int c; while ((c = getchar()) != '\n' && c != EOF);

        if (strcmp(cmd, "a") == 0) {
            long idx;
            if (scanf("%ld", &idx) != 1) {
                fprintf(stderr, "Invalid index\n");
                int c; while ((c = getchar()) != '\n' && c != EOF); 
                continue;
            }
            int c; while ((c = getchar()) != '\n' && c != EOF); 
            char *text = readInputLine("Enter text: ");
            if (!text) {
                puts("No input provided.");
                continue;
            }
            if (idx < 1) {
                fprintf(stderr, "Index must be >= 1\n");
                free(text);
                continue;
            }
            size_t insertPos = (size_t)(idx - 1);
            if (insertPos > ed.size) {
                fprintf(stderr, "Index too large (current size %zu). Use %zu to append.\n", ed.size, ed.size + 1);
                free(text);
                continue;
            }

            insertLine(&ed, insertPos, text);
            free(text);

            printf("Inserted at %zu. Size now: %zu\n", insertPos + 1, ed.size);
        }
        else if (strcmp(cmd, "d") == 0) {
            long idx;
            if (scanf("%ld", &idx) != 1) {
                fprintf(stderr, "Invalid index\n");
                int c; while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            if (idx < 1 || (size_t)idx > ed.size) {
                fprintf(stderr, "Invalid index (1..%zu)\n", ed.size);
                continue;
            }
            deleteLine(&ed, (size_t)idx - 1);
            printf("Deleted. Size now: %zu\n", ed.size);
        }
        else if (strcmp(cmd, "p") == 0) {
            printAllLines(&ed);
        }
        else if (strcmp(cmd, "s") == 0) {
            char filename[256];
            if (scanf("%255s", filename) != 1) {
                fprintf(stderr, "Invalid filename\n");
                continue;
            }
            if (saveToFile(&ed, filename) == 0) printf("Saved to %s\n", filename);
        }
        else if (strcmp(cmd, "l") == 0) {
            char filename[256];
            if (scanf("%255s", filename) != 1) {
                fprintf(stderr, "Invalid filename\n");
                continue;
            }
            if (loadFromFile(&ed, filename) == 0) {
                printf("Loaded %zu lines from %s\n", ed.size, filename);
            }
        }
        else if (strcmp(cmd, "r") == 0) {
            shrinkToFit(&ed);
            printf("ShrinkToFit: capacity now %zu\n", ed.capacity);
        }
        else if (strcmp(cmd, "q") == 0) {
            break;
        }
        else if (strcmp(cmd, "h") == 0) {
            printHelp();
        }
        else {
            printf("Unknown command '%s'. Type 'h' for help.\n", cmd);
            int c; while ((c = getchar()) != '\n' && c != EOF);
        }
    }

    freeAll(&ed);
    printf("Exited. Memory freed.\n");
    return 0;
}

/*  MEMORY MANAGEMENT EXPLANATION
Why dynamic allocation is more efficient than fixed-size rows.

Dynamic allocation is more efficient than fixed-size rows because it allocates memory only for the exact
amount of text the user enters instead of reserving a large buffer for every row whether it is used or not.
This reduces wasted memory, allows the editor to handle lines of any length, and lets the program grow or
shrink its storage as needed. With fixed-size rows, space is consumed even for short lines,
and very long lines cannot fit, making the program less flexible and less memory-efficient.
*/