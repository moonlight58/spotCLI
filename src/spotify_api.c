#include "api.h"
#include "spotify_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

SpotifyPlaylistList* spotify_get_user_playlists(SpotifyToken *token, int limit, int offset) {
    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/me/playlists?limit=%d&offset=%d",
             limit, offset);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) return NULL;

    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    SpotifyPlaylistList *list = malloc(sizeof(SpotifyPlaylistList));
    list->playlists = malloc(sizeof(SpotifyPlaylist) * count);
    list->count = count;

    struct json_object *total_obj;
    if (json_object_object_get_ex(root, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        parse_playlist_json(item, &list->playlists[i]);
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

SpotifyPlayerState* spotify_get_player_state(SpotifyToken *token) {
    const char *url = "https://api.spotify.com/v1/me/player";

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get player state (no active device or API error)\n");
        return NULL;
    }

    // Check if response is empty (no active device)
    if (json_object_get_type(root) == json_type_null) {
        json_object_put(root);
        fprintf(stderr, "No active playback device found\n");
        return NULL;
    }

    SpotifyPlayerState *state = malloc(sizeof(SpotifyPlayerState));
    if (!state) {
        json_object_put(root);
        return NULL;
    }

    parse_player_state_json(root, state);
    json_object_put(root);

    return state;
}

/**
 * Pause playback on the user's active device
 *
 * @param token - Valid Spotify token
 * @param device_id - Optional: specific device ID (NULL for current active device)
 * @return true if successful, false otherwise
 */
bool spotify_pause_playback(SpotifyToken *token, const char *device_id) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/pause?device_id=%s",
                device_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/pause");
    }

    return spotify_api_put_empty(token, url);
}

/**
 * Resume playback on the user's active device
 *
 * @param token - Valid Spotify token
 * @param device_id - Optional: specific device ID (NULL for current active device)
 * @return true if successful, false otherwise
 */
bool spotify_resume_playback(SpotifyToken *token, const char *device_id) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/me/player/play?device_id=%s",
                 device_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/play");
    }

    return spotify_api_post(token, url, NULL);
}

/**
 * Start/Resume playback with specific context or tracks
 *
 * @param token - Valid Spotify token
 * @param device_id - Optional: specific device ID (NULL for current active device)
 * @param context_uri - Optional: Spotify URI of context (playlist, album, artist)
 * @param uris - Optional: Array of track URIs to play
 * @param uri_count - Number of URIs in the array
 * @return true if successful, false otherwise
 *
 * Examples:
 * - Play a playlist: context_uri = "spotify:playlist:37i9dQZF1DXcBWIGoYBM5M"
 * - Play specific tracks: uris = {"spotify:track:4iV5W9uYEdYUVa79Axb7Rh", ...}
 */
bool spotify_start_playback(SpotifyToken *token, const char *device_id,
                            const char *context_uri, const char **uris, int uri_count) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/me/player/play?device_id=%s",
                 device_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/play");
    }

    // Build JSON body if context_uri or uris are provided
    const char *json_str = NULL;
    struct json_object *root = NULL;

    if (context_uri || (uris && uri_count > 0)) {
        root = json_object_new_object();

        // Add context URI (playlist, album, artist)
        if (context_uri) {
            json_object_object_add(root, "context_uri",
                                  json_object_new_string(context_uri));
        }

        // Add specific track URIs
        if (uris && uri_count > 0) {
            struct json_object *uris_array = json_object_new_array();
            for (int i = 0; i < uri_count; i++) {
                json_object_array_add(uris_array, json_object_new_string(uris[i]));
            }
            json_object_object_add(root, "uris", uris_array);
        }

        json_str = json_object_to_json_string(root);
    }

    bool result = spotify_api_post(token, url, json_str);

    if (root) {
        json_object_put(root);
    }

    return result;
}

bool spotify_skip_next_playback(SpotifyToken *token, const char *device_id) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/next?device_id=%s",
                device_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/next");
    }

    return spotify_api_post_empty(token, url);
}

bool spotify_skip_previous_playback(SpotifyToken *token, const char *device_id) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/previous?device_id=%s",
                device_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me/player/previous");
    }

    return spotify_api_post_empty(token, url);
}

/**
 * Toggle between play and pause based on current state
 */
bool spotify_toggle_playback(SpotifyToken *token) {
    SpotifyPlayerState *state = spotify_get_player_state(token);
    if (!state) {
        fprintf(stderr, "Cannot toggle: no active playback\n");
        return false;
    }

    bool result;
    if (state->is_playing) {
        printf("⏸ Pausing...\n");
        result = spotify_pause_playback(token, NULL);
    } else {
        printf("▶ Resuming...\n");
        result = spotify_resume_playback(token, NULL);
    }

    spotify_free_player_state(state);
    return result;
}

bool spotify_toggle_playback_shuffle(SpotifyToken *token, const char *device_id, bool state_shuffle) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/shuffle?state=%d&device_id=%s",
                state_shuffle, device_id);
    } else {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/shuffle?state=%d", state_shuffle);
    }

    return spotify_api_put_empty(token, url);
}
