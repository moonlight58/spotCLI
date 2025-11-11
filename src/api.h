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

// Free track list memory
void spotify_free_track_list(SpotifyTrackList *list);

// Free artist list memory
void spotify_free_artist_list(SpotifyArtistList *list);

// Helper to print track info
void spotify_print_track(SpotifyTrack *track, int index);

// Helper to print artist info
void spotify_print_artist(SpotifyArtist *artist, int index);

#endif
