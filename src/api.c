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

// ===== HELPER FUNCTIONS =====

/**
 * Performs a GET request to Spotify API
 * Returns parsed JSON object or NULL on error
 */
static struct json_object* spotify_api_get(SpotifyToken *token, const char *url) {
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
static bool spotify_api_put(SpotifyToken *token, const char *url, const char *json_data) {
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
 * Parse track data from JSON object into SpotifyTrack struct
 */
static void parse_track_json(struct json_object *item, SpotifyTrack *track) {
    struct json_object *obj;
    
    memset(track, 0, sizeof(SpotifyTrack));
    
    // ID
    if (json_object_object_get_ex(item, "id", &obj)) {
        strncpy(track->id, json_object_get_string(obj), sizeof(track->id) - 1);
    }
    
    // Name
    if (json_object_object_get_ex(item, "name", &obj)) {
        strncpy(track->name, json_object_get_string(obj), sizeof(track->name) - 1);
    }
    
    // Artist
    struct json_object *artists;
    if (json_object_object_get_ex(item, "artists", &artists) && 
        json_object_array_length(artists) > 0) {
        struct json_object *artist = json_object_array_get_idx(artists, 0);
        if (json_object_object_get_ex(artist, "name", &obj)) {
            strncpy(track->artist, json_object_get_string(obj), sizeof(track->artist) - 1);
        }
    }
    
    // Album - CORRECTION ICI: "albums" -> "album"
    struct json_object *album;
    if (json_object_object_get_ex(item, "album", &album) &&
        json_object_object_get_ex(album, "name", &obj)) {
        strncpy(track->album, json_object_get_string(obj), sizeof(track->album) - 1);
    }
    
    // Duration
    if (json_object_object_get_ex(item, "duration_ms", &obj)) {
        track->duration_ms = json_object_get_int(obj);
    }
    
    // URI
    if (json_object_object_get_ex(item, "uri", &obj)) {
        strncpy(track->uri, json_object_get_string(obj), sizeof(track->uri) - 1);
    }
}

/**
 * Parse artist data from JSON object into SpotifyArtist struct
 */
static void parse_artist_json(struct json_object *item, SpotifyArtist *artist) {
    struct json_object *obj;
    
    memset(artist, 0, sizeof(SpotifyArtist));
    
    // ID
    if (json_object_object_get_ex(item, "id", &obj)) {
        strncpy(artist->id, json_object_get_string(obj), sizeof(artist->id) - 1);
    }
    
    // Name
    if (json_object_object_get_ex(item, "name", &obj)) {
        strncpy(artist->name, json_object_get_string(obj), sizeof(artist->name) - 1);
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
            strncpy(artist->genres, genres_str, sizeof(artist->genres) - 1);
            artist->genres[sizeof(artist->genres) - 1] = '\0';
        }
    }
    
    // Followers
    struct json_object *followers_obj;
    if (json_object_object_get_ex(item, "followers", &followers_obj) &&
        json_object_object_get_ex(followers_obj, "total", &obj)) {
        artist->followers = json_object_get_int(obj);
    }
    
    // Popularity
    if (json_object_object_get_ex(item, "popularity", &obj)) {
        artist->popularity = json_object_get_int(obj);
    }
    
    // URI
    if (json_object_object_get_ex(item, "uri", &obj)) {
        strncpy(artist->uri, json_object_get_string(obj), sizeof(artist->uri) - 1);
    }
    
    // Image URL
    struct json_object *images;
    if (json_object_object_get_ex(item, "images", &images) && 
        json_object_array_length(images) > 0) {
        struct json_object *image = json_object_array_get_idx(images, 0);
        if (json_object_object_get_ex(image, "url", &obj)) {
            strncpy(artist->image_url, json_object_get_string(obj), sizeof(artist->image_url) - 1);
        }
    }
}

// ===== PUBLIC API FUNCTIONS =====

SpotifyTrackList* spotify_search_tracks(SpotifyToken *token, const char *query, int limit) {
    char *encoded_query = url_encode(query);
    if (!encoded_query) return NULL;
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/search?q=%s&type=track&limit=%d",
             encoded_query, limit);
    free(encoded_query);
    
    struct json_object *root = spotify_api_get(token, url);
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
        parse_track_json(item, &list->tracks[i]);
    }
    
    json_object_put(root);
    return list;
}

SpotifyArtistList* spotify_search_artists(SpotifyToken *token, const char *query, int limit) {
    char *encoded_query = url_encode(query);
    if (!encoded_query) return NULL;
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/search?q=%s&type=artist&limit=%d",
             encoded_query, limit);
    free(encoded_query);
    
    struct json_object *root = spotify_api_get(token, url);
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
        parse_artist_json(item, &list->artists[i]);
    }
    
    json_object_put(root);
    return list;
}

SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market) {
    if (!market) market = "US";
    
    char url[512];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/artists/%s/top-tracks?market=%s",
             artist_id, market);
    
    struct json_object *root = spotify_api_get(token, url);
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
        parse_track_json(item, &list->tracks[i]);
    }
    
    json_object_put(root);
    return list;
}

