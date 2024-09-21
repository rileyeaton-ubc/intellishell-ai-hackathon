#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the API key, and set it as an environment variable
int set_api_credential() {    
    // Open the file called "env" in read mode
    FILE *env = fopen("env", "r");
    if (env == NULL) {
        // If the file could not be opened, print an error message and exit
        perror("Error opening environment variable file");
        fclose(env);
        return -1;
    }

    // Buffer to hold each line of the file
    char line[256];
    // Read and print the contents of the file line by line (should just be one line)
    while (fgets(line, sizeof(line), env)) {
        // Remove the leading newline from the string
        line[strcspn(line, "\r\n")] = 0;

        // Set the GST_API_KEY environment variable
        if (setenv("GST_API_KEY", line, 1) != 0) {
            perror("Failed to set environment variable GST_API_KEY");
            fclose(env);
            return -1;
        }
    }

    // Close the file after reading
    fclose(env);

    // Return success
    return 0;
}

int main() {
    // Call the function to get the api key in the env file and set it as an environment variable
    if (set_api_credential() != 0) {
        perror("cannot proceed, as the API key could not be set");
        return -1;
    }
    
    return 0;
}
