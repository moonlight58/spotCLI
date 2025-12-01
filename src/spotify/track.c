

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
