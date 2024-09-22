#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>


// ANSI escape codes for colors
const char *green_text = "\e[0;32m"; // Green colour text
const char *reset_text_new = "\e[0m";  // Reset text colour to default

// Function to get the API key, and set it as an environment variable
int set_api_credential() {    
    // Open the file called "env" in read mode
    FILE *env = fopen("env", "r");
    if (env == NULL) {
        // If the file could not be opened, print an error message and exit
        fprintf(stderr, "Error opening environment variable file\n");
        return -1;
    }

    // Buffer to hold each line of the file
    char line[256];
    // Read and print the contents of the file line by line (should just be one line)
    while (fgets(line, sizeof(line), env)) {
        // Remove the leading newline from the string
        line[strcspn(line, "\r\n")] = 0;

        // Set the OPENAI_API_KEY environment variable
        if (setenv("OPENAI_API_KEY", line, 1) != 0) {
            fprintf(stderr, "Failed to set environment variable OPENAI_API_KEY\n");
            fclose(env);
            return -1;
        }
    }

    // Close the file after reading
    fclose(env);

    // Return success
    return 0;
}

// Structure to hold the response data
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function to write the response to a memory buffer
static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Calculate the real size of the incoming data
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp; // Cast the pointer to custom MemoryStruct

    // Reallocate memory to fit  new data
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        // Memory allocation failed
        fprintf(stderr, "Not enough memory for response\n");
        return 0;  // Returning 0 will cause libcurl to abort the operation
    }

    // Update the memory pointer and copy the new data into the buffer
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    // Update the memory pointer and copy the new data into the buffer
    mem->size += realsize;
    // Null-terminate the string
    mem->memory[mem->size] = 0;

    return realsize; // Return the number of bytes handled
}

// Function to extract and print the 'choices[0].message.content' from the JSON response
void print_choice_message_content(const char *json_response) {
    // Parse the JSON response
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON\n"); // Print an error if this fails
        return;
    }

    // Extract 'choices[0].message.content' from the json body
    cJSON *choices = cJSON_GetObjectItem(json, "choices");
    if (choices != NULL && cJSON_IsArray(choices)) {
        cJSON *first_choice = cJSON_GetArrayItem(choices, 0); // First item in choices array
        if (first_choice != NULL) {
            cJSON *message = cJSON_GetObjectItem(first_choice, "message"); // Message key
            if (message != NULL) {
                cJSON *content = cJSON_GetObjectItem(message, "content"); // Content key
                if (content != NULL && cJSON_IsString(content)) {
                    printf("%sGenerated Response =>%s %s\n", green_text, reset_text_new, content->valuestring); // Actual content of message
                }
            }
        }
    }

    // Clean up
    cJSON_Delete(json);
}

// Function to query the OpenAI API using a given prompt and return a generation 
int get_generation(char* prompt, CURL *curl, CURLcode res) {
    // First, check to see if the API environment variable exists
    const char *api_key = getenv("OPENAI_API_KEY");
    if (api_key == NULL) {
        fprintf(stderr, "Environment variable OPENAI_API_KEY is not set\n");
        return -1;
    }

    // Initialize the MemoryStruct
    struct MemoryStruct response;
    response.memory = NULL;  // Important: Initialize to NULL
    response.size = 0;       // Initialize size to 0

    // Set URL for the request, for this case it will always be the chat completions
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");

    // Set correct headers, including the API key as bearer
    struct curl_slist *headers = NULL;
    char *auth_start = "Authorization: Bearer ";
    char *auth_string = malloc(strlen(auth_start) + strlen(api_key) + 1);
    if (auth_string == NULL) {
        fprintf(stderr, "Failed to allocate memory for auth_string\n");
        return -1;
    }
    sprintf(auth_string, "%s%s", auth_start, api_key); // Add the token to the auth bearer string

    // Set required headers, including auth and setup
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_string);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    free(auth_string); // Clean up

    // Build the JSON data with the provided prompt using cJSON
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        fprintf(stderr, "Failed to create JSON root object\n"); // If failed, print
        curl_slist_free_all(headers);
        return -1;
    }

    // Add main parameters to pass to API such as model, temperature, and seed
    cJSON_AddStringToObject(root, "model", "gpt-4o-mini");
    cJSON_AddNumberToObject(root, "temperature", 0.9);
    cJSON_AddNumberToObject(root, "seed", 15);

    // Create the messages array and print on failure
    cJSON *messages = cJSON_AddArrayToObject(root, "messages");
    if (!messages) {
        fprintf(stderr, "Failed to create messages array\n");
        cJSON_Delete(root);
        curl_slist_free_all(headers);
        return -1;
    }

    // Create the system message JSON object and print on failure
    cJSON *system_message = cJSON_CreateObject();
    if (!system_message) {
        fprintf(stderr, "Failed to create system message object\n");
        cJSON_Delete(root);
        curl_slist_free_all(headers);
        return -1;
    }
    // Add the system role to the object, and the content that grounds it for each generation
    cJSON_AddStringToObject(system_message, "role", "system");
    cJSON_AddStringToObject(system_message, "content",
        "You are a helpful and knowledgeable UNIX shell assistant named IntelliShell. You will assist users who are using the UNIX shell by providing insight into command usage, error messages, recommended commands, and general tips as needed. Only respond in plain text, using no more than 3 lines.");
    cJSON_AddItemToArray(messages, system_message); // Add above system message object to array

    // Create the user message JSON object and print on failure
    cJSON *user_message = cJSON_CreateObject();
    if (!user_message) {
        fprintf(stderr, "Failed to create user message object\n");
        cJSON_Delete(root);
        curl_slist_free_all(headers);
        return -1;
    }
    // Add the user roll to the object, and their prompt that was passed as a parameter
    cJSON_AddStringToObject(user_message, "role", "user");
    cJSON_AddStringToObject(user_message, "content", prompt);
    cJSON_AddItemToArray(messages, user_message); // Add above user message object to array

    // Convert the JSON object to a string, and print on failure
    char *data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!data) {
        fprintf(stderr, "Failed to convert JSON to string\n");
        curl_slist_free_all(headers);
        return -1;
    }

    // Set the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    // Set the callback function and pass the response struct
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);

    // Perform the request
    res = curl_easy_perform(curl);

    // Free the JSON data string
    free(data);

    // Check for errors
    if (res != CURLE_OK) {
        // If the response failed, print the failure and free up all allocations
        fprintf(stderr, "Generation failed: %s\n", curl_easy_strerror(res));
        printf("Please try again\n");
        curl_slist_free_all(headers);
        if (response.memory) {
            free(response.memory);
        }
        return -1;
    } else {
        // Process the response
        print_choice_message_content(response.memory);
    }

    // Clean up
    curl_slist_free_all(headers);
    if (response.memory) {
        free(response.memory);
    }

    return 0;
}