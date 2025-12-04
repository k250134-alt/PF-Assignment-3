#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h> 

#define DATAFILE "members.dat"
#define NAME_LEN 100
#define DATE_LEN 11  
#define INITIAL_CAPACITY 8

// --- Data Structure ---
typedef struct {
    int id;                     
    char name[NAME_LEN];        
    char batch[32];             
    char membership[8];         
    char regDate[DATE_LEN];     
    char dob[DATE_LEN];         
    char interest[8];           
} Student;

// --- Global Data ---
Student *students = NULL;
size_t studentCount = 0;
size_t studentCapacity = 0;

// --- Utility Functions ---

// Removes trailing newline from a string
static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    if (n == 0) return;
    if (s[n-1] == '\n') s[n-1] = '\0';
}

// Resizes the student array if necessary
static void ensure_capacity(size_t minCap) {
    if(studentCapacity >= minCap) return;
    size_t newCap = studentCapacity ? studentCapacity : INITIAL_CAPACITY;
    while (newCap < minCap) newCap <<= 1;

    Student *tmp = realloc(students, newCap * sizeof(Student));
    if (!tmp) {
        perror("realloc");
        fprintf(stderr, "Fatal: Out of memory while resizing student array.\n");
        exit(EXIT_FAILURE);
    }
    students = tmp;
    studentCapacity = newCap;
}

// Finds the index of a student by ID
static long findStudentIndexByID(int id) {
    for (size_t i = 0; i < studentCount; ++i) {
        if (students[i].id == id) return (long)i;
    }
    return -1;
}

// Helper function to read a line of input
static void read_line_input(const char *prompt, char *out, size_t outLen) {
    printf("%s", prompt);
    fflush(stdout); // Ensure prompt is displayed before input
    if (!fgets(out, (int)outLen, stdin)) {
        out[0] = '\0';
        return;
    }
    trim_newline(out);
}

// --- Validation Functions ---

static int validBatch(const char *batch) {
    if (!batch) return 0;
    return (strcmp(batch, "CS") == 0 || strcmp(batch, "SE") == 0 || 
            strcmp(batch, "Cyber Security") == 0 || strcmp(batch, "AI") == 0);
}

static int validMembership(const char *m) {
    if (!m) return 0;
    return (strcmp(m, "IEEE") == 0 || strcmp(m, "ACM") == 0);
}

static int validInterest(const char *s) {
    if (!s) return 0;
    return (strcmp(s, "IEEE") == 0 || strcmp(s, "ACM") == 0 || strcmp(s, "Both") == 0);
}

// Checks if a year is a leap year
static int is_leap(int year) {
    // Leap year if divisible by 4, unless divisible by 100 but not 400.
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Full calendar validation for YYYY-MM-DD format
static int validDateFormat(const char *d) {
    if (!d || strlen(d) != 10) return 0;
    
    int year, month, day;
    
    // 1. Check structure and parse values
    if (sscanf(d, "%d-%d-%d", &year, &month, &day) != 3) {
        return 0; // Failed to parse YYYY-MM-DD pattern
    }
    
    // 2. Check reasonable year range (adjust bounds as needed)
    if (year < 1900 || year > 2100) return 0; 

    // 3. Check month range
    if (month < 1 || month > 12) return 0;

    // 4. Check day range based on month and leap year
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month == 2) {
        if (is_leap(year)) days_in_month[2] = 29;
    }
    
    if (day < 1 || day > days_in_month[month]) return 0;

    return 1;
}

// --- Core Logic Functions ---

static void printStudent(const Student *s) {
    if (!s) return;
    printf("\nID: %d\nName: %s\nBatch: %s\nMembership: %s\nRegDate: %s\nDOB: %s\nInterest: %s\n",
           s->id, s->name, s->batch, s->membership, s->regDate, s->dob, s->interest);
}

int loadDatabase(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        if (errno == ENOENT) {
            studentCount = 0;
            return 0;
        }
        perror("fopen");
        fprintf(stderr, "Error: could not open '%s' for reading.\n", filename);
        return -1;
    }

    studentCount = 0;
    Student tmp;
    while (fread(&tmp, sizeof(Student), 1, f) == 1) {
        ensure_capacity(studentCount + 1);
        students[studentCount++] = tmp;
    }

    if (ferror(f)) {
        perror("fread");
        fclose(f);
        return -1;
    }

    if (fclose(f) != 0) {
        perror("fclose");
        return -1;
    }
    return 0;
}

