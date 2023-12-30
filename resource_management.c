#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_RESOURCES 5
int available_resources = MAX_RESOURCES;
#define NUM_THREADS 5

pthread_mutex_t mutex;


int decrease_count(int thread_number, int count) {
    pthread_mutex_lock(&mutex);
    if (available_resources < count) {
        pthread_mutex_unlock(&mutex);
        printf("Thread %d could not acquire enough resources.\n", thread_number);
        return -1;
    } else {
        available_resources -= count;
        printf("The thread %d has acquired %d resources and %d more resources are available.\n", thread_number, count, available_resources);
        pthread_mutex_unlock(&mutex);
        return 0;
    }
}


int increase_count(int thread_number, int count) {
    pthread_mutex_lock(&mutex);
    available_resources += count;
    printf("The thread %d has released %d resources and %d resources are now available.\n", thread_number, count, available_resources);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void* thread_function(void* arg) {
    int thread_number = *(int*)arg;

    if (decrease_count(thread_number, 1) == 0) {
        sleep(1);
        increase_count(thread_number, 1);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_numbers[NUM_THREADS];

    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_numbers[i] = i;
        pthread_create(&threads[i], NULL, (void*)&thread_function, &thread_numbers[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    printf("All threads have finished execution. Available resources: %d\n", available_resources);

    return 0;
}
