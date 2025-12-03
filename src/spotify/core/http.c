#include "spotify/spotify_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Out of memory\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

char* url_encode(const char *str) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    char *encoded = curl_easy_escape(curl, str, 0);
    char *result = strdup(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);

    return result;
}

// ===== HELPER FUNCTIONS =====

/**
 * Performs a GET request to Spotify API
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_get(SpotifyToken *token, const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    MemoryStruct response = {0};

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
        free(response.data);
        return NULL;
    }

    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);

    return root;
}

/**
 * Performs a PUT request to Spotify API
 * Returns true if response code is 200
 */
bool spotify_api_put(SpotifyToken *token, const char *url, const char *json_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

    CURLcode res = curl_easy_perform(curl);


    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK && response_code == 200);
}

/**
 * Performs a PUT request to Spotify API (without body)
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_put_empty(SpotifyToken *token, const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Length: 0");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Spotify returns 204 No Content on success
    return (res == CURLE_OK && response_code == 204);
}

/**
 * Performs a PUT request to Spotify API and returns JSON response
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_put_json(SpotifyToken *token, const char *url,
                                         const char *json_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    MemoryStruct response = {0};

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (json_data && strlen(json_data) > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    }

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
        free(response.data);
        return NULL;
    }

    // Check for successful response codes (200, 201)
    if (response_code != 200 && response_code != 201) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        if (response.data) {
            fprintf(stderr, "Response: %s\n", response.data);
        }
        free(response.data);
        return NULL;
    }

    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);

    return root;
}

/**
 * Performs a POST request to Spotify API
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_post(SpotifyToken *token, const char *url, const char *json_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (json_data && strlen(json_data) > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    } else {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    }

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Spotify returns 204 No Content on success
    return (res == CURLE_OK && response_code == 204);
}

/**
 * Performs a POST request to Spotify API (without body)
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_post_empty(SpotifyToken *token, const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Length: 0");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Spotify returns 204 No Content on success
    return (res == CURLE_OK && response_code == 204);
}

/**
 * Performs a POST request to Spotify API and returns JSON response
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_post_json(SpotifyToken *token, const char *url,
                                          const char *json_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    MemoryStruct response = {0};

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (json_data && strlen(json_data) > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    } else {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    }

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
        free(response.data);
        return NULL;
    }

    // Check for successful response codes (200, 201)
    if (response_code != 200 && response_code != 201) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        if (response.data) {
            fprintf(stderr, "Response: %s\n", response.data);
        }
        free(response.data);
        return NULL;
    }

    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);

    return root;
}

/**
 * Performs a DELETE request to Spotify API with JSON body
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_delete_json(SpotifyToken *token, const char *url,
                                            const char *json_data) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    MemoryStruct response = {0};

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (json_data && strlen(json_data) > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
    }

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
        free(response.data);
        return NULL;
    }

    // Check for successful response code (200)
    if (response_code != 200) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        if (response.data) {
            fprintf(stderr, "Response: %s\n", response.data);
        }
        free(response.data);
        return NULL;
    }

    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);

    return root;
}

bool spotify_api_delete_empty(SpotifyToken *token, const char *url) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Length: 0");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    // Spotify returns 200 No Content on success
    return (res == CURLE_OK && response_code == 200);
}

