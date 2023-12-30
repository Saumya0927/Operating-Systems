#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 1048576

// block  memory
typedef struct {
    int start_address;
    int end_address;
    char process_name[10];
    int size;
    int is_free;
} MemoryBlock;

MemoryBlock memory[MAX_SIZE];
int numBlocks = 0;

// initialize memory
void initializeMemory() {
    memory[0].start_address = 0;
    memory[0].end_address = MAX_SIZE - 1;
    memory[0].size = MAX_SIZE;
    memory[0].is_free = 1;
    strcpy(memory[0].process_name, "Free");
    numBlocks = 1;
}

// best fit function
int findBestFit(int size) {
    int bestIndex = -1, bestSize = MAX_SIZE + 1;
    for (int i = 0; i < numBlocks; i++) {
        if (memory[i].is_free && memory[i].size >= size && memory[i].size < bestSize) {
            bestSize = memory[i].size;
            bestIndex = i;
        }
    }
    return bestIndex;
}

// mem allocation
void allocateBestFit(char process_name[], int size) {
    int index = findBestFit(size);
    if (index == -1) {
        printf("No suitable hole found\n");
        return;
    }

    if (memory[index].size > size) {
        for (int i = numBlocks; i > index; i--) {
            memory[i] = memory[i - 1];
        }
        numBlocks++;

        memory[index + 1].start_address = memory[index].start_address + size;
        memory[index + 1].end_address = memory[index].end_address;
        memory[index + 1].size = memory[index].size - size;
        memory[index + 1].is_free = 1;
        strcpy(memory[index + 1].process_name, "Free");

        memory[index].end_address = memory[index].start_address + size - 1;
        memory[index].size = size;
    }

    memory[index].is_free = 0;
    strcpy(memory[index].process_name, process_name);
    printf("Allocated %d to process %s\n", size, process_name);
}

// release memory
void releaseMemory(char process_name[]) {
    int found = 0;
    for (int i = 0; i < numBlocks; i++) {
        if (!memory[i].is_free && strcmp(memory[i].process_name, process_name) == 0) {
            memory[i].is_free = 1;
            strcpy(memory[i].process_name, "Free");
            found = 1;
            printf("releasing memory for process %s\n", process_name);
            break;
        }
    }
    if (!found) {
        printf("Process not found\n");
    }
}

// memory status
void displayMemoryStatus() {
    printf("Memory Allocation Status:\n");
    for (int i = 0; i < numBlocks; i++) {
        if (!memory[i].is_free) {
            printf("Block: [%d:%d] Process: %s\n", memory[i].start_address, memory[i].end_address, memory[i].process_name);
        }
    }
    printf("Free Memory Blocks:\n");
    for (int i = 0; i < numBlocks; i++) {
        if (memory[i].is_free) {
            printf("Block: [%d:%d] Size: %d\n", memory[i].start_address, memory[i].end_address, memory[i].size);
        }
    }
}

//  memory compaction
void compactMemory() {
    int freeSpace = 0, i, j;
    for (i = 0; i < numBlocks; i++) {
        if (memory[i].is_free) {
            freeSpace += memory[i].size;
            for (j = i; j < numBlocks - 1; j++) {
                memory[j] = memory[j + 1];
            }
            numBlocks--;
            i--;
        }
    }

    if (freeSpace > 0) {
        memory[numBlocks].start_address = memory[numBlocks - 1].end_address + 1;
        memory[numBlocks].end_address = memory[numBlocks].start_address + freeSpace - 1;
        memory[numBlocks].size = freeSpace;
        memory[numBlocks].is_free = 1;
        strcpy(memory[numBlocks].process_name, "Free");
        numBlocks++;
    }
    printf("Memory compaction completed\n");
}

int main() {
    initializeMemory();

    char command[10], process_name[10], strategy;
    int size;

    printf("Here the Best Fit approach has been implemented and the allocated 1048576 bytes of memory.\n");
    while (1) {
        printf("allocator> ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            scanf("%s %d %c", process_name, &size, &strategy);
            if (strategy == 'B') {
                allocateBestFit(process_name, size);
            } else {
                printf("Unsupported allocation strategy\n");
            }
        } else if (strcmp(command, "RL") == 0) {
            scanf("%s", process_name);
            releaseMemory(process_name);
        } else if (strcmp(command, "Status") == 0) {
            displayMemoryStatus();
        } else if (strcmp(command, "C") == 0) {
            compactMemory();
        } else if (strcmp(command, "Exit") == 0) {
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    return 0;
}