int saveDatabase(const char *filename) {
    char tmpName[512];
    snprintf(tmpName, sizeof(tmpName), "%s.tmp", filename);

    FILE *f = fopen(tmpName, "wb");
    if (!f) {
        perror("fopen");
        fprintf(stderr, "Error: could not open temporary file for writing.\n");
        return -1;
    }

    if (fwrite(students, sizeof(Student), studentCount, f) != studentCount) {
        perror("fwrite");
        fclose(f);
        remove(tmpName);
        return -1;
    }

    if (fflush(f) != 0) { perror("fflush"); fclose(f); remove(tmpName); return -1; }
    if (fclose(f) != 0) { perror("fclose"); remove(tmpName); return -1; }

    remove(filename); 
    if (rename(tmpName, filename) != 0) {
        perror("rename");
        fprintf(stderr, "Error: could not replace database file.\n");
        remove(tmpName);
        return -1;
    }

    return 0;
}

int addStudent(const Student *s, const char *filename) {
    if (!s) return -1;

    if (findStudentIndexByID(s->id) != -1) {
        fprintf(stderr, "Error: Student ID %d already exists. Registration failed.\n", s->id);
        return -1;
    }
    
    // Add to file (for persistence)
    FILE *f = fopen(filename, "ab"); 
    if (!f) {
        perror("fopen");
        fprintf(stderr, "Error: cannot open data file to append.\n");
        return -1;
    }
    if (fwrite(s, sizeof(Student), 1, f) != 1) {
        perror("fwrite");
        fclose(f);
        return -1;
    }
    if (fclose(f) != 0) {
        perror("fclose");
        return -1;
    }
    
    // Add to in-memory array
    ensure_capacity(studentCount + 1);
    students[studentCount++] = *s;
    return 0;
}

int updateStudent(int studentID) {
    long idx = findStudentIndexByID(studentID);
    if (idx == -1) {
        fprintf(stderr, "Student ID %d not found.\n", studentID);
        return -1;
    }

    Student *s = &students[idx];
    char buf[128];

    printf("Current record:\n");
    printStudent(s);

    // Clear buffer after previous scanf (if any)
    int c; while ((c = getchar()) != '\n' && c != EOF); 
    
    read_line_input("Enter new batch (CS / SE / Cyber Security / AI) or press Enter to keep: ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        if (!validBatch(buf)) {
            fprintf(stderr, "Invalid batch string. Update aborted.\n");
            return -1;
        }
        strncpy(s->batch, buf, sizeof(s->batch)-1);
        s->batch[sizeof(s->batch)-1] = '\0';
    }

    read_line_input("Enter new membership (IEEE / ACM) or press Enter to keep: ", buf, sizeof(buf));
    if (strlen(buf) > 0) {
        if (!validMembership(buf)) {
            fprintf(stderr, "Invalid membership. Update aborted.\n");
            return -1;
        }
        strncpy(s->membership, buf, sizeof(s->membership)-1);
        s->membership[sizeof(s->membership)-1] = '\0';
    }
    
    if (saveDatabase(DATAFILE) != 0) {
        fprintf(stderr, "Warning: update succeeded in memory but failed to save to disk.\n");
        return -1;
    }

    printf("Student ID %d updated successfully.\n", studentID);
    return 0;
}

int deleteStudent(int studentID) {
    long idx = findStudentIndexByID(studentID);
    if (idx == -1) {
        fprintf(stderr, "Student ID %d not found.\n", studentID);
        return -1;
    }

    for (size_t i = (size_t)idx; i + 1 < studentCount; ++i) {
        students[i] = students[i+1];
    }
    studentCount--;

    // Optional: Shrink array capacity
    if (studentCapacity > INITIAL_CAPACITY && studentCount * 4 < studentCapacity) {
        size_t newCap = studentCapacity / 2;
        if (newCap < INITIAL_CAPACITY) newCap = INITIAL_CAPACITY;
        Student *tmp = realloc(students, newCap * sizeof(Student));
        if (tmp) {
            students = tmp;
            studentCapacity = newCap;
        }
    }

    if (saveDatabase(DATAFILE) != 0) {
        fprintf(stderr, "Warning: deletion succeeded in memory but failed to save to disk.\n");
        return -1;
    }

    printf("Student ID %d deleted successfully.\n", studentID);
    return 0;
}

