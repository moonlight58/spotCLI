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

// Search for tracks
SpotifyTrackList* spotify_search_tracks(SpotifyToken *token, const char *query, int limit);

// Search for artists
SpotifyArtistList* spotify_search_artists(SpotifyToken *token, const char *query, int limit);

// Get artist's top tracks
SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market);

// Add tracks to user's library
bool spotify_save_tracks(SpotifyToken *token, const char **track_ids, int count);

// Get user's saved tracks
SpotifyTrackList* spotify_get_saved_tracks(SpotifyToken *token, int limit, int offset);

// Get artist's top tracks
SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market);

// Get artist's albums
SpotifyAlbumList* spotify_get_artist_albums(SpotifyToken *token, const char *artist_id);

// Get user's playlists
SpotifyPlaylistList* spotify_get_user_playlists(SpotifyToken *token, int limit, int offset);

// Get player's state
SpotifyPlayerState* spotify_get_player_state(SpotifyToken *token);

// Free track list memory
void spotify_free_track_list(SpotifyTrackList *list);

// Free artist list memory
void spotify_free_artist_list(SpotifyArtistList *list);

// Free album list memory
void spotify_free_album_list(SpotifyAlbumList *list);

// Free player state memory
void spotify_free_player_state(SpotifyPlayerState *state);

// Free user's playlist memory
void spotify_free_playlist_list(SpotifyPlaylistList *list);

// Helper to print track info
void spotify_print_track(SpotifyTrack *track, int index);

// Helper to print artist info
void spotify_print_artist(SpotifyArtist *artist, int index);

// Helper to print album info
void spotify_print_album(SpotifyAlbum *album, int index);

// Helper to print user's playlist info
void spotify_print_playlist(SpotifyPlaylist *playlist, int index);

// Helper to print player's state
void spotify_print_player_state(SpotifyPlayerState *state);

// Control playback
bool spotify_pause_playback(SpotifyToken *token, const char *device_id);
bool spotify_resume_playback(SpotifyToken *token, const char *device_id);
bool spotify_start_playback(SpotifyToken *token, const char *device_id, const char *context_uri, const char **uris, int uri_count);
bool spotify_toggle_playback(SpotifyToken *token);

bool spotify_skip_next_playback(SpotifyToken *token, const char *device_id);
bool spotify_skip_previous_playback(SpotifyToken *token, const char *device_id);



#endif
