#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORDS 100
#define MAX_FILES 10
#define MAX_WORD_LEN 50

// Structure to store the Index
typedef struct {
    char word[MAX_WORD_LEN];
    char filenames[MAX_FILES][MAX_WORD_LEN];
    int file_count;
} IndexEntry;

IndexEntry indexTable[MAX_WORDS];
int totalUniqueWords = 0;

// Helper function to create sample files as requested
void createSampleFiles() {
    FILE *f1 = fopen("file1.txt", "w");
    fprintf(f1, "hello world apple");
    fclose(f1);

    FILE *f2 = fopen("file2.txt", "w");
    fprintf(f2, "hello banana cherry");
    fclose(f2);

    FILE *f3 = fopen("file3.txt", "w");
    fprintf(f3, "apple cherry date");
    fclose(f3);
}

int main() {
    createSampleFiles(); // Set up the environment

    int numFiles;
    char filenames[MAX_FILES][MAX_WORD_LEN];
    char line[256];

    printf("Enter Number of Files (e.g., 3): ");
    scanf("%d", &numFiles);

    for (int i = 0; i < numFiles; i++) {
        printf("Enter filename %d: ", i + 1);
        scanf("%s", filenames[i]);
    }

    // --- Indexing Phase ---
    for (int i = 0; i < numFiles; i++) {
        FILE *fp = fopen(filenames[i], "r");
        
        // Flowchart: Is file open? (Logic check: 'if' handles the 'Yes/No')
        if (fp == NULL) {
            printf("File not found: %s\n", filenames[i]);
            continue;
        }

        // Flowchart: Read a line: fgets()
        while (fgets(line, sizeof(line), fp)) {
            // Flowchart: Split line into words: strtok()
            char *token = strtok(line, " \n\r\t");
            while (token != NULL) {
                
                // Flowchart: Word in Index?
                int foundIdx = -1;
                for (int j = 0; j < totalUniqueWords; j++) {
                    if (strcmp(indexTable[j].word, token) == 0) {
                        foundIdx = j;
                        break;
                    }
                }

                if (foundIdx == -1) {
                    // Flowchart: Add word to Index
                    strcpy(indexTable[totalUniqueWords].word, token);
                    strcpy(indexTable[totalUniqueWords].filenames[0], filenames[i]);
                    indexTable[totalUniqueWords].file_count = 1;
                    totalUniqueWords++;
                } else {
                    // Flowchart: Filename already stored for word?
                    int fileExists = 0;
                    for (int k = 0; k < indexTable[foundIdx].file_count; k++) {
                        if (strcmp(indexTable[foundIdx].filenames[k], filenames[i]) == 0) {
                            fileExists = 1;
                            break;
                        }
                    }
                    if (!fileExists) {
                        // Flowchart: Add filename to the word's list
                        strcpy(indexTable[foundIdx].filenames[indexTable[foundIdx].file_count], filenames[i]);
                        indexTable[foundIdx].file_count++;
                    }
                }
                token = strtok(NULL, " \n\r\t");
            }
        }
        fclose(fp);
    }

    // --- Search Phase ---
    char searchWord[MAX_WORD_LEN];
    printf("\nInput Search Word: ");
    scanf("%s", searchWord);

    int found = 0;
    for (int i = 0; i < totalUniqueWords; i++) {
        if (strcmp(indexTable[i].word, searchWord) == 0) {
            // Flowchart: Display filenames where word is found
            printf("Word found in: ");
            for (int k = 0; k < indexTable[i].file_count; k++) {
                printf("%s ", indexTable[i].filenames[k]);
            }
            printf("\n");
            found = 1;
            break;
        }
    }

    if (!found) {
        // Flowchart: Show "Word Not Found"
        printf("Word Not Found\n");
    }

    return 0;
}
