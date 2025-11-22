#ifndef API_H
#define API_H

#include "auth.h"
#include <stdbool.h>

typedef struct {
    char id[64];
    char name[256];
    char artist[256];
    char album[256];
    int duration_ms;
    char uri[128];
} SpotifyTrack;

typedef struct { // List of all the tracks
    SpotifyTrack *tracks;
    int count;
    int total;
} SpotifyTrackList;

typedef struct {
    char id[64];
    char name[256];
    char genres[512];  // Comma-separated genres
    int followers;
    int popularity;
    char uri[128];
    char image_url[512];
} SpotifyArtist;

typedef struct { // List of all the artists
    SpotifyArtist *artists;
    int count;
    int total;
} SpotifyArtistList;

typedef struct {
    char id[64];
    char name[256];
    char artist[256];
} SpotifyAlbum;

typedef struct { // List of all the albums
    SpotifyAlbum *albums;
    int count;
    int total;
} SpotifyAlbumList;

typedef struct {
    char id[64];
    char name[256];
    char uri[128];
    bool is_public;
    int count_tracks;
} SpotifyPlaylist;

typedef struct { // List of all the playlists
    SpotifyPlaylist* playlists;
    int count;
    int total;
} SpotifyPlaylistList;

// Full playlist with tracks and details
typedef struct {
    char id[64];
    char name[256];
    char description[1024];
    char uri[128];
    char snapshot_id[128];
    bool is_public;
    bool is_collaborative;
    char owner_id[64];
    char owner_name[256];
    int tracks_count;
    SpotifyTrack *tracks;  // Array of tracks (if fetched)
} SpotifyPlaylistFull;

// Structure for updating playlist details
typedef struct {
    const char *name;
    const char *description;
    bool *is_public;
    bool *is_collaborative;
} SpotifyPlaylistUpdate;

// Result from playlist operations
typedef struct {
    bool success;
    char snapshot_id[128];
} SpotifyPlaylistResult;

typedef struct {
    char device_id[64];
    char device_name[256];
    char device_type[64];  // "Computer", "Smartphone", "Speaker", etc.
    int volume_percent;
    bool is_active;
    bool is_private_session;
    bool is_restricted;
} SpotifyDevice;

typedef struct {
    // Track info
    char track_id[64];
    char track_name[256];
    char artist_name[256];
    char album_name[256];
    int duration_ms;
    char track_uri[128];
    
    // Playback state
    bool is_playing;
    int progress_ms;
    int timestamp;  // Unix timestamp
    
    // Context (playlist, album, etc.)
    char context_type[32];  // "playlist", "album", "artist", etc.
    char context_uri[128];
    
    // Settings
    bool shuffle_state;
    char repeat_state[16];  // "off", "track", "context"
    
    // Device
    SpotifyDevice device;
} SpotifyPlayerState;

typedef struct {
    SpotifyTrack currently_playing;
    SpotifyTrack *queue;
    int queue_count;
} SpotifyQueue;

// Album Detailed
typedef struct {
    SpotifyTrack *tracks;
    int count;
    SpotifyAlbumInfo album_info;
} SpotifyAlbumDetailed;

// User Profile
typedef struct {
    char user_id[64];
    char display_name[256];
    int followers;
    int public_playlists;
    char profile_url[512];
    char profile_image_url[512];
} SpotifyUserProfile;

// Audio Features
typedef struct {
    char track_id[64];
    char track_name[256];
    float acousticness;
    float danceability;
    float energy;
    float instrumentalness;
    float liveness;
    float loudness;
    float speechiness;
    float valence;
    int tempo;
    int time_signature;
} SpotifyAudioFeatures;

// Recommendations
typedef struct {
    SpotifyTrack *tracks;
    int count;
} SpotifyRecommendations;

// Recently Played
typedef struct {
    SpotifyTrack track;
    char played_at[32];
    char context_type[32];
} SpotifyPlayHistory;

typedef struct {
    SpotifyPlayHistory *history;
    int count;
} SpotifyRecentlyPlayed;

// Search for tracks, artists, artists top tracks
SpotifyTrackList* spotify_search_tracks(SpotifyToken *token, const char *query, int limit);
SpotifyArtistList* spotify_search_artists(SpotifyToken *token, const char *query, int limit);
SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market);

// Add tracks to user's library
bool spotify_save_tracks(SpotifyToken *token, const char **track_ids, int count);

// Get user's saved tracks, artists top tracks, artist's albums, user's albums, player's state
SpotifyTrackList* spotify_get_saved_tracks(SpotifyToken *token, int limit, int offset);
SpotifyAlbumList* spotify_get_artist_albums(SpotifyToken *token, const char *artist_id);
SpotifyPlaylistList* spotify_get_user_playlists(SpotifyToken *token, int limit, int offset);
SpotifyPlayerState* spotify_get_player_state(SpotifyToken *token);

// Playlist management
char* spotify_get_current_user_id(SpotifyToken *token);
SpotifyPlaylistFull* spotify_create_playlist(SpotifyToken *token, const char *name, const char *description, bool is_public, bool is_collaborative);
SpotifyPlaylistFull* spotify_get_playlist(SpotifyToken *token, const char *playlist_id, bool fetch_tracks, int track_limit);
bool spotify_update_playlist(SpotifyToken *token, const char *playlist_id, SpotifyPlaylistUpdate *updates);
SpotifyPlaylistResult* spotify_add_tracks_to_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, int position);
SpotifyPlaylistResult* spotify_remove_tracks_from_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, const char *snapshot_id);
bool spotify_unfollow_playlist(SpotifyToken *token, const char *playlist_id);

