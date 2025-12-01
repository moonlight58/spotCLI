#include "spotify/spotify_search.h"


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

SpotifyAlbumList* spotify_search_albums(SpotifyToken *token, const char *query, int limit) {
    if (!token || !query) {
        fprintf(stderr, "Invalid parameters for search_albums\n");
        return NULL;
    }

    if (limit <= 0) limit = 20;
    if (limit > 50) limit = 50;

    char *encoded_query = url_encode(query);
    if (!encoded_query) return NULL;

    char url[512];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/search?q=%s&type=album&limit=%d",
             encoded_query, limit);
    free(encoded_query);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) return NULL;

    struct json_object *albums_obj, *items;
    if (!json_object_object_get_ex(root, "albums", &albums_obj) ||
        !json_object_object_get_ex(albums_obj, "items", &items)) {
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    SpotifyAlbumList *list = malloc(sizeof(SpotifyAlbumList));
    list->albums = malloc(sizeof(SpotifyAlbum) * count);
    list->count = count;

    struct json_object *total_obj;
    if (json_object_object_get_ex(albums_obj, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *obj;

        memset(&list->albums[i], 0, sizeof(SpotifyAlbum));

        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(list->albums[i].id, json_object_get_string(obj),
                    sizeof(list->albums[i].id) - 1);
        }

        if (json_object_object_get_ex(item, "name", &obj)) {
            strncpy(list->albums[i].name, json_object_get_string(obj),
                    sizeof(list->albums[i].name) - 1);
        }

        struct json_object *artists;
        if (json_object_object_get_ex(item, "artists", &artists) &&
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (json_object_object_get_ex(artist, "name", &obj)) {
                strncpy(list->albums[i].artist, json_object_get_string(obj),
                        sizeof(list->albums[i].artist) - 1);
            }
        }
    }

    json_object_put(root);
    return list;
}
