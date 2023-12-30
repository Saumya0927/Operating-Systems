#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define SUBGRID_SIZE 3

// Structure to pass data to threads
typedef struct {
    int row;
    int column;
} parameters;

int sudoku[SIZE][SIZE]; // The Sudoku puzzle grid
int result[SIZE];      // Array to store results from threads


// Function to check if a row contains all digits from 1 to 9
void* validateRow(void* param) {
    parameters* data = (parameters*)param;
    int row = data->row;
    int used[SIZE] = {0};

    for (int col = 0; col < SIZE; col++) {
        int num = sudoku[row][col];
        if (used[num - 1] == 1) {
            result[row] = 0;
            pthread_exit(NULL);
        }
        used[num - 1] = 1;
    }

    result[row] = 1;
    pthread_exit(NULL);
}


// Function to check if a column contains all digits from 1 to 9.
void* validateColumn(void* param) {
    parameters* data = (parameters*)param;
    int col = data->column;
    int used[SIZE] = {0};

    for (int row = 0; row < SIZE; row++) {
        int num = sudoku[row][col];
        if (used[num - 1] == 1) {
            result[col] = 0;
            pthread_exit(NULL);
        }
        used[num - 1] = 1;
    }

    result[col] = 1;
    pthread_exit(NULL);
}



// Function to check if a 3x3 subgrid contains all digits from 1 to 9
void* validateSubgrid(void* param) {
    parameters* data = (parameters*)param;
    int startRow = data->row;
    int startCol = data->column;
    int used[SIZE] = {0};

    for (int i = startRow; i < startRow + SUBGRID_SIZE; i++) {
        for (int j = startCol; j < startCol + SUBGRID_SIZE; j++) {
            int num = sudoku[i][j];
            if (used[num - 1] == 1) {
                pthread_exit(NULL);
            }
            used[num - 1] = 1;
        }
    }

    pthread_exit(NULL);
}


// Main function
int main() {
    // Read sudoku puzzle from a file (sample_in_sudoku.txt)
    FILE* file = fopen("sample_in_sudoku.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (fscanf(file, "%d", &sudoku[i][j]) != 1) {
                perror("Error reading Sudoku data from file");
                return 1;
            }
        }
    }

    // This is to print out the puzzle. It isn't working properly
    // Print the Sudoku puzzle
    printf("Sudoku Puzzle Solution is: \n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }

    fclose(file);

    pthread_t threads[SIZE + SIZE + SIZE]; 
    int threadCount = 0;

    // Create threads to validate rows
    for (int i = 0; i < SIZE; i++) {
        parameters* data = (parameters*)malloc(sizeof(parameters));
        data->row = i;
        pthread_create(&threads[threadCount++], NULL, validateRow, data);
    }

    // Create threads to validate columns
    for (int i = 0; i < SIZE; i++) {
        parameters* data = (parameters*)malloc(sizeof(parameters));
        data->column = i;
        pthread_create(&threads[threadCount++], NULL, validateColumn, data);
    }

    // Create threads to validate subgrids
    for (int i = 0; i < SIZE; i += SUBGRID_SIZE) {
        for (int j = 0; j < SIZE; j += SUBGRID_SIZE) {
            parameters* data = (parameters*)malloc(sizeof(parameters));
            data->row = i;
            data->column = j;
            pthread_create(&threads[threadCount++], NULL, validateSubgrid, data);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    // Check the results
    int isValid = 1;
    for (int i = 0; i < SIZE; i++) {
        if (result[i] == 0) {
            isValid = 0;
            break;
        }
    }

    if (isValid) {
        printf("Sudoku puzzle is valid.\n");
    } else {
        printf("Sudoku puzzle is not valid.\n");
    }

    return 0;
}
