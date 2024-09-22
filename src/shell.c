// Must compile using gcc src/shell.c -o bin/shell_test -lreadline

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100
#define MAX_PATH 4096
#define MAX_USERID 500

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
        execvp(args[0], args); // Try to execute the command
        // If execvp returns, an error occurred
        perror(args[0]); // Print the command name with the error
        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Error forking
        perror("fork");
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete
    }
}

int main() {
    // Store the command ran, arguments, the current working directory, and the user's username
    char command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    char cwd[MAX_PATH];
    char username[MAX_USERID];
    cuserid(username);

    while (1) {
        // Store working directory and print error on failure
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        // Print shell prompt
        char prompt[MAX_COMMAND_LENGTH];
        snprintf(prompt, sizeof(prompt), "%s IntelliShell ~%s$ ", username, cwd);

        // Read command input
        char* input = readline(prompt); // prompt is the string you use in printf

        if (input == NULL) {
            break; // Exit on EOF (Ctrl+D)
        }

        // If the input is not empty, add it to the history
        if (*input != '\0') {
            add_history(input);
        }

        // Copy input to command buffer
        strncpy(command, input, MAX_COMMAND_LENGTH);
        command[MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null-termination

        free(input); // readline() allocates memory that needs to be freed

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

        // Handle the cd command in the parent process (changing directories in child process doesnt work)
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "cd: expected argument to \"cd\"\n");
            } else {
                if (chdir(args[1]) != 0) {
                    perror("cd");
                }
            }
            continue; // Skip the rest and start the next iteration
        }

        // Execute the command
        execute_command(args);
    }

    return 0;
}
