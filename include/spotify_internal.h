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
 * URL-encodes a string for use in HTTP requests
 * Returns allocated string that must be freed by caller
 */
char* url_encode(const char *str);

// ===== PARSER FUNCTIONS (spotify_parsers.c) =====

/**
 * Parse track data from JSON object into SpotifyTrack struct
 */
void parse_track_json(struct json_object *item, SpotifyTrack *track);

/**
 * Parse artist data from JSON object into SpotifyArtist struct
 */
void parse_artist_json(struct json_object *item, SpotifyArtist *artist);

/**
 * Parse playlist data from JSON object into SpotifyPlaylist struct
 */
void parse_playlist_json(struct json_object *item, SpotifyPlaylist *playlist);

/**
 * Parse device data from JSON object into SpotifyDevice struct
 */
void parse_device_json(struct json_object *device_obj, SpotifyDevice *device);

/**
 * Parse player state from JSON object into SpotifyPlayerState struct
 */
void parse_player_state_json(struct json_object *root, SpotifyPlayerState *state);

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

#endif // SPOTIFY_INTERNAL_H
