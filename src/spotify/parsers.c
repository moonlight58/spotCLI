#include "spotify/spotify_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parse track data from JSON object into SpotifyTrack struct
 */
void parse_track_json(struct json_object *item, SpotifyTrack *track) {
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

    // Album
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

void parse_album_json(struct json_object *item, SpotifyAlbum *album) {
    struct json_object *obj;

    memset(album, 0, sizeof(SpotifyAlbum));

    // ID
    if(json_object_object_get_ex(item, "id", &obj)) {
        strncpy(album->id, json_object_get_string(obj), sizeof(album->id) - 1);
    }

    // NAME
    if (json_object_object_get_ex(item, "name", &obj)) {
        strncpy(album->name, json_object_get_string(obj), sizeof(album->name) - 1);
    }

    // ARTIST
    struct json_object *artists;
    if (json_object_object_get_ex(item, "artists", &artists) && json_object_array_length(artists) > 0) {
        struct json_object *artist = json_object_array_get_idx(artists, 0);
        if (json_object_object_get_ex(artist, "name", &obj)) {
            strncpy(album->artist, json_object_get_string(obj), sizeof(album->artist) - 1);
        }
    }

    // TRACKS
    struct json_object *tracks;
    if (json_object_object_get_ex(root, "tracks", &tracks)) {
        // get total count
        if (json_object_object_get_ex(tracks, "total", &obj)) {
            album
        }
    }
}

/**
 * Parse artist data from JSON object into SpotifyArtist struct
 */
void parse_artist_json(struct json_object *item, SpotifyArtist *artist) {
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

/**
 * Parse playlist data from JSON object into SpotifyPlaylist struct
 */
void parse_playlist_json(struct json_object *item, SpotifyPlaylist *playlist) {
    struct json_object *obj;

    memset(playlist, 0, sizeof(SpotifyPlaylist));

    // ID
    if (json_object_object_get_ex(item, "id", &obj)) {
        strncpy(playlist->id, json_object_get_string(obj), sizeof(playlist->id) - 1);
    }

    // Name
    if (json_object_object_get_ex(item, "name", &obj)) {
        strncpy(playlist->name, json_object_get_string(obj), sizeof(playlist->name) - 1);
    }

    // URI
    if (json_object_object_get_ex(item, "uri", &obj)) {
        strncpy(playlist->uri, json_object_get_string(obj), sizeof(playlist->uri) - 1);
    }

    // Public (correction du type dans api.h si nécessaire)
    if (json_object_object_get_ex(item, "public", &obj)) {
        playlist->is_public = json_object_get_boolean(obj);
    }

    // Track count
    struct json_object *tracks_obj;
    if (json_object_object_get_ex(item, "tracks", &tracks_obj) &&
        json_object_object_get_ex(tracks_obj, "total", &obj)) {
        playlist->count_tracks = json_object_get_int(obj);
    }
}

/**
 * Parse device data from JSON object into SpotifyDevice struct
 */
void parse_device_json(struct json_object *device_obj, SpotifyDevice *device) {
    struct json_object *obj;

    memset(device, 0, sizeof(SpotifyDevice));

    if (json_object_object_get_ex(device_obj, "id", &obj)) {
        const char *id = json_object_get_string(obj);
        if (id) {
            strncpy(device->device_id, id, sizeof(device->device_id) - 1);
        }
    }

    if (json_object_object_get_ex(device_obj, "name", &obj)) {
        strncpy(device->device_name, json_object_get_string(obj),
                sizeof(device->device_name) - 1);
    }

    if (json_object_object_get_ex(device_obj, "type", &obj)) {
        strncpy(device->device_type, json_object_get_string(obj),
                sizeof(device->device_type) - 1);
    }

    if (json_object_object_get_ex(device_obj, "volume_percent", &obj)) {
        device->volume_percent = json_object_get_int(obj);
    }

    if (json_object_object_get_ex(device_obj, "is_active", &obj)) {
        device->is_active = json_object_get_boolean(obj);
    }

    if (json_object_object_get_ex(device_obj, "is_private_session", &obj)) {
        device->is_private_session = json_object_get_boolean(obj);
    }

    if (json_object_object_get_ex(device_obj, "is_restricted", &obj)) {
        device->is_restricted = json_object_get_boolean(obj);
    }
}

/**
 * Parse player state from JSON object into SpotifyPlayerState struct
 */
void parse_player_state_json(struct json_object *root, SpotifyPlayerState *state) {
    struct json_object *obj;

    memset(state, 0, sizeof(SpotifyPlayerState));

    // Parse track item
    struct json_object *item;
    if (json_object_object_get_ex(root, "item", &item) && item) {
        // Track ID
        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(state->track_id, json_object_get_string(obj),
                    sizeof(state->track_id) - 1);
        }

        // Track name
        if (json_object_object_get_ex(item, "name", &obj)) {
            strncpy(state->track_name, json_object_get_string(obj),
                    sizeof(state->track_name) - 1);
        }

        // Artist name
        struct json_object *artists;
        if (json_object_object_get_ex(item, "artists", &artists) &&
            json_object_array_length(artists) > 0) {
            struct json_object *artist = json_object_array_get_idx(artists, 0);
            if (json_object_object_get_ex(artist, "name", &obj)) {
                strncpy(state->artist_name, json_object_get_string(obj),
                        sizeof(state->artist_name) - 1);
            }
        }

        // Album name
        struct json_object *album;
        if (json_object_object_get_ex(item, "album", &album) &&
            json_object_object_get_ex(album, "name", &obj)) {
            strncpy(state->album_name, json_object_get_string(obj),
                    sizeof(state->album_name) - 1);
        }

        // Duration
        if (json_object_object_get_ex(item, "duration_ms", &obj)) {
            state->duration_ms = json_object_get_int(obj);
        }

        // URI
        if (json_object_object_get_ex(item, "uri", &obj)) {
            strncpy(state->track_uri, json_object_get_string(obj),
                    sizeof(state->track_uri) - 1);
        }
    }

    // Playback state
    if (json_object_object_get_ex(root, "is_playing", &obj)) {
        state->is_playing = json_object_get_boolean(obj);
    }

    if (json_object_object_get_ex(root, "progress_ms", &obj)) {
        state->progress_ms = json_object_get_int(obj);
    }

    if (json_object_object_get_ex(root, "timestamp", &obj)) {
        state->timestamp = json_object_get_int(obj);
    }

    // Context
    struct json_object *context;
    if (json_object_object_get_ex(root, "context", &context) && context) {
        if (json_object_object_get_ex(context, "type", &obj)) {
            strncpy(state->context_type, json_object_get_string(obj),
                    sizeof(state->context_type) - 1);
        }

        if (json_object_object_get_ex(context, "uri", &obj)) {
            strncpy(state->context_uri, json_object_get_string(obj),
                    sizeof(state->context_uri) - 1);
        }
    }

    // Settings
    if (json_object_object_get_ex(root, "shuffle_state", &obj)) {
        state->shuffle_state = json_object_get_boolean(obj);
    }

    if (json_object_object_get_ex(root, "repeat_state", &obj)) {
        strncpy(state->repeat_state, json_object_get_string(obj),
                sizeof(state->repeat_state) - 1);
    }

    // Device
    struct json_object *device;
    if (json_object_object_get_ex(root, "device", &device)) {
        parse_device_json(device, &state->device);
    }
}

/**
 * Parse queue data from JSON object into SpotifyQueue struct
 */
void parse_queue_json(struct json_object *root, SpotifyQueue *queue) {
    memset(queue, 0, sizeof(SpotifyQueue));

    // Parse currently playing track
    struct json_object *currently_playing;
    if (json_object_object_get_ex(root, "currently_playing", &currently_playing) && currently_playing) {
        parse_track_json(currently_playing, &queue->currently_playing);
    }

    // Parse queue array
    struct json_object *queue_array;
    if (json_object_object_get_ex(root, "queue", &queue_array)) {
        int count = json_object_array_length(queue_array);

        if (count > 0) {
            queue->queue = malloc(sizeof(SpotifyTrack) * count);
            if (queue->queue) {
                queue->queue_count = count;

                for (int i = 0; i < count; i++) {
                    struct json_object *item = json_object_array_get_idx(queue_array, i);
                    parse_track_json(item, &queue->queue[i]);
                }
            }
        }
    }
}

/**
 * Parse full playlist data from JSON object into SpotifyPlaylistFull struct
 */
void parse_playlist_full_json(struct json_object *root, SpotifyPlaylistFull *playlist) {
    struct json_object *obj;

    memset(playlist, 0, sizeof(SpotifyPlaylistFull));

    // ID
    if (json_object_object_get_ex(root, "id", &obj)) {
        strncpy(playlist->id, json_object_get_string(obj), sizeof(playlist->id) - 1);
    }

    // Name
    if (json_object_object_get_ex(root, "name", &obj)) {
        strncpy(playlist->name, json_object_get_string(obj), sizeof(playlist->name) - 1);
    }

    // Description
    if (json_object_object_get_ex(root, "description", &obj) && obj) {
        const char *desc = json_object_get_string(obj);
        if (desc) {
            strncpy(playlist->description, desc, sizeof(playlist->description) - 1);
        }
    }

    // URI
    if (json_object_object_get_ex(root, "uri", &obj)) {
        strncpy(playlist->uri, json_object_get_string(obj), sizeof(playlist->uri) - 1);
    }

    // Snapshot ID
    if (json_object_object_get_ex(root, "snapshot_id", &obj)) {
        strncpy(playlist->snapshot_id, json_object_get_string(obj), sizeof(playlist->snapshot_id) - 1);
    }

    // Public
    if (json_object_object_get_ex(root, "public", &obj)) {
        playlist->is_public = json_object_get_boolean(obj);
    }

    // Collaborative
    if (json_object_object_get_ex(root, "collaborative", &obj)) {
        playlist->is_collaborative = json_object_get_boolean(obj);
    }

    // Owner
    struct json_object *owner;
    if (json_object_object_get_ex(root, "owner", &owner)) {
        if (json_object_object_get_ex(owner, "id", &obj)) {
            strncpy(playlist->owner_id, json_object_get_string(obj), sizeof(playlist->owner_id) - 1);
        }
        if (json_object_object_get_ex(owner, "display_name", &obj) && obj) {
            const char *name = json_object_get_string(obj);
            if (name) {
                strncpy(playlist->owner_name, name, sizeof(playlist->owner_name) - 1);
            }
        }
    }

    // Tracks
    struct json_object *tracks_obj;
    if (json_object_object_get_ex(root, "tracks", &tracks_obj)) {
        // Get total count
        if (json_object_object_get_ex(tracks_obj, "total", &obj)) {
            playlist->tracks_count = json_object_get_int(obj);
        }

        // Parse track items if available
        struct json_object *items;
        if (json_object_object_get_ex(tracks_obj, "items", &items)) {
            int count = json_object_array_length(items);

            if (count > 0) {
                playlist->tracks = malloc(sizeof(SpotifyTrack) * count);
                if (playlist->tracks) {
                    playlist->tracks_count = count;  // Update with actual count

                    for (int i = 0; i < count; i++) {
                        struct json_object *item = json_object_array_get_idx(items, i);
                        struct json_object *track;

                        // Handle both direct tracks and wrapped tracks (playlist format)
                        if (json_object_object_get_ex(item, "track", &track)) {
                            parse_track_json(track, &playlist->tracks[i]);
                        } else {
                            parse_track_json(item, &playlist->tracks[i]);
                        }
                    }
                }
            }
        }
    }
}