SpotifyAlbumDetailed* spotify_get_album(SpotifyToken *token, const char *album_id);
SpotifyAlbumList* spotify_search_albums(SpotifyToken *token, const char *query, int limit);
SpotifyUserProfile* spotify_get_current_user_profile(SpotifyToken *token);
SpotifyUserProfile* spotify_get_user_profile(SpotifyToken *token, const char *user_id);
SpotifyAudioFeatures* spotify_get_audio_features(SpotifyToken *token, const char *track_id);
SpotifyAudioFeatures* spotify_get_audio_features_batch(SpotifyToken *token, const char **track_ids, int count);
SpotifyRecommendations* spotify_get_recommendations(SpotifyToken *token, const char **seed_tracks, const char **seed_artists, const char **seed_genres, int seed_count, int limit);
SpotifyRecentlyPlayed* spotify_get_recently_played(SpotifyToken *token, int limit);


// Free track list, artist list, album list, player state, user's playlist memory
void spotify_free_album_detailed(SpotifyAlbumDetailed *album);
void spotify_free_album_list(SpotifyAlbumList *list);
void spotify_free_artist_list(SpotifyArtistList *list);
void spotify_free_audio_features(SpotifyAudioFeatures *features);
void spotify_free_audio_features_batch(SpotifyAudioFeatures *features, int count);
void spotify_free_player_state(SpotifyPlayerState *state);
void spotify_free_playlist_full(SpotifyPlaylistFull *playlist);
void spotify_free_playlist_list(SpotifyPlaylistList *list);
void spotify_free_playlist_result(SpotifyPlaylistResult *result);
void spotify_free_recently_played(SpotifyRecentlyPlayed *history);
void spotify_free_recommendations(SpotifyRecommendations *recommendations);
void spotify_free_track_list(SpotifyTrackList *list);
void spotify_free_user_profile(SpotifyUserProfile *profile);

// Helper to print track, artist, album, user's playlist, player state info
void spotify_print_album(SpotifyAlbum *album, int index);
void spotify_print_album_detailed(SpotifyAlbumDetailed *album);
void spotify_print_artist(SpotifyArtist *artist, int index);
void spotify_print_audio_features(SpotifyAudioFeatures *features);
void spotify_print_device(SpotifyDevice *device, int index);
void spotify_print_player_state(SpotifyPlayerState *state);
void spotify_print_playlist(SpotifyPlaylist *playlist, int index);
void spotify_print_playlist_full(SpotifyPlaylistFull *playlist);
void spotify_print_recently_played(SpotifyRecentlyPlayed *history);
void spotify_print_recommendations(SpotifyRecommendations *recommendations);
void spotify_print_track(SpotifyTrack *track, int index);
void spotify_print_user_profile(SpotifyUserProfile *profile);

// Control playback (pause/resume/start/toggle)
bool spotify_pause_playback(SpotifyToken *token, const char *device_id);
bool spotify_resume_playback(SpotifyToken *token, const char *device_id);
bool spotify_start_playback(SpotifyToken *token, const char *device_id, const char *context_uri, const char **uris, int uri_count);
bool spotify_toggle_playback(SpotifyToken *token);

// Control playback (skip to next/skip to previous/toggle shuffle)
bool spotify_skip_next_playback(SpotifyToken *token, const char *device_id);
bool spotify_skip_previous_playback(SpotifyToken *token, const char *device_id);
bool spotify_toggle_playback_shuffle(SpotifyToken *token, const char *device_id, bool state_shuffle);

/**
 * Transfer playback to a different device
 * 
 * @param token - Valid Spotify token
 * @param device_id - Target device ID to transfer playback to
 * @param play - If true, playback starts on new device; if false, keep current state
 * @return true if successful, false otherwise
 */
bool spotify_transfer_playback(SpotifyToken *token, const char *device_id, bool play);

/**
 * Get list of available devices
 * 
 * @param token - Valid Spotify token
 * @param device_count - Output parameter for number of devices found
 * @return Array of SpotifyDevice or NULL on error (must be freed by caller)
 */
SpotifyDevice* spotify_get_available_devices(SpotifyToken *token, int *device_count);

bool spotify_set_playback_volume(SpotifyToken *token, const char *device_id, int volume);

/**
 * Get the current user's queue
 * 
 * @param token - Valid Spotify token
 * @return SpotifyQueue struct containing currently playing track and queue, or NULL on error
 */
SpotifyQueue* spotify_get_queue(SpotifyToken *token);

/**
 * Add an item to the end of the user's current playback queue
 * 
 * @param token - Valid Spotify token
 * @param uri - Spotify URI of the track/episode to add (e.g., "spotify:track:4iV5W9uYEdYUVa79Axb7Rh")
 * @param device_id - Optional: specific device ID (NULL for current active device)
 * @return true if successful, false otherwise
 */
bool spotify_add_to_queue(SpotifyToken *token, const char *uri, const char *device_id);

/**
 * Free queue memory
 * 
 * @param queue - Queue to free
 */
void spotify_free_queue(SpotifyQueue *queue);

/**
 * Print queue information
 * 
 * @param queue - Queue to print
 */
void spotify_print_queue(SpotifyQueue *queue);

#endif