void displayAllStudents(void) {
    if (studentCount == 0) {
        printf("No students found.\n");
        return;
    }
    for (size_t i = 0; i < studentCount; ++i) {
        printf("---- Record %zu ----", i+1);
        printStudent(&students[i]);
    }
}

void generateBatchReport(const char *batchFilter, const char *membershipFilter) {
    int found = 0;
    
    if (!validBatch(batchFilter)) {
        fprintf(stderr, "Invalid batch filter: %s\n", batchFilter);
        return;
    }
    
    if (strlen(membershipFilter) > 0 && !validInterest(membershipFilter)) {
        fprintf(stderr, "Invalid membership filter: %s (Must be IEEE, ACM, or Both)\n", membershipFilter);
        return;
    }

    printf("\n--- Batch Report (Batch: %s, Filter: %s) ---\n", 
           batchFilter, 
           strlen(membershipFilter) == 0 ? "Any" : membershipFilter);

    for (size_t i = 0; i < studentCount; ++i) {
        const Student *s = &students[i];
        
        if (strcmp(s->batch, batchFilter) != 0) continue;
        
        int membershipMatch = 0;
        if (strlen(membershipFilter) == 0) {
            membershipMatch = 1; 
        } else if (strcmp(membershipFilter, "Both") == 0) {
            if (strcmp(s->interest, "Both") == 0) membershipMatch = 1;
        } else {
            // Check if student's current membership or interest includes the filter
            if (strcmp(s->membership, membershipFilter) == 0 || strcmp(s->interest, membershipFilter) == 0 || strcmp(s->interest, "Both") == 0) {
                membershipMatch = 1;
            }
        }
        
        if (membershipMatch) {
            printStudent(s);
            found = 1;
        }
    }
    if (!found) {
        printf("No records matching the specified criteria.\n");
    }
    printf("---------------------------------------------\n");
}


// Function for interactive registration with robust input loops
void registerStudentInteractive(void) {
    Student s;
    int tmp_id;

    printf("\n--- New Student Registration ---\n");
    printf("Enter Student ID (integer): ");
    
    // Use scanf for ID, then aggressively clear the input buffer
    if (scanf("%d", &tmp_id) != 1) {
        fprintf(stderr, "Invalid ID input. Registration aborted.\n");
        int c; while ((c = getchar()) != '\n' && c != EOF); 
        return;
    }
    s.id = tmp_id;
    int c; while ((c = getchar()) != '\n' && c != EOF); // Clear buffer after scanf
    // Optional: Add a non-standard but often effective extra clear
    fflush(stdin); 

    if (findStudentIndexByID(s.id) != -1) {
        fprintf(stderr, "Error: Student with ID %d already exists.\n", s.id);
        return;
    }

    char input_buffer[NAME_LEN];
    int valid = 0;

    // 1. Full Name (NOW IN ROBUST LOOP to prevent skips)
    valid = 0;
    while(!valid) {
        read_line_input("Full Name: ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Full Name cannot be empty. Try again.\n");
            continue;
        }
        strncpy(s.name, input_buffer, sizeof(s.name) - 1);
        s.name[sizeof(s.name) - 1] = '\0';
        valid = 1;
    }
    
    // 2. Batch (Input loop ensures valid selection)
    valid = 0;
    while(!valid) {
        read_line_input("Batch (CS / SE / Cyber Security / AI): ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Batch cannot be empty. Try again.\n");
            continue;
        }
        if (!validBatch(input_buffer)) { fprintf(stderr, "Invalid batch. Try again.\n"); continue; }
        strncpy(s.batch, input_buffer, sizeof(s.batch) - 1);
        s.batch[sizeof(s.batch) - 1] = '\0';
        valid = 1;
    }

    // 3. Membership (Input loop ensures valid selection)
    valid = 0;
    while(!valid) {
        read_line_input("Membership (IEEE / ACM): ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Membership cannot be empty. Try again.\n");
            continue;
        }
        if (!validMembership(input_buffer)) { fprintf(stderr, "Invalid membership. Try again.\n"); continue; }
        strncpy(s.membership, input_buffer, sizeof(s.membership) - 1);
        s.membership[sizeof(s.membership) - 1] = '\0';
        valid = 1;
    }

    // 4. Registration Date (Input loop ensures valid YYYY-MM-DD date)
    valid = 0;
    while(!valid) {
        read_line_input("Registration Date (YYYY-MM-DD): ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Registration Date cannot be empty. Try again.\n");
            continue;
        }
        if (!validDateFormat(input_buffer)) { 
            fprintf(stderr, "Invalid date format or non-existent date (e.g., Feb 30th). Required: YYYY-MM-DD. Try again.\n"); 
            continue; 
        }
        strncpy(s.regDate, input_buffer, sizeof(s.regDate) - 1);
        s.regDate[sizeof(s.regDate) - 1] = '\0';
        valid = 1;
    }
    
    // 5. Date of Birth (Input loop ensures valid YYYY-MM-DD date)
    valid = 0;
    while(!valid) {
        read_line_input("Date of Birth (YYYY-MM-DD): ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Date of Birth cannot be empty. Try again.\n");
            continue; // Loop again until valid input is given
        }
        if (!validDateFormat(input_buffer)) { 
            fprintf(stderr, "Invalid date format or non-existent date (e.g., Feb 30th). Required: YYYY-MM-DD. Try again.\n"); 
            continue; 
        }
        strncpy(s.dob, input_buffer, sizeof(s.dob) - 1);
        s.dob[sizeof(s.dob) - 1] = '\0';
        valid = 1;
    }

    // 6. Interest (Input loop ensures valid selection)
    valid = 0;
    while(!valid) {
        read_line_input("Interest (IEEE / ACM / Both): ", input_buffer, sizeof(input_buffer));
        if (strlen(input_buffer) == 0) { 
            fprintf(stderr, "Interest cannot be empty. Try again.\n");
            continue;
        }
        if (!validInterest(input_buffer)) { fprintf(stderr, "Invalid interest option. Try again.\n"); continue; }
        strncpy(s.interest, input_buffer, sizeof(s.interest) - 1);
        s.interest[sizeof(s.interest) - 1] = '\0';
        valid = 1;
    }

    if (addStudent(&s, DATAFILE) == 0) {
        printf("Student ID %d registered successfully.\n", s.id);
    } else {
        fprintf(stderr, "Failed to register student.\n");
    }
}


