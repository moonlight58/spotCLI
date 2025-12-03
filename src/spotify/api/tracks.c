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

/**
 * Get detailed information about a single track
 * 
 * @param token - Valid Spotify token
 * @param track_id - Spotify track ID
 * @param market - Optional: ISO 3166-1 alpha-2 country code (NULL for no market filter)
 * @return SpotifyTrack or NULL on error (must be freed with spotify_free_track)
 */
SpotifyTrack* spotify_get_track(SpotifyToken *token, const char *track_id, const char *market) {
    if (!token || !track_id) {
        fprintf(stderr, "Invalid parameters for get_track\n");
        return NULL;
    }

    char url[512];
    if (market) {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/tracks/%s?market=%s",
                 track_id, market);
    } else {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/tracks/%s",
                 track_id);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get track\n");
        return NULL;
    }

    // Check for error response
    struct json_object *error_obj;
    if (json_object_object_get_ex(root, "error", &error_obj)) {
        fprintf(stderr, "API Error: %s\n", json_object_to_json_string(error_obj));
        json_object_put(root);
        return NULL;
    }

    SpotifyTrack *track = malloc(sizeof(SpotifyTrack));
    if (!track) {
        json_object_put(root);
        return NULL;
    }

    parse_track_json(root, track);
    json_object_put(root);

    return track;
}

/**
 * Get detailed information about multiple tracks (max 50)
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs
 * @param count - Number of track IDs (max 50)
 * @param market - Optional: ISO 3166-1 alpha-2 country code (NULL for no market filter)
 * @return SpotifyTrackList or NULL on error
 */
SpotifyTrackList* spotify_get_tracks(SpotifyToken *token, const char **track_ids, int count, const char *market) {
    if (!token || !track_ids || count <= 0 || count > 50) {
        fprintf(stderr, "Invalid parameters (max 50 tracks)\n");
        return NULL;
    }

    // Build URL with query parameters
    char url[2048] = "https://api.spotify.com/v1/tracks?ids=";
    
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(url, ",");
        strcat(url, track_ids[i]);
    }

    // Add market parameter if provided
    if (market) {
        strcat(url, "&market=");
        strcat(url, market);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get tracks\n");
        return NULL;
    }

    struct json_object *tracks_array;
    if (!json_object_object_get_ex(root, "tracks", &tracks_array)) {
        fprintf(stderr, "No 'tracks' field in response\n");
        json_object_put(root);
        return NULL;
    }

    int actual_count = json_object_array_length(tracks_array);
    
    SpotifyTrackList *list = malloc(sizeof(SpotifyTrackList));
    if (!list) {
        json_object_put(root);
        return NULL;
    }

    list->tracks = malloc(sizeof(SpotifyTrack) * actual_count);
    if (!list->tracks) {
        free(list);
        json_object_put(root);
        return NULL;
    }

    list->count = actual_count;
    list->total = actual_count;

    for (int i = 0; i < actual_count; i++) {
        struct json_object *item = json_object_array_get_idx(tracks_array, i);
        
        // Handle null entries (invalid IDs or unavailable tracks)
        if (item && json_object_get_type(item) != json_type_null) {
            parse_track_json(item, &list->tracks[i]);
        } else {
            // Initialize empty track for null entries
            memset(&list->tracks[i], 0, sizeof(SpotifyTrack));
            snprintf(list->tracks[i].name, sizeof(list->tracks[i].name), 
                     "[Unavailable Track]");
        }
    }

    json_object_put(root);
    return list;
}

/**
 * Get user's saved tracks
 * 
 * @param token - Valid Spotify token
 * @param limit - Number of tracks to return (1-50, default 20)
 * @param offset - Index of first track to return (default 0)
 * @return SpotifyTrackList or NULL on error
 */
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

/**
 * Save tracks to current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to save
 * @param count - Number of track IDs (max 50)
 * @return true if successful, false otherwise
 */
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

/**
 * Remove tracks from current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to remove
 * @param count - Number of track IDs (max 50)
 * @return true if successful, false otherwise
 */
bool spotify_remove_tracks(SpotifyToken *token, const char **track_ids, int count) {
    if (!token || !track_ids || count <= 0 || count > 50) {
        fprintf(stderr, "Invalid parameters (max 50 tracks)\n");
        return false;
    }

    const char *url = "https://api.spotify.com/v1/me/tracks";

    // Build JSON body
    struct json_object *root = json_object_new_object();
    struct json_object *ids_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        json_object_array_add(ids_array, json_object_new_string(track_ids[i]));
    }

    json_object_object_add(root, "ids", ids_array);
    const char *json_str = json_object_to_json_string(root);

    // Use DELETE method with JSON body
    struct json_object *response = spotify_api_delete_json(token, url, json_str);
    json_object_put(root);

    if (response) {
        json_object_put(response);
        return true;
    }

    return false;
}

/**
 * Check if one or more tracks are saved in current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to check
 * @param count - Number of track IDs (max 50)
 * @param result_count - Output parameter for number of results returned
 * @return Array of booleans (true = saved, false = not saved), or NULL on error
 *         Caller must free the returned array
 */
bool* spotify_check_saved_tracks(SpotifyToken *token, const char **track_ids, int count, int *result_count) {
    if (!token || !track_ids || count <= 0 || count > 50 || !result_count) {
        fprintf(stderr, "Invalid parameters (max 50 tracks)\n");
        return NULL;
    }

    // Build URL with query parameters
    char url[2048] = "https://api.spotify.com/v1/me/tracks/contains?ids=";
    
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(url, ",");
        strcat(url, track_ids[i]);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to check saved tracks\n");
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
 * Check if a single track is saved - convenience wrapper
 * 
 * @param token - Valid Spotify token
 * @param track_id - Spotify track ID to check
 * @return true if saved, false otherwise
 */
bool spotify_is_track_saved(SpotifyToken *token, const char *track_id) {
    if (!token || !track_id) {
        return false;
    }

    const char *ids[] = { track_id };
    int result_count = 0;
    
    bool *results = spotify_check_saved_tracks(token, ids, 1, &result_count);
    if (!results) {
        return false;
    }

    bool is_saved = results[0];
    free(results);
    
    return is_saved;
}
