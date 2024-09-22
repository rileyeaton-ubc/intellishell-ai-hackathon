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
#define MAX_PATH 500
#define MAX_USERID 500

// ANSI escape codes for colors
const char *purple_text = "\e[0;35m"; // Purple colour text
const char *reset_text = "\e[0m";  // Reset text colour to default
const char *cyan_bold_text = "\e[1;36m"; // Cyan colour bold text
const char *red_bold_text = "\e[1;31m"; // Bolt red text

// Function to parse the command and its arguments
void parse_command(char* command, char** args) {
    int i = 0;
    args[i] = strtok(command, " \t\n"); // Tokenize the command
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " \t\n"); // Continue tokenizing for arguments
    }
}

// Function to concatenate all arguments into a single string
char* concat_args(char** args) {
    // Step 1: Calculate the total length needed for the concatenated string
    int total_length = 0;
    for (int i = 0; args[i] != NULL; i++) {
        total_length += strlen(args[i]); // Length of each argument
        total_length++; // For the space between arguments
    }

    // Step 2: Allocate memory for the concatenated string (including the null terminator)
    char* result = malloc(total_length + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Step 3: Concatenate all arguments into the allocated string
    result[0] = '\0'; // Start with an empty string
    for (int i = 0; args[i] != NULL; i++) {
        strcat(result, args[i]); // Append argument
        if (args[i + 1] != NULL) {
            strcat(result, " "); // Add space between arguments
        }
    }

    return result; // Return the concatenated string
}

// Function to execute the command
void execute_command(char** args) {
    pid_t pid = fork(); // Create a child process
    if (pid == 0) { // Child process
        execvp(args[0], args); // Try to execute the command
        // If execvp returns, an error occurred
        printf("The following error occurred, generating AI response...\n");
        
        // Concatenate all arguments to a single string
        size_t len = strlen(red_bold_text) + strlen(args[0]) + 1;
        char* concatenated_error = (char*)malloc(len);
        snprintf(concatenated_error, len, "%s%s", red_bold_text, args[0]);
        perror(concatenated_error); // Print the command name with the error
        
        // Concatenate all arguments to a single string
        char* concatenated_args = concat_args(args);

        // Call API function to get generation based on failure
        if (concatenated_args != NULL) {
            // Call the API function to get generation based on the concatenated string
            if (get_generation(concatenated_args, curl, res) != 0) {
                fprintf(stderr, "Failed to get generation\n");
                curl_easy_cleanup(curl);
            }
            free(concatenated_args); // Free the allocated memory for concatenated_args
        }

        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Error forking
        perror("fork");
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to complete
    }
}

int main() { 
    // Store the command ran, arguments, the current working directory
    char command[MAX_COMMAND_LENGTH];
    char* args[MAX_ARGS];
    char cwd[MAX_PATH];

    while (1) {
        // Store working directory and print error on failure
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        // Print shell prompt
        char prompt[MAX_COMMAND_LENGTH];
        snprintf(prompt, sizeof(prompt), "%sIntelliShell%s ~%s%s$ ", cyan_bold_text, purple_text, cwd, reset_text);

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
