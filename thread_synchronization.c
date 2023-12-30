#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <time.h>

sem_t running;
sem_t even;
sem_t odd;
time_t programClock; // Global timer/clock for the program

typedef struct thread {
    char tid[4];
    unsigned int startTime;
    int state;
    pthread_t handle;
    int retVal;
} Thread;

// Function prototypes
void logStart(char *tID);
void logFinish(char *tID);
void startClock();
long getCurrentTime();
int readFile(char *fileName, Thread **threads);
int threadsLeft(Thread *threads, int threadCount);
int threadToStart(Thread *threads, int threadCount);
void* threadRun(void *t);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Input file name missing...exiting with error code -1\n");
        return -1;
    }

    Thread *threads = NULL;
    int threadCount = readFile(argv[1], &threads);

    sem_init(&running, 0, 1);
    sem_init(&even, 0, 1);
    sem_init(&odd, 0, 0);

    startClock();

    while (threadsLeft(threads, threadCount) > 0) {
        int i = 0;
        while ((i = threadToStart(threads, threadCount)) > -1) {
            threads[i].state = 1;
            pthread_create(&(threads[i].handle), NULL, threadRun, &threads[i]);
        }
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i].handle, NULL);
    }

    sem_destroy(&running);
    sem_destroy(&even);
    sem_destroy(&odd);
    free(threads);

    return 0;
}

void logStart(char *tID) {
    printf("[%ld] New Thread with ID %s is started.\n", getCurrentTime(), tID);
}

void logFinish(char *tID) {
    printf("[%ld] Thread with ID %s is finished.\n", getCurrentTime(), tID);
}

void startClock() {
    programClock = time(NULL);
}

long getCurrentTime() {
    time_t now;
    now = time(NULL);
    return now - programClock;
}

int readFile(char *fileName, Thread **threads) {
    FILE *in = fopen(fileName, "r");
    if (!in) {
        printf("Error in opening input file...exiting with error code -1\n");
        return -1;
    }

    struct stat st;
    fstat(fileno(in), &st);
    char *fileContent = (char*) malloc(((int) st.st_size + 1) * sizeof(char));
    fileContent[0] = '\0';
    while (!feof(in)) {
        char line[100];
        if (fgets(line, 100, in) != NULL) {
            strncat(fileContent, line, strlen(line));
        }
    }
    fclose(in);

    char *command = NULL;
    int threadCount = 0;
    char *fileCopy = (char*) malloc((strlen(fileContent) + 1) * sizeof(char));
    strcpy(fileCopy, fileContent);
    command = strtok(fileCopy, "\r\n");
    while (command != NULL) {
        threadCount++;
        command = strtok(NULL, "\r\n");
    }
    *threads = (Thread*) malloc(sizeof(Thread) * threadCount);

    char *lines[threadCount];
    command = NULL;
    int i = 0;
    command = strtok(fileContent, "\r\n");
    while (command != NULL) {
        lines[i] = malloc(sizeof(command) * sizeof(char));
        strcpy(lines[i], command);
        i++;
        command = strtok(NULL, "\r\n");
    }

    for (int k = 0; k < threadCount; k++) {
        char *token = NULL;
        int j = 0;
        token = strtok(lines[k], ";");
        while (token != NULL) {
            (*threads)[k].state = 0;
            if (j == 0) {
                strcpy((*threads)[k].tid, token);
            }
            if (j == 1) {
                (*threads)[k].startTime = atoi(token);
            }
            j++;
            token = strtok(NULL, ";");
        }
    }
    return threadCount;
}

int threadsLeft(Thread *threads, int threadCount) {
    int remainingThreads = 0;
    for (int k = 0; k < threadCount; k++) {
        if (threads[k].state != -1)
            remainingThreads++;
    }
    return remainingThreads;
}

int threadToStart(Thread *threads, int threadCount) {
    for (int k = 0; k < threadCount; k++) {
        if (threads[k].state == 0 && threads[k].startTime <= getCurrentTime())
            return k;
    }
    return -1;
}

void* threadRun(void *t) {
    Thread *thread = (Thread*) t;
    logStart(thread->tid);

    int lastDigit = thread->tid[2] - '0';
    if (lastDigit % 2 == 0) {
        sem_wait(&even);
    } else {
        sem_wait(&odd);
    }

    printf("[%ld] Thread %s is in its critical section\n", getCurrentTime(), thread->tid);

    if (lastDigit % 2 == 0) {
        sem_post(&odd);
    } else {
        sem_post(&even);
    }

    logFinish(thread->tid);
    thread->state = -1;
    pthread_exit(0);
}