static Student createMockStudent(int id) {
    static const char *names[] = {"Aamir Khan", "Sara Ali", "Usman Tariq", "Javeria Nasir", "Zainab Raza", "Tariq Ali", "Ayesha Ahmed"};
    static const char *batches[] = {"CS", "SE", "Cyber Security", "AI"};
    static const char *memberships[] = {"IEEE", "ACM"};
    static const char *interests[] = {"IEEE", "ACM", "Both"};

    Student s;
    s.id = id;
    
    strncpy(s.name, names[rand() % (sizeof(names) / sizeof(names[0]))], NAME_LEN - 1);
    s.name[NAME_LEN - 1] = '\0';
    strncpy(s.batch, batches[rand() % (sizeof(batches) / sizeof(batches[0]))], 31);
    s.batch[31] = '\0';
    strncpy(s.membership, memberships[rand() % (sizeof(memberships) / sizeof(memberships[0]))], 7);
    s.membership[7] = '\0';
    strncpy(s.interest, interests[rand() % (sizeof(interests) / sizeof(interests[0]))], 7);
    s.interest[7] = '\0';

    snprintf(s.regDate, DATE_LEN, "2024-09-%02d", (rand() % 30) + 1);
    snprintf(s.dob, DATE_LEN, "2000-01-%02d", (rand() % 28) + 1);

    return s;
}

