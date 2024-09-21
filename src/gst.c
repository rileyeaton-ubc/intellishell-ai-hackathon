#include <stdio.h>
#include <stdlib.h>

int main() {
    // Open the file called "env" in read mode
    FILE *file = fopen("env", "r");
    if (file == NULL) {
        // If the file could not be opened, print an error message and exit
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Buffer to hold each line of the file
    char line[256];

    // Read and print the contents of the file line by line
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);  // Print the line to stdout
    }

    // Close the file after reading
    fclose(file);

    return EXIT_SUCCESS;
}
