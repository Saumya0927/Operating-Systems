#include <stdio.h>
#include <stdlib.h>
#define COLUMN_WIDTH 20
#define MAX_THREADS 100  // Assuming a maximum of 100 threads for simplicity

// Define a structure to store thread information
struct ThreadInfo {
    int p_id;
    int arr_time;
    int burst_time;
    int completionTime;
    int turn_around_time;
    int waiting_time;
};

int main() {
    struct ThreadInfo threads[MAX_THREADS];
    int threadCount = 0; 

    // Open the input file
    FILE *file = fopen("sample_in_schedule.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Read thread information from the file dynamically
    while (fscanf(file, "%d,%d,%d", &threads[threadCount].p_id, &threads[threadCount].arr_time, &threads[threadCount].burst_time) == 3) {
        threadCount++;
    }

    fclose(file);

    // Calculate completion time, turn-around time, and waiting time
    int currentTime = 0;
    for (int i = 0; i < threadCount; i++) {
        // Update completion time
        if (currentTime < threads[i].arr_time) {
            currentTime = threads[i].arr_time;
        }
        threads[i].completionTime = currentTime + threads[i].burst_time;

        // Calculate turn-around time and waiting time
        threads[i].turn_around_time = threads[i].completionTime - threads[i].arr_time;
        threads[i].waiting_time = threads[i].turn_around_time - threads[i].burst_time;

        // Update current time
        currentTime = threads[i].completionTime;
    }

    // Calculate the average waiting time and average turn-around time
    double averageWaitTime = 0.0;
    double averageTurnAroundTime = 0.0;

    for (int i = 0; i < threadCount; i++) {
        averageWaitTime += threads[i].waiting_time;
        averageTurnAroundTime += threads[i].turn_around_time;
    }

    averageWaitTime /= threadCount;
    averageTurnAroundTime /= threadCount;

    // Print the results
    printf("%-*s%-*s%-*s%-*s%-*s%-*s\n", 
       COLUMN_WIDTH, "Thread ID", 
       COLUMN_WIDTH, "Arrival Time", 
       COLUMN_WIDTH, "Burst Time", 
       COLUMN_WIDTH, "Completion Time", 
       COLUMN_WIDTH, "Turn-Around Time", 
       COLUMN_WIDTH, "Waiting Time");

    for (int i = 0; i < threadCount; i++) {
        printf("%-*d%-*d%-*d%-*d%-*d%-*d\n", 
                COLUMN_WIDTH, threads[i].p_id,
                COLUMN_WIDTH, threads[i].arr_time,
                COLUMN_WIDTH, threads[i].burst_time,
                COLUMN_WIDTH, threads[i].completionTime,
                COLUMN_WIDTH,  threads[i].turn_around_time,
                COLUMN_WIDTH,  threads[i].waiting_time);
    }

    printf("\nAverage Waiting Time: %.2lf\n", averageWaitTime);
    printf("Average Turn-Around Time: %.2lf\n", averageTurnAroundTime);

    return 0;
}
