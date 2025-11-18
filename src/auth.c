#include "auth.h"
#include "dotenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

// Get full path to token file
char* get_token_path() {
    static char path[512];
    const char *home = getenv("HOME");
    if (!home) home = getenv("USERPROFILE"); // Windows fallback
    if (!home) return NULL;
    
    snprintf(path, sizeof(path), "%s/%s/%s", home, TOKEN_DIR, TOKEN_FILENAME);
    return path;
}

// Create token directory if it doesn't exist
static void ensure_token_dir() {
    const char *home = getenv("HOME");
    if (!home) return;
    
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, TOKEN_DIR);
    mkdir(dir_path, 0700);
}

static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {

size_t realsize = size * nmemb;
    strncat((char *)stream, ptr, realsize);
    return realsize;
}

bool spotify_is_authenticated() {
    char *token_path = get_token_path();
    if (!token_path) return false;
    
    FILE *f = fopen(token_path, "r");
    if (!f) return false;
    return true;
}

bool spotify_save_token(SpotifyToken *token) {
    ensure_token_dir();
    char *token_path = get_token_path();
    if (!token_path) return false;
    
    FILE *f = fopen(token_path, "w");
    if (!f) return false;
    
    // Save current time as obtained_at if not set
    if (token->obtained_at == 0) {
        token->obtained_at = time(NULL);
    }
    
    fprintf(f, "{ \"access_token\": \"%s\", \"refresh_token\": \"%s\", \"expires_in\": %ld, \"obtained_at\": %ld }",
            token->access_token, token->refresh_token, token->expires_in, token->obtained_at);
    fclose(f);
    return true;
}

bool spotify_load_token(SpotifyToken *token) {
    char *token_path = get_token_path();
    if (!token_path) return false;
    
    FILE *f = fopen(token_path, "r");
    if (!f) return false;

    struct json_object *parsed = json_object_from_file(token_path);
    if (!parsed) {
        fclose(f);
        return false;
    }

    strcpy(token->access_token, json_object_get_string(json_object_object_get(parsed, "access_token")));
    strcpy(token->refresh_token, json_object_get_string(json_object_object_get(parsed, "refresh_token")));
    token->expires_in = json_object_get_int64(json_object_object_get(parsed, "expires_in"));
    
    // Load obtained_at if it exists, otherwise set to current time
    struct json_object *obtained_at_obj = json_object_object_get(parsed, "obtained_at");
    if (obtained_at_obj) {
        token->obtained_at = json_object_get_int64(obtained_at_obj);
    } else {
        // For backwards compatibility with old token files
        token->obtained_at = time(NULL);
    }

    json_object_put(parsed);
    fclose(f);
    return true;
}

bool spotify_refresh_token(SpotifyToken *token) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;

    // Get credentials from environment
    const char *client_id = getenv("CLIENT_ID");
    const char *client_secret = getenv("CLIENT_SECRET");
    
    if (!client_id || !client_secret) {
        fprintf(stderr, "Error: CLIENT_ID or CLIENT_SECRET not set in environment\n");
        curl_easy_cleanup(curl);
        return false;
    }

    char post_data[1024];
    sprintf(post_data,
            "grant_type=refresh_token&refresh_token=%s&client_id=%s&client_secret=%s",
            token->refresh_token, client_id, client_secret);

    char response[4096] = {0};

    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return false;

    struct json_object *json = json_tokener_parse(response);
    strcpy(token->access_token, json_object_get_string(json_object_object_get(json, "access_token")));
    token->expires_in = json_object_get_int64(json_object_object_get(json, "expires_in"));
    token->obtained_at = time(NULL);  // ADD THIS LINE
    json_object_put(json);

    spotify_save_token(token);
    return true;
}

bool spotify_token_is_expired(SpotifyToken *token) {
    // If obtained_at is not set (0 or invalid), consider token valid
    // This prevents segfault when obtained_at is uninitialized
    if (token->obtained_at <= 0) {
        printf("Token obtained_at not set, assuming valid\n");
        return false;
    }
    
    time_t now = time(NULL);
    time_t elapsed = now - token->obtained_at;
    time_t remaining = token->expires_in - elapsed;
    
    // Refresh if less than 5 minutes remaining
    printf("Checking if token expired: elapsed=%ld, remaining=%ld\n", elapsed, remaining);
    
    bool is_expired = (now - token->obtained_at) >= (token->expires_in - 300);
    printf("Token expired: %s\n", is_expired ? "yes" : "no");
    
    return is_expired;
}

char* start_callback_server(int port, char *code_buffer, size_t buffer_size);

bool spotify_authorize(SpotifyToken *token) {
    
    if (!load_dotenv(".env")) {
        fprintf(stderr, "Erreur : impossible de charger le fichier .env\n");
        return false;
    }

    const char *client_id = getenv("CLIENT_ID");
    const char *client_secret = getenv("CLIENT_SECRET");
    const char *redirect_uri = getenv("REDIRECT_URI");
    const char *user_scopes = "user-library-read \
                               user-library-modify \
                               playlist-modify-public \
                               playlist-modify-private \
                               user-read-playback-state \
                               user-modify-playback-state";

    if (!client_id || !client_secret || !redirect_uri) {
        fprintf(stderr, "Error: Missing environment variables (CLIENT_ID, CLIENT_SECRET, or REDIRECT_URI)\n");
        return false;
    }

    printf("Open this URL in your browser to authorize spotCLI:\n");
    printf("https://accounts.spotify.com/authorize?client_id=%s&response_type=code&redirect_uri=%s"
            "&scope=%s\n\n", client_id, redirect_uri, user_scopes);

    char auth_code[512];
    if (!start_callback_server(8888, auth_code, sizeof(auth_code))) {
        fprintf(stderr, "Failed to start callback server\n");
        return false;
    }

    printf("✓ Authorization code received: %s\n\n", auth_code);

    CURL *curl = curl_easy_init();
    if (!curl) return false;

    char post_data[1024];
    sprintf(post_data,
            "grant_type=authorization_code&code=%s&redirect_uri=%s&client_id=%s&client_secret=%s",
            auth_code, redirect_uri, client_id, client_secret);
    
    char response[4096] = {0};
    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return false;

    struct json_object *json = json_tokener_parse(response);
    if (!json) {
        fprintf(stderr, "❌ Failed to parse JSON.\n");
        return false;
    }

    struct json_object *access = NULL, *refresh = NULL, *expires = NULL;
    json_object_object_get_ex(json, "access_token", &access);
    json_object_object_get_ex(json, "refresh_token", &refresh);
    json_object_object_get_ex(json, "expires_in", &expires);

    if (!access || !refresh || !expires) {
        fprintf(stderr, "❌ Missing token fields in response.\n");
        json_object_put(json);
        return false;
    }

    strcpy(token->access_token, json_object_get_string(access));
    strcpy(token->refresh_token, json_object_get_string(refresh));
    token->expires_in = json_object_get_int64(expires);
    token->obtained_at = time(NULL);  // ADD THIS LINE

    json_object_put(json);

    spotify_save_token(token);
    printf("✅ Authorization successful! Tokens saved.\n");
    return true;
}

bool spotify_get_access_token(SpotifyToken *token) {
    if (!spotify_load_token(token)) {
        printf("No token found, starting authorization...\n");
        return spotify_authorize(token);
    }
    
    // Check if token is expired
    if (spotify_token_is_expired(token)) {
        printf("Token expired, refreshing...\n");
        return spotify_refresh_token(token);
    }
    
    return true;
}
