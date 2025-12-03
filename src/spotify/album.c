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
