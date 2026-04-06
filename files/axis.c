/*
 * ============================================================
 *  AXIX - Word Search & Dictionary
 *  (Simplified + Original Output Format Preserved)
 *
 *
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ENTRIES 2000
#define MAX_WORD 100
#define MAX_MEANING 512
#define MAX_LINE 1024
#define MAX_FILES 10
#define MAX_PATH 260

#define DICT_FILE "dictionary.txt"
#define HISTORY_FILE "search_history.txt"   //  NEW

#define CLR_RESET   "\033[0m"
#define CLR_BOLD    "\033[1m"
#define CLR_RED     "\033[31m"
#define CLR_GREEN   "\033[32m"
#define CLR_YELLOW  "\033[33m"
#define CLR_CYAN    "\033[36m"

typedef struct {
    char word[MAX_WORD];
    char meaning[MAX_MEANING];
} Entry;

Entry dict[MAX_ENTRIES];
int dictSize = 0;

char fileList[MAX_FILES][MAX_PATH];
int fileCount = 0;

/* ================= HELPER ================= */

void toLowerStr(char *s) {
    for (int i = 0; s[i]; i++)
        s[i] = tolower(s[i]);
}

void trim(char *s) {
    int start = 0, end = strlen(s) - 1;
    while (isspace(s[start])) start++;
    while (end >= start && isspace(s[end])) end--;

    int j = 0;
    for (int i = start; i <= end; i++)
        s[j++] = s[i];
    s[j] = '\0';
}

void flushInput() {
    while (getchar() != '\n');
}

char* findSubstring(const char *text, const char *word) {
    static char t[MAX_LINE], w[MAX_WORD];
    strcpy(t, text);
    strcpy(w, word);
    toLowerStr(t);
    toLowerStr(w); 
    return strstr(t, w);
}

void printLine(char ch) {
    for (int i = 0; i < 44; i++) putchar(ch);
    printf("\n");
}

/* ================= SEARCH HISTORY ================= */

void saveSearchHistory(const char *query) {
    FILE *fp = fopen(HISTORY_FILE, "a"); // append
    if (!fp) return; // no output change

    fprintf(fp, "%s\n", query);
    fclose(fp);
}

/* ================= FILE ================= */

void addFile(const char *path) {
    if (fileCount >= MAX_FILES) return;

    for (int i = 0; i < fileCount; i++)
        if (strcmp(fileList[i], path) == 0) return;

    strcpy(fileList[fileCount++], path);
}

void createIfMissing(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp) { fclose(fp); return; }

    fp = fopen(path, "w");
    fprintf(fp, "Hello world from file one.\nThis is a test file.\n");
    fclose(fp);
}

/* ================= DICTIONARY ================= */

void loadDictionary() {
    FILE *fp = fopen(DICT_FILE, "r");
    if (!fp) {
        printf(CLR_RED "  [!] dictionary.txt not found.\n" CLR_RESET);
        return;
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        trim(line);
        trim(colon + 1);

        strcpy(dict[dictSize].word, line);
        strcpy(dict[dictSize].meaning, colon + 1);
        dictSize++;
    }

    fclose(fp);

    printf(CLR_GREEN "  Dictionary: %d words loaded.\n" CLR_RESET, dictSize);
}

/* ================= SEARCH ================= */

int searchFiles(const char *query) {
    int total = 0;

    for (int i = 0; i < fileCount; i++) {
        FILE *fp = fopen(fileList[i], "r");
        if (!fp) continue;

        char line[MAX_LINE];
        int lineNum = 0, hits = 0;

        while (fgets(line, sizeof(line), fp)) {
            lineNum++;

            if (findSubstring(line, query)) {
                if (hits == 0) {
                    printf(CLR_CYAN "\n  File: %s\n" CLR_RESET, fileList[i]);
                    printLine('-');
                }

                printf(CLR_GREEN "  Line %3d | " CLR_RESET, lineNum);
                printf("%s", line);

                hits++;
                total++;
            }
        }

        if (hits > 0)
            printf(CLR_CYAN "  Matches in this file: %d\n" CLR_RESET, hits);

        fclose(fp);
    }

    return total;
}

/* ================= DICTIONARY LOOKUP ================= */

