#ifndef SPOTIFY_INTERNAL_H
#define SPOTIFY_INTERNAL_H

#include "api.h"
#include <json-c/json.h>
#include <stdbool.h>

// PUBLIC INTERFACE (WHAT USERS CAN SEE)

// ===== HTTP FUNCTIONS (spotify_http.c) =====

/**
 * Performs a GET request to Spotify API
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_get(SpotifyToken *token, const char *url);

/**
 * Performs a PUT request to Spotify API
 * Returns true if response code is 200
 */
bool spotify_api_put(SpotifyToken *token, const char *url, const char *json_data);

/**
 * Performs a PUT request to Spotify API (without body)
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_put_empty(SpotifyToken *token, const char *url);

/**
 * Performs a POST request to Spotify API
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_post(SpotifyToken *token, const char *url, const char *json_data);

/**
 * Performs a POST request to Spotify API (without body)
 * Returns true if response code is 204 (No Content)
 */
bool spotify_api_post_empty(SpotifyToken *token, const char *url);

/**
 * Performs a POST request to Spotify API and returns JSON response
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_post_json(SpotifyToken *token, const char *url, const char *json_data);

/**
 * Performs a DELETE request to Spotify API with JSON body
 * Returns parsed JSON object or NULL on error
 */
struct json_object* spotify_api_delete_json(SpotifyToken *token, const char *url, const char *json_data);

/**
 * Performs a DELETE request to Spotify API (without body)
 * Returns true if response code is 200 or 204
 */
bool spotify_api_delete_empty(SpotifyToken *token, const char *url);

/**
 * URL-encodes a string for use in HTTP requests
 * Returns allocated string that must be freed by caller
 */
char* url_encode(const char *str);

// ===== PARSER FUNCTIONS (spotify_parsers.c) =====

/**
 * Parse track, artist, playlist, device, player state data from JSON object into SpotifyTrack struct
 */
void parse_track_json(struct json_object *item, SpotifyTrack *track);
void parse_artist_json(struct json_object *item, SpotifyArtist *artist);
void parse_playlist_json(struct json_object *item, SpotifyPlaylist *playlist);
void parse_playlist_full_json(struct json_object *root, SpotifyPlaylistFull *playlist);
void parse_device_json(struct json_object *device_obj, SpotifyDevice *device);
void parse_player_state_json(struct json_object *root, SpotifyPlayerState *state);

/**
 * Parse queue data from JSON object into SpotifyQueue struct
 */
void parse_queue_json(struct json_object *root, SpotifyQueue *queue);

#endif // SPOTIFY_INTERNAL_H
