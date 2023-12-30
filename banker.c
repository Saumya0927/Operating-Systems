#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <time.h>   

#define MAX_CUSTOMERS 50
#define MAX_RESOURCES 50

int available[MAX_RESOURCES];
int maximum[MAX_CUSTOMERS][MAX_RESOURCES];
int allocation[MAX_CUSTOMERS][MAX_RESOURCES];
int need[MAX_CUSTOMERS][MAX_RESOURCES];
int numberOfCustomers;
int numberOfResources;
pthread_mutex_t lock;

void initializeSystem(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(1);
    }

    numberOfCustomers = 0;
    char line[256]; 

    while (fgets(line, sizeof(line), file) && numberOfCustomers < MAX_CUSTOMERS) {
        char *token = strtok(line, ",");
        for (int j = 0; j < numberOfResources && token != NULL; j++) {
            maximum[numberOfCustomers][j] = atoi(token);
            allocation[numberOfCustomers][j] = 0;
            need[numberOfCustomers][j] = maximum[numberOfCustomers][j];
            token = strtok(NULL, ","); 
        }
        numberOfCustomers++;
    }

    fclose(file);
}

bool isSafe() {
    int work[MAX_RESOURCES];
    bool finish[MAX_CUSTOMERS] = {false};

    for (int i = 0; i < numberOfResources; i++) {
        work[i] = available[i];
    }

    while (true) {
        bool found = false;
        for (int i = 0; i < numberOfCustomers; i++) {
            if (!finish[i]) {
                int j;
                for (j = 0; j < numberOfResources; j++) {
                    if (need[i][j] > work[j])
                        break;
                }
                if (j == numberOfResources) {
                    for (int k = 0; k < numberOfResources; k++)
                        work[k] += allocation[i][k];
                    finish[i] = true;
                    found = true;
                }
            }
        }
        if (!found)
            break;
    }

    for (int i = 0; i < numberOfCustomers; i++) {
        if (!finish[i])
            return false;
    }
    return true;
}

bool requestResources(int customerNum, int request[]) {
    pthread_mutex_lock(&lock);

    printf("Processing request from customer %d\n", customerNum);

    // checks if request is greater than need or available resources
    for (int i = 0; i < numberOfResources; i++) {
        if (request[i] > need[customerNum][i]) {
            printf("Error: Requesting more resources than needed.\n");
            pthread_mutex_unlock(&lock);
            return false;
        }
        if (request[i] > available[i]) {
            printf("Resources not available currently.\n");
            pthread_mutex_unlock(&lock);
            return false;
        }
    }

    for (int i = 0; i < numberOfResources; i++) {
        available[i] -= request[i];
        allocation[customerNum][i] += request[i];
        need[customerNum][i] -= request[i];
    }

    if (isSafe()) {
        pthread_mutex_unlock(&lock);
        return true;
    } else {
        for (int i = 0; i < numberOfResources; i++) {
            available[i] += request[i];
            allocation[customerNum][i] -= request[i];
            need[customerNum][i] += request[i];
        }
        pthread_mutex_unlock(&lock);
        return false;
    }
}

void releaseResources(int customerNum, int release[]) {
    pthread_mutex_lock(&lock);

    for (int i = 0; i < numberOfResources; i++) {
        allocation[customerNum][i] -= release[i];
        available[i] += release[i];
        need[customerNum][i] += release[i];
    }

    pthread_mutex_unlock(&lock);
}

void *customerThread(void *arg) {
    int customerNum = *(int *)arg;
    srand(time(NULL) + customerNum); 

    printf("Customer/Thread %d is starting.\n", customerNum);

    int request[MAX_RESOURCES];
    for (int i = 0; i < numberOfResources; i++) {
        request[i] = rand() % (need[customerNum][i] + 1); 
    }

    // allocate resources
    if (requestResources(customerNum, request)) {
        printf("Customer/Thread %d: Allocated resources.\n", customerNum);
        
        // simulate some processing (work) by the thread
        sleep(rand() % 3 + 1);  // sleep for a random time between 1 to 3 seconds

        printf("Customer/Thread %d: Work done, releasing resources.\n", customerNum);

        // release resources
        releaseResources(customerNum, request);
        printf("Customer/Thread %d: Released resources.\n", customerNum);
    } else {
        printf("Customer/Thread %d: Request denied. Cannot allocate resources.\n", customerNum);
    }

    pthread_exit(NULL);
}


void printSystemState() {
    printf("Number of Customers: %d\n", numberOfCustomers);
    printf("Currently Available resources: ");
    for (int i = 0; i < numberOfResources; i++) {
        printf("%d ", available[i]);
    }
    printf("\nMaximum resources from file:\n");
    for (int i = 0; i < numberOfCustomers; i++) {
        for (int j = 0; j < numberOfResources; j++) {
            printf("%d ", maximum[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number of resources> <resource1> <resource2> ...\n", argv[0]);
        return 1;
    }

    numberOfResources = argc - 1;
    for (int i = 0; i < numberOfResources; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    pthread_mutex_init(&lock, NULL);
    initializeSystem("sample_in_banker.txt");
    printSystemState();

    pthread_t threads[MAX_CUSTOMERS];
    int customerIds[MAX_CUSTOMERS];

    while (true) {
        char command[10];
        int customerNum, request[MAX_RESOURCES];

        printf("Enter Command: ");
        scanf("%s", command);

        if (strcmp(command, "RQ") == 0) {
            scanf("%d", &customerNum);
            for (int i = 0; i < numberOfResources; i++) {
                scanf("%d", &request[i]);
            }
            if (requestResources(customerNum, request)) {
                printf("State is safe, and request is satisfied\n");
            } else {
                printf("Request cannot be granted, leads to unsafe state\n");
            }
        } else if (strcmp(command, "RL") == 0) {
            scanf("%d", &customerNum);
            for (int i = 0; i < numberOfResources; i++) {
                scanf("%d", &request[i]);
            }
            releaseResources(customerNum, request);
            printf("The resources have been released successfully\n");
        } else if (strcmp(command, "Status") == 0) {
            printSystemState();
        } else if (strcmp(command, "Run") == 0) {
            for (int i = 0; i < numberOfCustomers; i++) {
                customerIds[i] = i;
                pthread_create(&threads[i], NULL, customerThread, (void *)&customerIds[i]);
            }
            for (int i = 0; i < numberOfCustomers; i++) {
                pthread_join(threads[i], NULL);
            }
        } else if (strcmp(command, "Exit") == 0) {
            break;
        } else {
            printf("Invalid input, use one of RQ, RL, Status, Run, Exit.\n");
        }
    }

    pthread_mutex_destroy(&lock);
    return 0;
}
