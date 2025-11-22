#include "spotify/spotify_advanced.h"
#include "spotify/spotify_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== ALBUM FUNCTIONS =====

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

// ===== USER PROFILE FUNCTIONS =====

SpotifyUserProfile* spotify_get_current_user_profile(SpotifyToken *token) {
    return spotify_get_user_profile(token, NULL);
}

SpotifyUserProfile* spotify_get_user_profile(SpotifyToken *token, const char *user_id) {
    if (!token) {
        fprintf(stderr, "Invalid token parameter\n");
        return NULL;
    }

    char url[256];
    if (user_id) {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/users/%s", user_id);
    } else {
        snprintf(url, sizeof(url), "https://api.spotify.com/v1/me");
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get user profile\n");
        return NULL;
    }

    SpotifyUserProfile *profile = malloc(sizeof(SpotifyUserProfile));
    if (!profile) {
        json_object_put(root);
        return NULL;
    }

    memset(profile, 0, sizeof(SpotifyUserProfile));

    struct json_object *obj;

    if (json_object_object_get_ex(root, "id", &obj)) {
        strncpy(profile->user_id, json_object_get_string(obj),
                sizeof(profile->user_id) - 1);
    }

    if (json_object_object_get_ex(root, "display_name", &obj)) {
        const char *name = json_object_get_string(obj);
        if (name) {
            strncpy(profile->display_name, name, sizeof(profile->display_name) - 1);
        }
    }

    struct json_object *followers;
    if (json_object_object_get_ex(root, "followers", &followers)) {
        if (json_object_object_get_ex(followers, "total", &obj)) {
            profile->followers = json_object_get_int(obj);
        }
    }

    if (json_object_object_get_ex(root, "external_urls", &obj)) {
        struct json_object *spotify_url;
        if (json_object_object_get_ex(obj, "spotify", &spotify_url)) {
            strncpy(profile->profile_url, json_object_get_string(spotify_url),
                    sizeof(profile->profile_url) - 1);
        }
    }

    struct json_object *images;
    if (json_object_object_get_ex(root, "images", &images) &&
        json_object_array_length(images) > 0) {
        struct json_object *image = json_object_array_get_idx(images, 0);
        if (json_object_object_get_ex(image, "url", &obj)) {
            strncpy(profile->profile_image_url, json_object_get_string(obj),
                    sizeof(profile->profile_image_url) - 1);
        }
    }

    json_object_put(root);
    return profile;
}

// ===== AUDIO FEATURES FUNCTIONS =====

SpotifyAudioFeatures* spotify_get_audio_features(SpotifyToken *token, const char *track_id) {
    if (!token || !track_id) {
        fprintf(stderr, "Invalid parameters for get_audio_features\n");
        return NULL;
    }

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/audio-features/%s", track_id);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get audio features\n");
        return NULL;
    }

    SpotifyAudioFeatures *features = malloc(sizeof(SpotifyAudioFeatures));
    if (!features) {
        json_object_put(root);
        return NULL;
    }

    memset(features, 0, sizeof(SpotifyAudioFeatures));

    struct json_object *obj;

    if (json_object_object_get_ex(root, "id", &obj)) {
        strncpy(features->track_id, json_object_get_string(obj),
                sizeof(features->track_id) - 1);
    }

    if (json_object_object_get_ex(root, "acousticness", &obj)) {
        features->acousticness = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "danceability", &obj)) {
        features->danceability = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "energy", &obj)) {
        features->energy = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "instrumentalness", &obj)) {
        features->instrumentalness = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "liveness", &obj)) {
        features->liveness = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "loudness", &obj)) {
        features->loudness = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "speechiness", &obj)) {
        features->speechiness = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "valence", &obj)) {
        features->valence = json_object_get_double(obj);
    }

    if (json_object_object_get_ex(root, "tempo", &obj)) {
        features->tempo = json_object_get_int(obj);
    }

    if (json_object_object_get_ex(root, "time_signature", &obj)) {
        features->time_signature = json_object_get_int(obj);
    }

    json_object_put(root);
    return features;
}

