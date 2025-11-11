#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStruct;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
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

static char* url_encode(const char *str) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    
    char *encoded = curl_easy_escape(curl, str, 0);
    char *result = strdup(encoded);
    curl_free(encoded);
    curl_easy_cleanup(curl);
    
    return result;
}

SpotifyTrackList* spotify_search_tracks(SpotifyToken *token, const char *query, int limit) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    
    char *encoded_query = url_encode(query);
    if (!encoded_query) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/search?q=%s&type=track&limit=%d",
             encoded_query, limit);
    free(encoded_query);
    
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
    
    // Parse JSON response
    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);
    
    if (!root) return NULL;
    
    struct json_object *tracks_obj, *items;
    if (!json_object_object_get_ex(root, "tracks", &tracks_obj) ||
        !json_object_object_get_ex(tracks_obj, "items", &items)) {
        json_object_put(root);
        return NULL;
    }
    
    int count = json_object_array_length(items);
    SpotifyTrackList *list = malloc(sizeof(SpotifyTrackList));
    list->tracks = malloc(sizeof(SpotifyTrack) * count);
    list->count = count;
    
    struct json_object *total_obj;
    if (json_object_object_get_ex(tracks_obj, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }
    
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *obj;
        
        // ID
        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(list->tracks[i].id, json_object_get_string(obj), sizeof(list->tracks[i].id) - 1);
        }
        
        // Name
        if (json_object_object_get_ex(item, "name", &obj)) {
            strncpy(list->tracks[i].name, json_object_get_string(obj), sizeof(list->tracks[i].name) - 1);
        }
        
        // Artist
        struct json_object *artists;
        if (json_object_object_get_ex(item, "artists", &artists) && 
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (json_object_object_get_ex(artist, "name", &obj)) {
                strncpy(list->tracks[i].artist, json_object_get_string(obj), sizeof(list->tracks[i].artist) - 1);
            }
        }
        
        // Album
        struct json_object *album;
        if (json_object_object_get_ex(item, "album", &album) &&
            json_object_object_get_ex(album, "name", &obj)) {
            strncpy(list->tracks[i].album, json_object_get_string(obj), sizeof(list->tracks[i].album) - 1);
        }
        
        // Duration
        if (json_object_object_get_ex(item, "duration_ms", &obj)) {
            list->tracks[i].duration_ms = json_object_get_int(obj);
        }
        
        // URI
        if (json_object_object_get_ex(item, "uri", &obj)) {
            strncpy(list->tracks[i].uri, json_object_get_string(obj), sizeof(list->tracks[i].uri) - 1);
        }
    }
    
    json_object_put(root);
    return list;
}

bool spotify_save_tracks(SpotifyToken *token, const char **track_ids, int count) {
    CURL *curl = curl_easy_init();
    if (!curl) return false;
    
    // Build JSON body
    struct json_object *root = json_object_new_object();
    struct json_object *ids_array = json_object_new_array();
    
    for (int i = 0; i < count; i++) {
        json_object_array_add(ids_array, json_object_new_string(track_ids[i]));
    }
    
    json_object_object_add(root, "ids", ids_array);
    const char *json_str = json_object_to_json_string(root);
    
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.spotify.com/v1/me/tracks");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);
    
    CURLcode res = curl_easy_perform(curl);
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    json_object_put(root);
    
    return (res == CURLE_OK && response_code == 200);
}