int lookupExact(const char *query) {
    char q[MAX_WORD];
    strcpy(q, query);
    toLowerStr(q);

    for (int i = 0; i < dictSize; i++) {
        char w[MAX_WORD];
        strcpy(w, dict[i].word);
        toLowerStr(w);

        if (strcmp(w, q) == 0) {
            printLine('-');
            printf(CLR_YELLOW "  Word    : " CLR_RESET CLR_BOLD "%s\n" CLR_RESET, dict[i].word);
            printf(CLR_YELLOW "  Meaning : " CLR_RESET "%s\n", dict[i].meaning); 
            printLine('-');
            return 1;
        }
    }
    return 0;
}

void lookupPartial(const char *query) {
    int found = 0;

    for (int i = 0; i < dictSize && found < 10; i++) {
        if (findSubstring(dict[i].word, query)) {
            printf("  " CLR_YELLOW "%-22s" CLR_RESET ": %s\n",
                   dict[i].word, dict[i].meaning);
            found++;
        }
    }

    if (!found)
        printf("  No related words found.\n");
}

/* ================= DRIVER ================= */

void runSearch() {
    char query[MAX_WORD];

    printf("\n  Enter word or phrase: ");
    flushInput();
    fgets(query, sizeof(query), stdin);

    query[strcspn(query, "\n")] = 0;
    trim(query);

    saveSearchHistory(query);   //  NEW LINE

    printf(CLR_CYAN "\n  SEARCHING FILES FOR: \"%s\"\n" CLR_RESET, query);
    printLine('=');

    int total = searchFiles(query);

    if (!total)
        printf("  No matches found in any file.\n");
    else
        printf(CLR_GREEN "\n  Total matches: %d\n" CLR_RESET, total);

    printf(CLR_CYAN "\n  DICTIONARY LOOKUP\n" CLR_RESET);
    printLine('=');

    if (!lookupExact(query)) {
        printf("  No exact match. Related words:\n\n");
        lookupPartial(query);
    }

    printLine('=');
}

/* ================= VIEW FILE ================= */

void viewFile() {
    if (!fileCount) {
        printf(CLR_RED "  No files registered.\n" CLR_RESET);
        return;
    }

    for (int i = 0; i < fileCount; i++)
        printf("  [%d] %s\n", i + 1, fileList[i]);

    printf("  Enter number: ");
    int ch;
    scanf("%d", &ch);

    FILE *fp = fopen(fileList[ch - 1], "r");
    if (!fp) return;

    char line[MAX_LINE];
    int n = 1;

    printf(CLR_CYAN "\n  -- %s --\n" CLR_RESET, fileList[ch - 1]);
    printLine('-');

    while (fgets(line, sizeof(line), fp))
        printf("  %4d  %s", n++, line);

    printLine('-');

    fclose(fp);
}

/* ================= MENU ================= */

void printMenu() {
    printf(CLR_CYAN
    "\n  +------------------------------------------+\n"
    "  |        AXIX  -  Search & Dictionary     |\n"
    "  +------------------------------------------+\n" CLR_RESET);

    printf("  |  Files indexed    : %-5d                |\n", fileCount);
    printf("  |  Dictionary words : %-5d                |\n", dictSize);

    printf(CLR_CYAN
    "  +------------------------------------------+\n"
    "  |  1. Search word or phrase                |\n"
    "  |  2. View file contents                   |\n"
    "  |  0. Exit                                 |\n"
    "  +------------------------------------------+\n" CLR_RESET);

    printf("  Choice: ");
}

/* ================= MAIN ================= */

int main() {

    printf(CLR_CYAN
    "\n  +------------------------------------------+\n"
    "  |           AXIX  -  Starting Up          |\n"
    "  +------------------------------------------+\n" CLR_RESET);

    createIfMissing("file1.txt");
    addFile("file1.txt");

    loadDictionary();

    int choice;

    while (1) {
        printMenu();

        if (scanf("%d", &choice) != 1) {
            flushInput();
            continue;
        }

        switch (choice) {
            case 1: runSearch(); break;
            case 2: viewFile(); break;
            case 0:
                printf(CLR_GREEN "\n  Goodbye!\n\n" CLR_RESET);
                return 0;
            default:
                printf(CLR_RED "  Invalid option.\n" CLR_RESET);
        }
    }
}