void runStressTest(void) {
    srand(time(NULL)); 
    const int initial_id = 9000;
    
    printf("\n--- Running Stress Test (Task 5) ---\n");
    size_t initial_count = studentCount;

    // 1. Add 20–30 student records
    int count_to_add = (rand() % 11) + 20; 
    printf("1. Adding %d mock records...\n", count_to_add);
    for (int i = 0; i < count_to_add; ++i) {
        int id = initial_id + i;
        Student s = createMockStudent(id);
        addStudent(&s, DATAFILE);
    }
    printf("   -> Added %zu records. Total records: %zu.\n", studentCount - initial_count, studentCount);

    // 2. Delete 5–10 random students.
    int count_to_delete = (rand() % 6) + 5; 
    printf("2. Deleting %d random mock records...\n", count_to_delete);
    
    for (int i = 0; i < count_to_delete; ++i) {
        if (studentCount == 0) break;
        
        size_t index_to_delete = rand() % studentCount;
        int id_to_delete = students[index_to_delete].id;

        deleteStudent(id_to_delete); 
    }
    printf("   -> Deleted records. Total records: %zu.\n", studentCount);

    // 3. Update 5 random records.
    int count_to_update = 5;
    printf("3. Updating %d random records...\n", count_to_update);
    
    const char *new_batch = "AI";
    const char *new_membership = "ACM";

    for (int i = 0; i < count_to_update; ++i) {
        if (studentCount == 0) break;
        
        size_t index_to_update = rand() % studentCount;
        Student *s = &students[index_to_update];
        
        // Ensure values are changed
        if (strcmp(s->batch, "AI") != 0) {
             strncpy(s->batch, new_batch, sizeof(s->batch));
        } else {
             strncpy(s->batch, "CS", sizeof(s->batch));
        }

        if (strcmp(s->membership, "ACM") != 0) {
            strncpy(s->membership, new_membership, sizeof(s->membership));
        } else {
             strncpy(s->membership, "IEEE", sizeof(s->membership));
        }
        
        // Note: The original logic in updateStudent saves the database. Here we save manually.
        saveDatabase(DATAFILE); 
    }
    printf("   -> Updated and saved %d records.\n", count_to_update);

    // 4. Verify that file integrity is maintained 
    printf("4. Verifying file integrity...\n");
    
    size_t temp_count = 0;
    FILE *f_test = fopen(DATAFILE, "rb");
    if (f_test) {
        Student tmp;
        while (fread(&tmp, sizeof(Student), 1, f_test) == 1) temp_count++;
        fclose(f_test);
    }

    if (temp_count == studentCount) {
        printf("   -> Verification SUCCEEDED: Memory count (%zu) matches File count (%zu).\n", studentCount, temp_count);
    } else {
        printf("   -> Verification FAILED: Memory count (%zu) DOES NOT match File count (%zu).\n", studentCount, temp_count);
    }
    printf("--- Stress Test Complete ---\n");
}


void printMenu(void) {
    puts("\n--- FAST University Membership System ---");
    puts("1. Register new student");
    puts("2. Update existing student (batch / membership)");
    puts("3. Delete a student");
    puts("4. View all registrations");
    puts("5. Generate batch-wise report");
    puts("6. Run Stress Test (Add, Delete, Update Mock Data)");
    puts("7. Exit");
    printf("Enter choice: ");
}

int main(void) {
    students = NULL;
    studentCount = 0;
    studentCapacity = 0;
    ensure_capacity(INITIAL_CAPACITY);
    
    printf("\nInitializing FAST University Membership Manager...\n");

    if (loadDatabase(DATAFILE) != 0) {
        fprintf(stderr, "Warning: could not load database file '%s'. Starting with empty DB.\n", DATAFILE);
    } else {
        printf("Successfully loaded %zu existing records from '%s'.\n", studentCount, DATAFILE);
    }

    int choice = 0;
    while (1) {
        printMenu();
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Invalid choice input. Please enter a number.\n");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF); 
            continue;
        }
        int ch; while ((ch = getchar()) != '\n' && ch != EOF); 

        if (choice == 1) {
            registerStudentInteractive();
        } else if (choice == 2) {
            printf("Enter Student ID to update: ");
            int id;
            if (scanf("%d", &id) != 1) { fprintf(stderr, "Invalid ID\n"); int c; while ((c = getchar()) != '\n' && c != EOF); continue; }
            updateStudent(id);
        } else if (choice == 3) {
            printf("Enter Student ID to delete: ");
            int id;
            if (scanf("%d", &id) != 1) { fprintf(stderr, "Invalid ID\n"); int c; while ((c = getchar()) != '\n' && c != EOF); continue; }
            deleteStudent(id);
        } else if (choice == 4) {
            displayAllStudents();
        } else if (choice == 5) {
            char batch[64], membership[16];
            read_line_input("Enter batch (CS / SE / Cyber Security / AI): ", batch, sizeof(batch));
            read_line_input("Membership filter (IEEE / ACM / Both) or press Enter for any interest: ", membership, sizeof(membership));
            generateBatchReport(batch, membership);
        } else if (choice == 6) { 
            runStressTest();
        } else if (choice == 7) { 
            if (saveDatabase(DATAFILE) != 0) {
                fprintf(stderr, "Error: failed to save database before exit. Exiting anyway.\n");
            } else {
                printf("Database saved to '%s'.\n", DATAFILE);
            }
            break;
        } else {
            printf("Unknown option.\n");
        }
    }

    free(students);
    students = NULL;
    studentCapacity = 0;
    studentCount = 0;
    printf("\nExiting program. Memory freed.\n");
    return 0;
}