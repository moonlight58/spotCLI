#include "spotify/spotify_player.h"
#include <stdio.h>

static char *context_repeat[] = {"off", "context", "track"};
static int context_index = 0;

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

bool spotify_toggle_playback_repeat(SpotifyToken *token, const char *device_id) {
    char url[256];

    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/repeat?state=%s&device_id=%s",
                context_repeat[context_index++], device_id);
    } else {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/repeat?state=%s",
                context_repeat[context_index++]);
    }

    context_index++;
    return spotify_api_put_empty(token, url);
}

/**
 * Transfer playback to a different device
 */
bool spotify_transfer_playback(SpotifyToken *token, const char *device_id, bool play) {
    if (!token || !device_id) {
        fprintf(stderr, "Invalid parameters for transfer_playback\n");
        return false;
    }

    const char *url = "https://api.spotify.com/v1/me/player";

    // Build JSON body
    struct json_object *root = json_object_new_object();
    struct json_object *device_ids = json_object_new_array();

    json_object_array_add(device_ids, json_object_new_string(device_id));
    json_object_object_add(root, "device_ids", device_ids);
    json_object_object_add(root, "play", json_object_new_boolean(play));

    const char *json_str = json_object_to_json_string(root);

    bool result = spotify_api_put(token, url, json_str);

    json_object_put(root);
    return result;
}

bool spotify_set_playback_volume(SpotifyToken *token, const char *device_id, int volume) {
    char url[256];

    if (device_id && volume) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/volume?volume_percent=%d&device_id=%s",
                volume, device_id);
    } else {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/volume?volume_percent=%d",
                volume);
    }
    return spotify_api_put_empty(token, url);
}

/**
 * Get list of available devices
 */
SpotifyDevice* spotify_get_available_devices(SpotifyToken *token, int *device_count) {
    if (!token || !device_count) {
        fprintf(stderr, "Invalid parameters for get_available_devices\n");
        return NULL;
    }

    *device_count = 0;

    const char *url = "https://api.spotify.com/v1/me/player/devices";

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get devices\n");
        return NULL;
    }

    struct json_object *devices_array;
    if (!json_object_object_get_ex(root, "devices", &devices_array)) {
        fprintf(stderr, "No 'devices' field in response\n");
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(devices_array);
    if (count == 0) {
        fprintf(stderr, "No devices found\n");
        json_object_put(root);
        return NULL;
    }

    SpotifyDevice *devices = malloc(sizeof(SpotifyDevice) * count);
    if (!devices) {
        fprintf(stderr, "Failed to allocate memory for devices\n");
        json_object_put(root);
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        struct json_object *device_obj = json_object_array_get_idx(devices_array, i);
        parse_device_json(device_obj, &devices[i]);
    }

    *device_count = count;
    json_object_put(root);

    return devices;
}

/**
 * Get the current user's queue
 */
SpotifyQueue* spotify_get_queue(SpotifyToken *token) {
    if (!token) {
        fprintf(stderr, "Invalid token parameter\n");
        return NULL;
    }

    const char *url = "https://api.spotify.com/v1/me/player/queue";

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get queue (no active device or API error)\n");
        return NULL;
    }

    // Check if response is empty
    if (json_object_get_type(root) == json_type_null) {
        json_object_put(root);
        fprintf(stderr, "No active playback device found\n");
        return NULL;
    }

    SpotifyQueue *queue = malloc(sizeof(SpotifyQueue));
    if (!queue) {
        fprintf(stderr, "Failed to allocate memory for queue\n");
        json_object_put(root);
        return NULL;
    }

    parse_queue_json(root, queue);
    json_object_put(root);

    return queue;
}

/**
 * Add an item to the end of the user's current playback queue
 */
bool spotify_add_to_queue(SpotifyToken *token, const char *uri, const char *device_id) {
    if (!token || !uri) {
        fprintf(stderr, "Invalid parameters for add_to_queue\n");
        return false;
    }

    char url[512];
    
    if (device_id) {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/queue?uri=%s&device_id=%s",
                uri, device_id);
    } else {
        snprintf(url, sizeof(url),
                "https://api.spotify.com/v1/me/player/queue?uri=%s",
                uri);
    }

    return spotify_api_post_empty(token, url);
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