SpotifyAlbumList* spotify_get_artist_albums(SpotifyToken *token, const char *artist_id) {
    char url[256];
    snprintf(url, sizeof(url),
            "https://api.spotify.com/v1/artists/%s/albums?limit=50&include_groups=album",
            artist_id);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get API response\n");
        return NULL;
    }

    // Check if there's an error in the response
    struct json_object *error_obj;
    if (json_object_object_get_ex(root, "error", &error_obj)) {
        fprintf(stderr, "API Error: %s\n", json_object_to_json_string(error_obj));
        json_object_put(root);
        return NULL;
    }

    // Get items array directly from root
    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        fprintf(stderr, "Failed to get 'items' from response\n");
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    if (count == 0) {
        fprintf(stderr, "No albums found (items array is empty)\n");
        json_object_put(root);
        return NULL;
    }

    SpotifyAlbumList *list = malloc(sizeof(SpotifyAlbumList));
    if (!list) {
        fprintf(stderr, "Failed to allocate memory for album list\n");
        json_object_put(root);
        return NULL;
    }

    list->albums = malloc(sizeof(SpotifyAlbum) * count);
    if (!list->albums) {
        fprintf(stderr, "Failed to allocate memory for albums array\n");
        free(list);
        json_object_put(root);
        return NULL;
    }

    list->count = count;
    
    struct json_object *total_obj;
    if (json_object_object_get_ex(root, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        if (!item) {
            fprintf(stderr, "Failed to get album at index %d\n", i);
            continue;
        }

        struct json_object *obj;
        
        memset(&list->albums[i], 0, sizeof(SpotifyAlbum));
        
        // ID
        if (json_object_object_get_ex(item, "id", &obj)) {
            const char *id_str = json_object_get_string(obj);
            if (id_str) {
                strncpy(list->albums[i].id, id_str, sizeof(list->albums[i].id) - 1);
                list->albums[i].id[sizeof(list->albums[i].id) - 1] = '\0';
            }
        }
        
        // Name
        if (json_object_object_get_ex(item, "name", &obj)) {
            const char *name_str = json_object_get_string(obj);
            if (name_str) {
                strncpy(list->albums[i].name, name_str, sizeof(list->albums[i].name) - 1);
                list->albums[i].name[sizeof(list->albums[i].name) - 1] = '\0';
            }
        }
        
        // Artist (from artists array)
        struct json_object *artists;
        if (json_object_object_get_ex(item, "artists", &artists) && 
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (artist && json_object_object_get_ex(artist, "name", &obj)) {
                const char *artist_str = json_object_get_string(obj);
                if (artist_str) {
                    strncpy(list->albums[i].artist, artist_str, sizeof(list->albums[i].artist) - 1);
                    list->albums[i].artist[sizeof(list->albums[i].artist) - 1] = '\0';
                }
            }
        }
    }

    json_object_put(root);
    return list;
}

SpotifyTrackList* spotify_get_saved_tracks(SpotifyToken *token, int limit, int offset) {
    char url[256];
    snprintf(url, sizeof(url), 
             "https://api.spotify.com/v1/me/tracks?limit=%d&offset=%d",
             limit, offset);
    
    struct json_object *root = spotify_api_get(token, url);
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
        struct json_object *track;
        
        if (json_object_object_get_ex(item, "track", &track)) {
            parse_track_json(track, &list->tracks[i]);
        }
    }
    
    json_object_put(root);
    return list;
}

bool spotify_save_tracks(SpotifyToken *token, const char **track_ids, int count) {
    // Build JSON body
    struct json_object *root = json_object_new_object();
    struct json_object *ids_array = json_object_new_array();
    
    for (int i = 0; i < count; i++) {
        json_object_array_add(ids_array, json_object_new_string(track_ids[i]));
    }
    
    json_object_object_add(root, "ids", ids_array);
    const char *json_str = json_object_to_json_string(root);
    
    bool result = spotify_api_put(token, "https://api.spotify.com/v1/me/tracks", json_str);
    
    json_object_put(root);
    return result;
}

void spotify_free_track_list(SpotifyTrackList *list) {
    if (!list) return;
    free(list->tracks);
    free(list);
}

void spotify_free_artist_list(SpotifyArtistList *list) {
    if (!list) return;
    free(list->artists);
    free(list);
}

void spotify_free_album_list(SpotifyAlbumList *list) {
    if (!list) return;
    free(list->albums);
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

void spotify_print_artist(SpotifyArtist *artist, int index) {
    printf("%d. %s\n", index, artist->name);
    printf("   Followers: %d\n", artist->followers);
    printf("   Popularity: %d/100\n", artist->popularity);
    if (strlen(artist->genres) > 0) {
        printf("   Genres: %s\n", artist->genres);
    }
    printf("   ID: %s\n", artist->id);
}

void spotify_print_album(SpotifyAlbum *album, int index) {
    printf("%d. %s\n", index, album->name);
    printf("   Artist: %s\n", album->artist);
    printf("   ID: %s\n", album->id);
}
