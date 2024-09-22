#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

// Function to get the API key, and set it as an environment variable
int set_api_credential() {    
    // Open the file called "env" in read mode
    FILE *env = fopen("env", "r");
    if (env == NULL) {
        // If the file could not be opened, print an error message and exit
        perror("Error opening environment variable file\n");
        fclose(env);
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
            perror("Failed to set environment variable OPENAI_API_KEY\n");
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

// Function to extract and print the 'choices[0].message.content' from the JSON response
void print_choice_message_content(const char *json_response) {
    // Parse the JSON response
    cJSON *json = cJSON_Parse(json_response);
    if (json == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    // Extract 'choices[0].message.content' from the json body
    cJSON *choices = cJSON_GetObjectItem(json, "choices");
    if (choices != NULL && cJSON_IsArray(choices)) {
        cJSON *first_choice = cJSON_GetArrayItem(choices, 0);
        if (first_choice != NULL) {
            cJSON *message = cJSON_GetObjectItem(first_choice, "message");
            if (message != NULL) {
                cJSON *content = cJSON_GetObjectItem(message, "content");
                if (content != NULL && cJSON_IsString(content)) {
                    printf("Message Content: %s\n", content->valuestring);
                }
            }
        }
    }

    // Clean up
    cJSON_Delete(json);
}

// Callback function to write the response to a memory buffer
static size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        // Memory allocation failed
        fprintf(stderr, "Not enough memory for response\n");
        return 0;
    }

    // Add the new data to the memory buffer
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;  // Null-terminate the string

    return realsize;
}

// Function to query the OpenAI API using a given prompt and return a generation 
int get_generation(char* prompt, CURL *curl, CURLcode res) {
    // First, check to see if the API environment variable exists
    const char *api_key = getenv("OPENAI_API_KEY");
    if (api_key == NULL) {
        perror("Environment variable OPENAI_API_KEY is already set\n");
        return -1;
    }

    // Set URL for the request, for this case it will always be the chat completions
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");

    // Set correct headers, including the API key as bearer
    struct curl_slist *headers = NULL;
    char *auth_start = "Authorization: Bearer ";
    char *auth_string = malloc(strlen(auth_start) + strlen(api_key) + 1);
    sprintf(auth_string, "%s%s", auth_start, api_key);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_string);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    free(auth_string);

    // Data in the post request
    const char *data =
        "{"
        "  \"model\": \"gpt-4o-mini\","
        "  \"messages\": ["
        "    {"
        "      \"role\": \"system\","
        "      \"content\": \"You are a helpful and knowledgeable UNIX shell assistant named UNIX-GST (Generative Shell Tool). You will assist users who are using the UNIX shell by providing insight into command usage, error messages, recommended commands, and general tips as needed. Respond in plain text (no formatting), and ensure you are succinct yet insightful.\""
        "    },"
        "    {"
        "      \"role\": \"user\","
        "      \"content\": \"What is the command for changing directories?\""
        "    }"
        "  ]"
        "}";

    // Load the above data to POST
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

    // Set the callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);

    // Create the request    
    res = curl_easy_perform(curl);

    // If there is an error getting the response, print this and return a fail code
    if (res != CURLE_OK) {
        fprintf(stderr, "Generation failed for reason %s\n", curl_easy_strerror(res));
        printf("Please try again\n");
        return -1;
    }

    return 0;
}

int main() {
    // initialize curl library with variable
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    // Call the function to get the api key in the env file and set it as an environment variable
    if (set_api_credential() != 0) {
        perror("Cannot proceed, as the API key could not be set\n");
        return -1;
    }

    // Call api function to test
    get_generation("What is the command for changing directories?", curl, res);
    
    // Clean up curl
    curl_easy_cleanup(curl);

    return 0;
}
