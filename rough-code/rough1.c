#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

// ── Limits ───────────────────────────────────────────────────
#define MAX_FILES 20
#define MAX_LINE 1024
#define MAX_WORD_LEN 100
#define MAX_MEANING_LEN 500
#define MAX_DICT_ENTRIES 1000

// ── File names ───────────────────────────────────────────────
#define DICT_FILE "dictionary.txt"
#define HISTORY_FILE "history.txt"
#define INDEX_FILE "index.txt"

// ── ANSI Colors ──────────────────────────────────────────────
#define RED    "\033[1;31m"
#define GREEN  "\033[1;32m"
#define YELLOW "\033[1;33m"
#define CYAN   "\033[1;36m"
#define BOLD   "\033[1m"
#define RESET  "\033[0m"

// ── Structs ──────────────────────────────────────────────────
typedef struct {
    char fileName[100];
    int occurrences;
} SearchResult;

typedef struct {
    char word[MAX_WORD_LEN];
    char meaning[MAX_MEANING_LEN];
} DictEntry;

// ── Globals ──────────────────────────────────────────────────
char files[MAX_FILES][100];
int numFiles = 0;

DictEntry dictionary[MAX_DICT_ENTRIES];
int dictCount = 0;

// ═══════════════════════════════════════════════════════════
// UTILITIES
// ═══════════════════════════════════════════════════════════

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void toLowerCase(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower((unsigned char)str[i]);
}

void trim(char *str) {
    int start = 0;
    while (isspace((unsigned char)str[start])) start++;
    memmove(str, str + start, strlen(str + start) + 1);

    int end = strlen(str) - 1;
    while (end >= 0 && isspace((unsigned char)str[end]))
        str[end--] = '\0';
}


// ═══════════════════════════════════════════════════════════
// FILE DISCOVERY
// ═══════════════════════════════════════════════════════════

void discoverFiles() {
    DIR *d = opendir(".");
    struct dirent *dir;
    numFiles = 0;

    while ((dir = readdir(d)) != NULL && numFiles < MAX_FILES) {
        char *ext = strrchr(dir->d_name, '.');
        if (ext && strcmp(ext, ".txt") == 0) {
            if (strcmp(dir->d_name, DICT_FILE) == 0) continue;
            if (strcmp(dir->d_name, HISTORY_FILE) == 0) continue;
            if (strcmp(dir->d_name, INDEX_FILE) == 0) continue;

            strcpy(files[numFiles++], dir->d_name);
        }
    }
    closedir(d);

    // 🔥 If no files found → auto create
    if (numFiles == 0) {
        printf(YELLOW "\nNo text files found!\n" RESET);
        createSampleFiles();
        discoverFiles(); // reload again
    }
}

// ═══════════════════════════════════════════════════════════
// SEARCH ENGINE
// ═══════════════════════════════════════════════════════════

int searchInFile(const char *filename, const char *query) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[MAX_LINE], lowerLine[MAX_LINE], lowerQuery[MAX_WORD_LEN];
    strcpy(lowerQuery, query);
    toLowerCase(lowerQuery);

    int count = 0, lineNum = 1;

    while (fgets(line, sizeof(line), fp)) {
        strcpy(lowerLine, line);
        toLowerCase(lowerLine);

        if (strstr(lowerLine, lowerQuery)) {
            printf(GREEN "[%s]" RESET " Line %d: %s", filename, lineNum, line);
            count++;
        }
        lineNum++;
    }

    fclose(fp);
    return count;
}

void searchEngine() {
    char query[100];

    printf("\nEnter search term: ");
    clearBuffer();
    fgets(query, sizeof(query), stdin);
    query[strcspn(query, "\n")] = 0;

    int total = 0;

    for (int i = 0; i < numFiles; i++) {
        total += searchInFile(files[i], query);
    }

    printf("\nTotal matches: %d\n", total);
}

// ═══════════════════════════════════════════════════════════
// DICTIONARY
// ═══════════════════════════════════════════════════════════

void loadDictionary() {
    FILE *fp = fopen(DICT_FILE, "r");
    if (!fp) return;

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp)) {
        char *sep = strstr(line, " : ");
        if (!sep) continue;

        *sep = '\0';
        strcpy(dictionary[dictCount].word, line);
        strcpy(dictionary[dictCount].meaning, sep + 3);
        trim(dictionary[dictCount].meaning);

        dictCount++;
    }

    fclose(fp);
}

void lookupWord() {
    char word[MAX_WORD_LEN];

    printf("\nEnter word: ");
    clearBuffer();
    fgets(word, sizeof(word), stdin);
    word[strcspn(word, "\n")] = 0;

    for (int i = 0; i < dictCount; i++) {
        if (strcasecmp(dictionary[i].word, word) == 0) {
            printf(GREEN "%s: %s\n" RESET, dictionary[i].word, dictionary[i].meaning);
            return;
        }
    }

    printf(YELLOW "Word not found\n" RESET);
}

void addWord() {
    char word[MAX_WORD_LEN], meaning[MAX_MEANING_LEN];

    printf("\nEnter word: ");
    clearBuffer();
    fgets(word, sizeof(word), stdin);
    word[strcspn(word, "\n")] = 0;

    printf("Enter meaning: ");
    fgets(meaning, sizeof(meaning), stdin);
    meaning[strcspn(meaning, "\n")] = 0;

    FILE *fp = fopen(DICT_FILE, "a");
    fprintf(fp, "%s : %s\n", word, meaning);
    fclose(fp);

    printf(GREEN "Word added!\n" RESET);
}

// ═══════════════════════════════════════════════════════════
// MENU
// ═══════════════════════════════════════════════════════════

void printMenu() {
    printf(CYAN "\n+===========================================+\n" RESET);
    printf(CYAN "|        LEXIS - SEARCH SYSTEM              |\n" RESET);
    printf(CYAN "+===========================================+\n" RESET);

    printf("| Files Indexed    : %-5d                |\n", numFiles);
    printf("| Dictionary Words : %-5d                |\n", dictCount);

    printf("+-------------------------------------------+\n");
    printf("| 1. Search files                          |\n");
    printf("| 2. Refresh file list                     |\n");
    printf("| 3. Lookup word                           |\n");
    printf("| 4. Add word                              |\n");
    printf("| 0. Exit                                  |\n");
    printf("+-------------------------------------------+\n");

    printf("Enter choice: ");
}

// ═══════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════

int main() {
    system(""); // enable color

    discoverFiles();
    loadDictionary();

    int choice;

    while (1) {
        printMenu();

        if (scanf("%d", &choice) != 1) {
            clearBuffer();
            continue;
        }

        switch (choice) {
            case 1: searchEngine(); break;
            case 2: discoverFiles(); break;
            case 3: lookupWord(); break;
            case 4: addWord(); break;
            case 0: exit(0);
            default: printf("Invalid choice\n");
        }
    }
}