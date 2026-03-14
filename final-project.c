#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define MAX_FILES 20
#define MAX_LINE 1024
#define MAX_WORDS 20
#define MAX_WORD_LEN 50

// ANSI color for highlight
#define RED "\033[1;31m"
#define RESET "\033[0m"

typedef struct {
    char fileName[100];
    int occurrences;
} SearchResult;

// Global storage for files found in the directory
char files[MAX_FILES][100];
int numFiles = 0;

// Convert string to lowercase
void toLowerCase(char *str) {
    if (!str) return;
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
}

// Automatically find .txt files in the current directory
void discoverFiles() {
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    numFiles = 0;

    if (d) {
        while ((dir = readdir(d)) != NULL && numFiles < MAX_FILES) {
            char *ext = strrchr(dir->d_name, '.');
            // Add only .txt files, excluding our own internal log files
            if (ext && strcmp(ext, ".txt") == 0) {
                if (strcmp(dir->d_name, "history.txt") != 0 && 
                    strcmp(dir->d_name, "index.txt") != 0) {
                    strncpy(files[numFiles], dir->d_name, 99);
                    numFiles++;
                }
            }
        }
        closedir(d);
    }
}

// Save search history
void saveHistory(char *term) {
    FILE *fp = fopen("history.txt", "a");
    if (fp) {
        fprintf(fp, "%s\n", term);
        fclose(fp);
    }
}

// Show search history
void showHistory() {
    FILE *fp = fopen("history.txt", "r");
    if (!fp) {
        printf("\nNo history available.\n");
        return;
    }

    char line[100];
    printf("\n----- SEARCH HISTORY -----\n");
    while (fgets(line, sizeof(line), fp))
        printf("- %s", line);

    fclose(fp);
}

// Detect phrase search
int isPhrase(char *input) {
    int len = strlen(input);
    if (len > 2 && input[0] == '"' && input[len - 1] == '"')
        return 1;
    return 0;
}

// Remove quotes from phrase
void removeQuotes(char *str) {
    int len = strlen(str);
    memmove(str, str + 1, len - 2);
    str[len - 2] = '\0';
}

// Highlight all occurrences of a word in a line
void highlightWord(char *line, char *word) {
    char lowerLine[MAX_LINE];
    char lowerWord[MAX_WORD_LEN];

    snprintf(lowerLine, sizeof(lowerLine), "%s", line);
    toLowerCase(lowerLine);
    snprintf(lowerWord, sizeof(lowerWord), "%s", word);
    toLowerCase(lowerWord);

    char *pos = lowerLine;
    const char *orig = line;
    int len = strlen(lowerWord);

    while ((pos = strstr(pos, lowerWord)) != NULL) {
        int offset = pos - lowerLine;
        printf("%.*s", (int)(line + offset - orig), orig);
        printf(RED "%.*s" RESET, len, line + offset);
        orig = line + offset + len;
        pos += len;
    }
    printf("%s", orig);
}

// Fast substring search
int fastSearch(char *text, char *pattern) {
    if (strstr(text, pattern)) return 1;
    return 0;
}

// Sort results by occurrences (Descending)
void sortResults(SearchResult results[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (results[j].occurrences > results[i].occurrences) {
                SearchResult temp = results[i];
                results[i] = results[j];
                results[j] = temp;
            }
        }
    }
}

// Search inside a single file
int searchInFile(char *filename, char *query) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return 0;

    char line[MAX_LINE];
    char lowerLine[MAX_LINE];
    char lowerQuery[MAX_WORD_LEN];
    strncpy(lowerQuery, query, MAX_WORD_LEN - 1);
    toLowerCase(lowerQuery);

    int count = 0;
    int lineNum = 1;

    while (fgets(line, sizeof(line), fp)) {
        strncpy(lowerLine, line, MAX_LINE - 1);
        toLowerCase(lowerLine);

        if (fastSearch(lowerLine, lowerQuery)) {
            printf("[%s] Line %d: ", filename, lineNum);
            highlightWord(line, query);
            printf("\n");
            count++;
        }
        lineNum++;
    }

    fclose(fp);
    return count;
}

// Main search engine function
void searchEngine() {
    char query[100];
    char queryCopy[100];
    SearchResult results[MAX_FILES];

    printf("\nEnter search query: ");
    while (getchar() != '\n'); // Clear buffer
    if (!fgets(query, sizeof(query), stdin)) return;
    query[strcspn(query, "\n")] = 0; 

    if (strlen(query) == 0) {
        printf("Empty query.\n");
        return;
    }

    saveHistory(query);
    strcpy(queryCopy, query);

    if (isPhrase(queryCopy))
        removeQuotes(queryCopy);

    printf("\nSearching for: \"%s\"\n", queryCopy);
    printf("----------------------------------\n");

    for (int i = 0; i < numFiles; i++) {
        int found = searchInFile(files[i], queryCopy);
        strncpy(results[i].fileName, files[i], 99);
        results[i].occurrences = found;
    }

    sortResults(results, numFiles);

    printf("\n----- RANKED RESULTS -----\n");
    int foundAny = 0;
    for (int i = 0; i < numFiles; i++) {
        if (results[i].occurrences > 0) {
            printf("%-15s : %d matches\n", results[i].fileName, results[i].occurrences);
            foundAny = 1;
        }
    }
    if (!foundAny) printf("No matches found in any file.\n");
}

// Build simple index
void buildIndex() {
    FILE *index = fopen("index.txt", "w");
    if (!index) return;

    char word[MAX_WORD_LEN];
    for (int i = 0; i < numFiles; i++) {
        FILE *fp = fopen(files[i], "r");
        if (!fp) continue;
        while (fscanf(fp, "%49s", word) != EOF)
            fprintf(index, "%s %s\n", word, files[i]);
        fclose(fp);
    }
    fclose(index);
}

int main() {
    int choice;
    
    // Dynamically load files and prepare index
    discoverFiles();
    buildIndex();

    while (1) {
        printf("\n=================================\n");
        printf("        C TEXT SEARCH ENGINE\n");
        printf("      (Monitoring %d files)\n", numFiles);
        printf("=================================\n");
        printf("1. Search\n");
        printf("2. View History\n");
        printf("3. Refresh File List\n");
        printf("4. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input.\n");
            continue;
        }

        switch (choice) {
            case 1: searchEngine(); break;
            case 2: showHistory(); break;
            case 3: discoverFiles(); buildIndex(); printf("Files refreshed!\n"); break;
            case 4: printf("Exiting...\n"); exit(0);
            default: printf("Invalid choice\n");
        }
    }
    return 0;
}