SpotifyAudioFeatures* spotify_get_audio_features_batch(SpotifyToken *token, const char **track_ids, int count) {
    if (!token || !track_ids || count <= 0 || count > 100) {
        fprintf(stderr, "Invalid parameters (max 100 tracks)\n");
        return NULL;
    }

    char url[2048] = "https://api.spotify.com/v1/audio-features?ids=";

    for (int i = 0; i < count; i++) {
        if (i > 0) strcat(url, ",");
        strcat(url, track_ids[i]);
    }

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get audio features batch\n");
        return NULL;
    }

    struct json_object *audio_features;
    if (!json_object_object_get_ex(root, "audio_features", &audio_features)) {
        json_object_put(root);
        return NULL;
    }

    int actual_count = json_object_array_length(audio_features);
    SpotifyAudioFeatures *features = malloc(sizeof(SpotifyAudioFeatures) * actual_count);
    if (!features) {
        json_object_put(root);
        return NULL;
    }

    for (int i = 0; i < actual_count; i++) {
        struct json_object *item = json_object_array_get_idx(audio_features, i);
        struct json_object *obj;

        memset(&features[i], 0, sizeof(SpotifyAudioFeatures));

        if (json_object_object_get_ex(item, "id", &obj)) {
            strncpy(features[i].track_id, json_object_get_string(obj),
                    sizeof(features[i].track_id) - 1);
        }

        if (json_object_object_get_ex(item, "acousticness", &obj)) {
            features[i].acousticness = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "danceability", &obj)) {
            features[i].danceability = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "energy", &obj)) {
            features[i].energy = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "instrumentalness", &obj)) {
            features[i].instrumentalness = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "liveness", &obj)) {
            features[i].liveness = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "loudness", &obj)) {
            features[i].loudness = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "speechiness", &obj)) {
            features[i].speechiness = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "valence", &obj)) {
            features[i].valence = json_object_get_double(obj);
        }

        if (json_object_object_get_ex(item, "tempo", &obj)) {
            features[i].tempo = json_object_get_int(obj);
        }

        if (json_object_object_get_ex(item, "time_signature", &obj)) {
            features[i].time_signature = json_object_get_int(obj);
        }
    }

    json_object_put(root);
    return features;
}

// ===== RECOMMENDATIONS FUNCTIONS =====

SpotifyRecommendations* spotify_get_recommendations(SpotifyToken *token, const char **seed_tracks, const char **seed_artists, const char **seed_genres, int seed_count, int limit) {
    if (!token || seed_count <= 0 || seed_count > 5) {
        fprintf(stderr, "Invalid parameters (need 1-5 seeds)\n");
        return NULL;
    }

    if (limit <= 0) limit = 20;
    if (limit > 100) limit = 100;

    char url[2048] = "https://api.spotify.com/v1/recommendations?";
    char seed_str[512] = {0};

    // Build seed strings
    if (seed_tracks) {
        strcat(seed_str, "seed_tracks=");
        for (int i = 0; i < seed_count && seed_tracks[i]; i++) {
            if (i > 0) strcat(seed_str, ",");
            strcat(seed_str, seed_tracks[i]);
        }
    }

    if (seed_artists) {
        if (strlen(seed_str) > 0) strcat(seed_str, "&");
        strcat(seed_str, "seed_artists=");
        for (int i = 0; i < seed_count && seed_artists[i]; i++) {
            if (i > 0) strcat(seed_str, ",");
            strcat(seed_str, seed_artists[i]);
        }
    }

    if (seed_genres) {
        if (strlen(seed_str) > 0) strcat(seed_str, "&");
        strcat(seed_str, "seed_genres=");
        for (int i = 0; i < seed_count && seed_genres[i]; i++) {
            if (i > 0) strcat(seed_str, ",");
            strcat(seed_str, seed_genres[i]);
        }
    }

    snprintf(url, sizeof(url), "%s%s&limit=%d", url, seed_str, limit);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get recommendations\n");
        return NULL;
    }

    struct json_object *tracks;
    if (!json_object_object_get_ex(root, "tracks", &tracks)) {
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(tracks);
    SpotifyRecommendations *recs = malloc(sizeof(SpotifyRecommendations));
    recs->tracks = malloc(sizeof(SpotifyTrack) * count);
    recs->count = count;

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(tracks, i);
        parse_track_json(item, &recs->tracks[i]);
    }

    json_object_put(root);
    return recs;
}

// ===== RECENTLY PLAYED FUNCTIONS =====

SpotifyRecentlyPlayed* spotify_get_recently_played(SpotifyToken *token, int limit) {
    if (!token) {
        fprintf(stderr, "Invalid token parameter\n");
        return NULL;
    }

    if (limit <= 0) limit = 20;
    if (limit > 50) limit = 50;

    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/me/player/recently-played?limit=%d",
             limit);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) {
        fprintf(stderr, "Failed to get recently played\n");
        return NULL;
    }

    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    SpotifyRecentlyPlayed *history = malloc(sizeof(SpotifyRecentlyPlayed));
    history->history = malloc(sizeof(SpotifyPlayHistory) * count);
    history->count = count;

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        struct json_object *track;

        if (json_object_object_get_ex(item, "track", &track)) {
            parse_track_json(track, &history->history[i].track);
        }

        struct json_object *obj;
        if (json_object_object_get_ex(item, "played_at", &obj)) {
            strncpy(history->history[i].played_at, json_object_get_string(obj),
                    sizeof(history->history[i].played_at) - 1);
        }

        struct json_object *context;
        if (json_object_object_get_ex(item, "context", &context) && context) {
            if (json_object_object_get_ex(context, "type", &obj)) {
                strncpy(history->history[i].context_type, json_object_get_string(obj),
                        sizeof(history->history[i].context_type) - 1);
            }
        }
    }

    json_object_put(root);
    return history;
}

