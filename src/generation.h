// generation.h
#ifndef GENERATION_H
#define GENERATION_H

#include <curl/curl.h>

// Function prototypes to export
int set_api_credential();
int get_generation(char *prompt, CURL *curl, CURLcode res);

#endif