#include "spotify/search.h"

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

SpotifyArtist* spotify_get_artist(SpotifyToken *token, const char *artist_id) {
    if (!token || !artist_id) {
        fprintf(stderr, "Invalid parameters for get_artist\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/artists/%s",
             artist_id);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get artist\n");
        return NULL;
    }

    SpotifyArtist *artist = malloc(sizeof(SpotifyArtist));
    if (!artist) {
        json_object_put(root);
        return NULL;
    }

    parse_artist_json(root, artist);
    json_object_put(root);

    return artist;
}

SpotifyArtistList* spotify_get_artists(SpotifyToken *token, const char **artist_ids, int count) {
    if (!token || !artist_ids || count <= 0 || count > 50) {
        fprintf(stderr, "Invalid parameters (max 50 artists)\n");
        return NULL;
    }

    // Build URL with query parameters
    char url[2048] = "https://api.spotify.com/v1/artists?ids=";
    
    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(url, ",");
        strcat(url, artist_ids[i]);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get artists\n");
        return NULL;
    }

    struct json_object *artists_array;
    if (!json_object_object_get_ex(root, "artists", &artists_array)) {
        fprintf(stderr, "No 'artists' field in response\n");
        json_object_put(root);
        return NULL;
    }

    int actual_count = json_object_array_length(artists_array);
    
    SpotifyArtistList *list = malloc(sizeof(SpotifyArtistList));
    if (!list) {
        json_object_put(root);
        return NULL;
    }

    list->artists = malloc(sizeof(SpotifyArtist) * actual_count);
    if (!list->artists) {
        free(list);
        json_object_put(root);
        return NULL;
    }

    list->count = actual_count;
    list->total = actual_count;

    for (int i = 0; i < actual_count; i++) {
        struct json_object *item = json_object_array_get_idx(artists_array, i);
        
        // Handle null entries (invalid IDs)
        if (item && json_object_get_type(item) != json_type_null) {
            parse_artist_json(item, &list->artists[i]);
        } else {
            memset(&list->artists[i], 0, sizeof(SpotifyArtist));
        }
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

