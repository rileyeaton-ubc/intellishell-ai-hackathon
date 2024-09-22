#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100

// Function to parse the command and its arguments
void parse_command(char* command, char** args) {
    int i = 0;
    args[i] = strtok(command, " \t\n"); // Tokenize the command
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n"); // Continue tokenizing for arguments
    }
}

// Function to execute the command
void execute_command(char** args) {
    pid_t pid = fork(); // Create a child process
    if (pid == 0) { // Child process
        if (execvp(args[0], args) == -1) { // Execute the command
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Error forking
        perror("shell");
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];

    while (1) {
        // Print shell prompt
        printf("myshell> ");
        fflush(stdout);

        // Read command input
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            break; // Exit on EOF
        }

        // Remove newline character from the command
        command[strcspn(command, "\n")] = '\0';

        // Parse the command into args
        parse_command(command, args);

        // Check for exit command
        if (args[0] == NULL) {
            continue; // Ignore empty input
        }
        if (strcmp(args[0], "exit") == 0) {
            break; // Exit the shell
        }

        // Execute the command
        execute_command(args);
    }

    return 0;
}
