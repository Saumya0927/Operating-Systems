#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

void generate_collatz_sequence(int n) {
    printf("%d: ", n);
    while (n != 1) {
        printf("%d, ", n);
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
    }
    printf("1\n");
}

int main() {
    FILE *file = fopen("start_numbers.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        int n = atoi(line);
        printf("Parent Process: The positive integer read from file is %d\n", n);

        // Create shared memory object
        int shm_fd = shm_open("/collatz_sequence", O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, sizeof(int));

        // Map shared memory object
        int *shared_data = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        // Write the number to shared memory
        *shared_data = n;

        // Fork a child process
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process
            printf("Child Process: The generated collatz sequence is\n");
            generate_collatz_sequence(*shared_data);

            // Remove shared memory object
            shm_unlink("/collatz_sequence");
            fclose(file);
            exit(0);
        } else {
            // Parent process
            
            // Wait for the child to finish
            wait(NULL);

            // Cleanup shared memory
            munmap(shared_data, sizeof(int));
            close(shm_fd);
        }
    }

    fclose(file);
    return 0;
}
