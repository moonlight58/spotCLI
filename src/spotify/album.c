#include "spotify/advanced.h"

SpotifyAlbumDetailed* spotify_get_album(SpotifyToken *token, const char *album_id) {
    if (!token || !album_id) {
        fprintf(stderr, "Invalid parameters for get_album\n");
        return NULL;
    }

    char url[512];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/albums/%s", album_id);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get album\n");
        return NULL;
    }

    SpotifyAlbumDetailed *album = malloc(sizeof(SpotifyAlbumDetailed));
    if (!album) {
        json_object_put(root);
        return NULL;
    }

    struct json_object *obj;

    // Album info
    if (json_object_object_get_ex(root, "id", &obj)) {
        strncpy(album->album_info.id, json_object_get_string(obj),
                sizeof(album->album_info.id) - 1);
    }

    if (json_object_object_get_ex(root, "name", &obj)) {
        strncpy(album->album_info.name, json_object_get_string(obj),
                sizeof(album->album_info.name) - 1);
    }

    if (json_object_object_get_ex(root, "release_date", &obj)) {
        strncpy(album->album_info.release_date, json_object_get_string(obj),
                sizeof(album->album_info.release_date) - 1);
    }

    // Artist
    struct json_object *artists;
    if (json_object_object_get_ex(root, "artists", &artists) &&
        json_object_array_length(artists) > 0) {
        struct json_object *artist = json_object_array_get_idx(artists, 0);
        if (json_object_object_get_ex(artist, "name", &obj)) {
            strncpy(album->album_info.artist, json_object_get_string(obj),
                    sizeof(album->album_info.artist) - 1);
        }
    }

    // Tracks
    struct json_object *tracks;
    if (json_object_object_get_ex(root, "tracks", &tracks)) {
        struct json_object *items;
        if (json_object_object_get_ex(tracks, "items", &items)) {
            int count = json_object_array_length(items);
            album->album_info.total_tracks = count;

            album->tracks = malloc(sizeof(SpotifyTrack) * count);
            if (album->tracks) {
                album->count = count;

                for (int i = 0; i < count; i++) {
                    struct json_object *item = json_object_array_get_idx(items, i);
                    parse_track_json(item, &album->tracks[i]);
                }
            }
        }
    }

    json_object_put(root);
    return album;
}

SpotifyAlbumList* spotify_get_user_saved_albums(SpotifyToken *token, int limit, int offset) {
    if (!token) {
        fprintf(stderr, "Invalid token parameter\n");
        return NULL;
    }

    // Validate and set default limit
    if (limit <= 0) limit = 20;
    if (limit > 50) limit = 50;  // Spotify API max for this endpoint
    
    if (offset < 0) offset = 0;

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/me/albums?limit=%d&offset=%d",
             limit, offset);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get saved albums\n");
        return NULL;
    }

    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        fprintf(stderr, "No 'items' field in response\n");
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    
    SpotifyAlbumList *list = malloc(sizeof(SpotifyAlbumList));
    if (!list) {
        fprintf(stderr, "Failed to allocate memory for album list\n");
        json_object_put(root);
        return NULL;
    }

    list->albums = malloc(sizeof(SpotifyAlbum) * count);
    if (!list->albums) {
        fprintf(stderr, "Failed to allocate memory for albums\n");
        free(list);
        json_object_put(root);
        return NULL;
    }
    
    list->count = count;

    // Get total count
    struct json_object *total_obj;
    if (json_object_object_get_ex(root, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }

    // Parse each saved album item
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *album;

        // The response wraps albums in an object with "added_at" and "album" fields
        if (json_object_object_get_ex(item, "album", &album)) {
            parse_album_json(album, &list->albums[i]);
        } else {
            // If parse fails, zero out the album entry
            memset(&list->albums[i], 0, sizeof(SpotifyAlbum));
        }
    }

    json_object_put(root);
    return list;
}

bool spotify_save_albums(SpotifyToken *token, const chat **album_ids, int count) {
    // Build JSON Body
    struct json_object *root = json_object_new_object();
    struct json_object *ids_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        json_object_array_add(ids_array, json_object_new_string(album_ids[i]));
    }

    json_object_object_add(root, "ids", ids_array);
    const char *json_str = json_object_to_json_string(root);

    bool result = spotify_api_put(token, "https://api.spotify.com/v1/me/albums", json_str);

    json_object_put(root);
    return result;
}

bool spotify_remove_albums(SpotifyToken *token, const char **album_ids, int count) {
    if (!token || !album_ids || count <= 0) {
        fprintf(stderr, "Invalid parameters for remove_albums\n");
        return false;
    }

    if (count > 50) {
        fprintf(stderr, "Cannot remove more than 50 albums at once\n");
        return false;
    }

    const char *url = "https://api.spotify.com/v1/me/albums";

    // Build JSON body
    struct json_object *root = json_object_new_object();
    struct json_object *ids_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        json_object_array_add(ids_array, json_object_new_string(album_ids[i]));
    }

    json_object_object_add(root, "ids", ids_array);
    const char *json_str = json_object_to_json_string(root);

    // Spotify's DELETE /v1/me/albums returns 200 OK (not 204)
    // So we need a custom implementation for this endpoint
    CURL *curl = curl_easy_init();
    if (!curl) {
        json_object_put(root);
        return false;
    }

    char auth_header[1024];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token->access_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str);

    CURLcode res = curl_easy_perform(curl);

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    json_object_put(root);

    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
        return false;
    }

    // Spotify returns 200 OK for this endpoint
    if (response_code != 200) {
        fprintf(stderr, "HTTP error: %ld\n", response_code);
        return false;
    }

    return true;
}

bool* spotify_check_saved_albums(SpotifyToken *token, const char **album_ids, int count, int *result_count) {
    if (!token || !album_ids || count <= 0 || !result_count) {
        fprintf(stderr, "Invalid parameters for check_saved_albums\n");
        return NULL;
    }

    if (count > 50) {
        fprintf(stderr, "Cannot check more than 50 albums at once\n");
        return NULL;
    }

    // Build URL with query parameters
    char url[2048] = "https://api.spotify.com/v1/me/albums/contains?ids=";
    
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(url, ",");
        strcat(url, album_ids[i]);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to check saved albums\n");
        return NULL;
    }

    // Response is a JSON array of booleans
    if (json_object_get_type(root) != json_type_array) {
        fprintf(stderr, "Unexpected response format\n");
        json_object_put(root);
        return NULL;
    }

    int response_count = json_object_array_length(root);
    if (response_count != count) {
        fprintf(stderr, "Response count mismatch: expected %d, got %d\n", count, response_count);
        json_object_put(root);
        return NULL;
    }

    // Allocate result array
    bool *results = malloc(sizeof(bool) * count);
    if (!results) {
        fprintf(stderr, "Failed to allocate memory for results\n");
        json_object_put(root);
        return NULL;
    }

    // Parse boolean array
    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(root, i);
        results[i] = json_object_get_boolean(item);
    }

    *result_count = count;
    json_object_put(root);
    return results;
}

/**
 * Check if a single album is saved - convenience wrapper
 */
bool spotify_is_album_saved(SpotifyToken *token, const char *album_id) {
    if (!token || !album_id) {
        return false;
    }

    const char *ids[] = { album_id };
    int result_count = 0;
    
    bool *results = spotify_check_saved_albums(token, ids, 1, &result_count);
    if (!results) {
        return false;
    }

    bool is_saved = results[0];
    free(results);
    
    return is_saved;
}
