#include "spotify/spotify_playlist.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

SpotifyPlaylistFull* spotify_create_playlist(SpotifyToken *token, const char *name, const char *description, bool is_public, bool is_collaborative) {
    if (!token || !name) {
        fprintf(stderr, "Invalid parameters for create_playlist\n");
        return NULL;
    }

    // Get current user ID
    char *user_id = spotify_get_current_user_id(token);
    if (!user_id) {
        fprintf(stderr, "Failed to get current user ID\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/users/%s/playlists", user_id);
    free(user_id);

    // Build JSON body
    struct json_object *body = json_object_new_object();
    json_object_object_add(body, "name", json_object_new_string(name));
    json_object_object_add(body, "public", json_object_new_boolean(is_public));
    json_object_object_add(body, "collaborative", json_object_new_boolean(is_collaborative));

    if (description) {
        json_object_object_add(body, "description", json_object_new_string(description));
    }

    const char *json_str = json_object_to_json_string(body);

    // Use POST request that returns JSON response
    struct json_object *response = spotify_api_post_json(token, url, json_str);
    json_object_put(body);

    if (!response) {
        fprintf(stderr, "Failed to create playlist\n");
        return NULL;
    }

    SpotifyPlaylistFull *playlist = malloc(sizeof(SpotifyPlaylistFull));
    if (!playlist) {
        json_object_put(response);
        return NULL;
    }

    parse_playlist_full_json(response, playlist);
    json_object_put(response);

    return playlist;
}

SpotifyPlaylistFull* spotify_get_playlist(SpotifyToken *token, const char *playlist_id, bool fetch_tracks, int track_limit) {
    if (!token || !playlist_id) {
        fprintf(stderr, "Invalid parameters for get_playlist\n");
        return NULL;
    }

    if (track_limit <= 0) track_limit = 100;
    if (track_limit > 100) track_limit = 100;

    char url[512];
    if (fetch_tracks) {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/playlists/%s?fields=id,name,description,uri,"
                 "snapshot_id,public,collaborative,owner(id,display_name),"
                 "tracks(total,items(track(id,name,uri,duration_ms,artists(name),album(name))))"
                 "&limit=%d",
                 playlist_id, track_limit);
    } else {
        snprintf(url, sizeof(url),
                 "https://api.spotify.com/v1/playlists/%s?fields=id,name,description,uri,"
                 "snapshot_id,public,collaborative,owner(id,display_name),tracks(total)",
                 playlist_id);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get playlist\n");
        return NULL;
    }

    SpotifyPlaylistFull *playlist = malloc(sizeof(SpotifyPlaylistFull));
    if (!playlist) {
        json_object_put(root);
        return NULL;
    }

    parse_playlist_full_json(root, playlist);
    json_object_put(root);

    return playlist;
}

bool spotify_update_playlist(SpotifyToken *token, const char *playlist_id, SpotifyPlaylistUpdate *updates) {
    if (!token || !playlist_id || !updates) {
        fprintf(stderr, "Invalid parameters for update_playlist\n");
        return false;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s", playlist_id);

    // Build JSON body with only non-NULL fields
    struct json_object *body = json_object_new_object();

    if (updates->name) {
        json_object_object_add(body, "name", json_object_new_string(updates->name));
    }
    if (updates->description) {
        json_object_object_add(body, "description", json_object_new_string(updates->description));
    }
    if (updates->is_public) {
        json_object_object_add(body, "public", json_object_new_boolean(*updates->is_public));
    }
    if (updates->is_collaborative) {
        json_object_object_add(body, "collaborative",
                               json_object_new_boolean(*updates->is_collaborative));
    }

    const char *json_str = json_object_to_json_string(body);

    bool result = spotify_api_put(token, url, json_str);
    json_object_put(body);

    return result;
}

SpotifyPlaylistResult* spotify_add_tracks_to_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, int position) {
    if (!token || !playlist_id || !track_uris || count <= 0) {
        fprintf(stderr, "Invalid parameters for add_tracks_to_playlist\n");
        return NULL;
    }

    if (count > 100) {
        fprintf(stderr, "Cannot add more than 100 tracks at once\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);

    // Build JSON body
    struct json_object *body = json_object_new_object();
    struct json_object *uris_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        json_object_array_add(uris_array, json_object_new_string(track_uris[i]));
    }

    json_object_object_add(body, "uris", uris_array);

    if (position >= 0) {
        json_object_object_add(body, "position", json_object_new_int(position));
    }

    const char *json_str = json_object_to_json_string(body);

    struct json_object *response = spotify_api_post_json(token, url, json_str);
    json_object_put(body);

    if (!response) {
        fprintf(stderr, "Failed to add tracks to playlist\n");
        return NULL;
    }

    SpotifyPlaylistResult *result = malloc(sizeof(SpotifyPlaylistResult));
    if (!result) {
        json_object_put(response);
        return NULL;
    }

    result->success = true;

    struct json_object *snapshot_obj;
    if (json_object_object_get_ex(response, "snapshot_id", &snapshot_obj)) {
        strncpy(result->snapshot_id, json_object_get_string(snapshot_obj),
                sizeof(result->snapshot_id) - 1);
    }

    json_object_put(response);
    return result;
}
SpotifyPlaylistResult* spotify_remove_tracks_from_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, const char *snapshot_id) {
    if (!token || !playlist_id || !track_uris || count <= 0) {
        fprintf(stderr, "Invalid parameters for remove_tracks_from_playlist\n");
        return NULL;
    }

    if (count > 100) {
        fprintf(stderr, "Cannot remove more than 100 tracks at once\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://api.spotify.com/v1/playlists/%s/tracks", playlist_id);

    // Build JSON body
    struct json_object *body = json_object_new_object();
    struct json_object *tracks_array = json_object_new_array();

    for (int i = 0; i < count; i++) {
        struct json_object *track_obj = json_object_new_object();
        json_object_object_add(track_obj, "uri", json_object_new_string(track_uris[i]));
        json_object_array_add(tracks_array, track_obj);
    }

    json_object_object_add(body, "tracks", tracks_array);

    if (snapshot_id) {
        json_object_object_add(body, "snapshot_id", json_object_new_string(snapshot_id));
    }

    const char *json_str = json_object_to_json_string(body);

    struct json_object *response = spotify_api_delete_json(token, url, json_str);
    json_object_put(body);

    if (!response) {
        fprintf(stderr, "Failed to remove tracks from playlist\n");
        return NULL;
    }
    SpotifyPlaylistResult *result = malloc(sizeof(SpotifyPlaylistResult));
    if (!result) {
        json_object_put(response);
        return NULL;
    }

    result->success = true;

    struct json_object *snapshot_obj;
    if (json_object_object_get_ex(response, "snapshot_id", &snapshot_obj)) {
        strncpy(result->snapshot_id, json_object_get_string(snapshot_obj),
                sizeof(result->snapshot_id) - 1);
    }

    json_object_put(response);
    return result;
}

bool spotify_unfollow_playlist(SpotifyToken *token, const char *playlist_id) {
    if (!token || !playlist_id) {
        fprintf(stderr, "Invalid parameters for unfollow_playlist\n");
        return NULL;
    }
    char url[256];

    snprintf(url, sizeof(url),
            "https://api.spotify.com/v1/playlists/%s/followers",
            playlist_id);
    return spotify_api_delete_empty(token, url);
}

