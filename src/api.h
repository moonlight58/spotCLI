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

typedef struct {
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

typedef struct {
    SpotifyArtist *artists;
    int count;
    int total;
} SpotifyArtistList;

typedef struct {
    char id[64];
    char name[256];
    char artist[256];
} SpotifyAlbum;

typedef struct {
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

typedef struct {
    SpotifyPlaylist* playlists;
    int count;
    int total;
} SpotifyPlaylistList;

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

// Free track list memory
void spotify_free_track_list(SpotifyTrackList *list);

// Free artist list memory
void spotify_free_artist_list(SpotifyArtistList *list);

// Free album list memory
void spotify_free_album_list(SpotifyAlbumList *list);

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

#endif