SpotifyTrackList* spotify_get_saved_tracks(SpotifyToken *token, int limit, int offset) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    
    char url[256];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/me/tracks?limit=%d&offset=%d",
             limit, offset);
    
    MemoryStruct response = {0};
    
    char auth_header[512];
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
    
    // Parse JSON
    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);
    
    if (!root) return NULL;
    
    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        json_object_put(root);
        return NULL;
    }
    
    int count = json_object_array_length(items);
    SpotifyTrackList *list = malloc(sizeof(SpotifyTrackList));
    list->tracks = malloc(sizeof(SpotifyTrack) * count);
    list->count = count;
    
    struct json_object *total_obj;
    if (json_object_object_get_ex(root, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }
    
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *track, *obj;
        
        if (!json_object_object_get_ex(item, "track", &track)) continue;
        
        // ID
        if (json_object_object_get_ex(track, "id", &obj)) {
            strncpy(list->tracks[i].id, json_object_get_string(obj), sizeof(list->tracks[i].id) - 1);
        }
        
        // Name
        if (json_object_object_get_ex(track, "name", &obj)) {
            strncpy(list->tracks[i].name, json_object_get_string(obj), sizeof(list->tracks[i].name) - 1);
        }
        
        // Artist
        struct json_object *artists;
        if (json_object_object_get_ex(track, "artists", &artists) && 
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (json_object_object_get_ex(artist, "name", &obj)) {
                strncpy(list->tracks[i].artist, json_object_get_string(obj), sizeof(list->tracks[i].artist) - 1);
            }
        }
        
        // Album
        struct json_object *album;
        if (json_object_object_get_ex(track, "album", &album) &&
            json_object_object_get_ex(album, "name", &obj)) {
            strncpy(list->tracks[i].album, json_object_get_string(obj), sizeof(list->tracks[i].album) - 1);
        }
        
        // Duration
        if (json_object_object_get_ex(track, "duration_ms", &obj)) {
            list->tracks[i].duration_ms = json_object_get_int(obj);
        }
        
        // URI
        if (json_object_object_get_ex(track, "uri", &obj)) {
            strncpy(list->tracks[i].uri, json_object_get_string(obj), sizeof(list->tracks[i].uri) - 1);
        }
    }
    
    json_object_put(root);
    return list;
}

void spotify_free_track_list(SpotifyTrackList *list) {
    if (!list) return;
    free(list->tracks);
    free(list);
}

void spotify_print_track(SpotifyTrack *track, int index) {
    printf("%d. %s\n", index, track->name);
    printf("   Artist: %s\n", track->artist);
    printf("   Album: %s\n", track->album);
    printf("   Duration: %d:%02d\n", 
           track->duration_ms / 60000, 
           (track->duration_ms / 1000) % 60);
    printf("   ID: %s\n", track->id);
}

SpotifyArtistList* spotify_search_artists(SpotifyToken *token, const char *query, int limit) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    
    char *encoded_query = url_encode(query);
    if (!encoded_query) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/search?q=%s&type=artist&limit=%d",
             encoded_query, limit);
    free(encoded_query);
    
    MemoryStruct response = {0};
    
    char auth_header[512];
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
    
    // Parse JSON response
    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);
    
    if (!root) return NULL;
    
    struct json_object *artists_obj, *items;
    if (!json_object_object_get_ex(root, "artists", &artists_obj) ||
        !json_object_object_get_ex(artists_obj, "items", &items)) {
        json_object_put(root);
        return NULL;
    }
    
    int count = json_object_array_length(items);
    SpotifyArtistList *list = malloc(sizeof(SpotifyArtistList));
    list->artists = malloc(sizeof(SpotifyArtist) * count);
    list->count = count;
    
    struct json_object *total_obj;
    if (json_object_object_get_ex(artists_obj, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }
    
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *obj;
        
        // Initialize with empty strings
        memset(&list->artists[i], 0, sizeof(SpotifyArtist));
        
        // ID
        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(list->artists[i].id, json_object_get_string(obj), 
                    sizeof(list->artists[i].id) - 1);
        }
        
        // Name
        if (json_object_object_get_ex(item, "name", &obj)) {
            strncpy(list->artists[i].name, json_object_get_string(obj), 
                    sizeof(list->artists[i].name) - 1);
        }
        
        // Genres
        struct json_object *genres;
        if (json_object_object_get_ex(item, "genres", &genres)) {
            int genre_count = json_object_array_length(genres);
            char genres_str[512] = {0};
            for (int j = 0; j < genre_count && j < 5; j++) {
                struct json_object *genre = json_object_array_get_idx(genres, j);
                if (j > 0) strcat(genres_str, ", ");
                strncat(genres_str, json_object_get_string(genre), 
                        sizeof(genres_str) - strlen(genres_str) - 1);
            }

            if (genres_str[0] != '\0') {
                strncpy(list->artists[i].genres, genres_str, sizeof(list->artists[i].genres) - 1);
                list->artists[i].genres[sizeof(list->artists[i].genres) - 1] = '\0';
            }
        }
        
        // Followers
        struct json_object *followers_obj;
        if (json_object_object_get_ex(item, "followers", &followers_obj) &&
            json_object_object_get_ex(followers_obj, "total", &obj)) {
            list->artists[i].followers = json_object_get_int(obj);
        }
        
        // Popularity
        if (json_object_object_get_ex(item, "popularity", &obj)) {
            list->artists[i].popularity = json_object_get_int(obj);
        }
        
        // URI
        if (json_object_object_get_ex(item, "uri", &obj)) {
            strncpy(list->artists[i].uri, json_object_get_string(obj), 
                    sizeof(list->artists[i].uri) - 1);
        }
        
        // Image URL (first image if available)
        struct json_object *images;
        if (json_object_object_get_ex(item, "images", &images) && 
            json_object_array_length(images) > 0) {
            struct json_object *image = json_object_array_get_idx(images, 0);
            if (json_object_object_get_ex(image, "url", &obj)) {
                strncpy(list->artists[i].image_url, json_object_get_string(obj), 
                        sizeof(list->artists[i].image_url) - 1);
            }
        }
    }
    
    json_object_put(root);
    return list;
}

