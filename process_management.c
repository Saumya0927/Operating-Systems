#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#define MAX_COMMAND_LENGTH 256
#define BUFFER_SIZE 4096
#define TEMP_FILE "temp_output.txt"

void writeOutput(char* command, char* output) {
    FILE* fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "The output of: %s : is\n", command);
    fprintf(fp, ">>>>>>>>>>>>>>>\n%s<<<<<<<<<<<<<<<\n", output);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    char command[MAX_COMMAND_LENGTH];

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        exit(1);
    }

    while (fgets(command, sizeof(command), file)) {
        // Remove newline from command
        command[strcspn(command, "\n")] = '\0';

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child process to execute commands
            int temp_fd = open(TEMP_FILE, O_RDWR | O_CREAT | O_TRUNC, 0644);
            if (temp_fd == -1) {
                perror("open");
                exit(1);
            }

            dup2(temp_fd, STDOUT_FILENO);  // Redirect stdout to the temp file
            dup2(temp_fd, STDERR_FILENO);  // Redirect stderr to the temp file
            close(temp_fd);
            
            execlp("sh", "sh", "-c", command, NULL);  // Execute the command
            perror("execlp");
            exit(1);
        } else {
            // Parent process
            wait(NULL);

            // Read from the temporary file
            FILE *tempFile = fopen(TEMP_FILE, "r");
            if (tempFile) {
                size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, tempFile);
                buffer[bytesRead] = '\0';  // Null terminate the buffer
                writeOutput(command, buffer);
                fclose(tempFile);
            }
        }
    }

    remove(TEMP_FILE); // remove the temporary file
    fclose(file);
    return 0;
}