void spotify_free_artist_list(SpotifyArtistList *list) {
    if (!list) return;
    free(list->artists);
    free(list);
}

SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;
    
    // Default to US market if not specified
    if (!market) market = "US";
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/artists/%s/top-tracks?market=%s",
             artist_id, market);
    
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
    
    // Parse JSON response
    struct json_object *root = json_tokener_parse(response.data);
    free(response.data);
    
    if (!root) return NULL;
    
    struct json_object *tracks;
    if (!json_object_object_get_ex(root, "tracks", &tracks)) {
        json_object_put(root);
        return NULL;
    }
    
    int count = json_object_array_length(tracks);
    SpotifyTrackList *list = malloc(sizeof(SpotifyTrackList));
    list->tracks = malloc(sizeof(SpotifyTrack) * count);
    list->count = count;
    list->total = count;
    
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(tracks, i);
        struct json_object *obj;
        
        // Initialize with empty strings
        memset(&list->tracks[i], 0, sizeof(SpotifyTrack));
        
        // ID
        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(list->tracks[i].id, json_object_get_string(obj), 
                    sizeof(list->tracks[i].id) - 1);
        }
        
        // Name
        if (json_object_object_get_ex(item, "name", &obj)) {
            strncpy(list->tracks[i].name, json_object_get_string(obj), 
                    sizeof(list->tracks[i].name) - 1);
        }
        
        // Artist
        struct json_object *artists;
        if (json_object_object_get_ex(item, "artists", &artists) && 
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (json_object_object_get_ex(artist, "name", &obj)) {
                strncpy(list->tracks[i].artist, json_object_get_string(obj), 
                        sizeof(list->tracks[i].artist) - 1);
            }
        }
        
        // Album
        struct json_object *album;
        if (json_object_object_get_ex(item, "album", &album) &&
            json_object_object_get_ex(album, "name", &obj)) {
            strncpy(list->tracks[i].album, json_object_get_string(obj), 
                    sizeof(list->tracks[i].album) - 1);
        }
        
        // Duration
        if (json_object_object_get_ex(item, "duration_ms", &obj)) {
            list->tracks[i].duration_ms = json_object_get_int(obj);
        }
        
        // URI
        if (json_object_object_get_ex(item, "uri", &obj)) {
            strncpy(list->tracks[i].uri, json_object_get_string(obj), 
                    sizeof(list->tracks[i].uri) - 1);
        }
    }
    
    json_object_put(root);
    return list;
}

void spotify_print_artist(SpotifyArtist *artist, int index) {
    printf("%d. %s\n", index, artist->name);
    printf("   Followers: %d\n", artist->followers);
    printf("   Popularity: %d/100\n", artist->popularity);
    if (strlen(artist->genres) > 0) {
        printf("   Genres: %s\n", artist->genres);
    }
    printf("   ID: %s\n", artist->id);
}


