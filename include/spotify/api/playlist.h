#ifndef SPOTIFY_PLAYLIST_H
#define SPOTIFY_PLAYLIST_H

#include "spotify/spotify_internal.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

SpotifyPlaylistFull* spotify_create_playlist(SpotifyToken *token, const char *name, const char *description, bool is_public, bool is_collaborative);
SpotifyPlaylistFull* spotify_get_playlist(SpotifyToken *token, const char *playlist_id, bool fetch_tracks, int track_limit);
SpotifyPlaylistResult* spotify_add_tracks_to_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, int position);
SpotifyPlaylistResult* spotify_remove_tracks_from_playlist(SpotifyToken *token, const char *playlist_id, const char **track_uris, int count, const char *snapshot_id);
bool spotify_unfollow_playlist(SpotifyToken *token, const char *playlist_id);
bool spotify_update_playlist(SpotifyToken *token, const char *playlist_id, SpotifyPlaylistUpdate *updates);
/**
 * Get the user's currently playing track (more lightweight than full player state)
 * 
 * @param token - Valid Spotify token
 * @return SpotifyPlayerState with only currently playing info, or NULL on error
 */
SpotifyPlayerState* spotify_get_currently_playing(SpotifyToken *token);

/**
 * Seek to a specific position in the currently playing track
 * 
 * @param token - Valid Spotify token
 * @param position_ms - Position in milliseconds to seek to
 * @param device_id - Optional: specific device ID (NULL for current active device)
 * @return true if successful, false otherwise
 */
bool spotify_seek_to_position(SpotifyToken *token, int position_ms, const char *device_id);

/**
 * Get tracks from a specific playlist with pagination support
 * More flexible than spotify_get_playlist() for large playlists
 * 
 * @param token - Valid Spotify token
 * @param playlist_id - Spotify playlist ID
 * @param limit - Number of tracks to return (1-100, default 100)
 * @param offset - Index of first track to return (default 0)
 * @return SpotifyTrackList or NULL on error
 */
SpotifyTrackList* spotify_get_playlist_tracks(SpotifyToken *token, const char *playlist_id, int limit, int offset);

/**
 * Reorder or replace tracks in a playlist
 * Can be used to move tracks around or completely replace playlist contents
 * 
 * @param token - Valid Spotify token
 * @param playlist_id - Spotify playlist ID
 * @param range_start - Position of first track to move (0-indexed)
 * @param insert_before - Position where tracks should be inserted (0-indexed)
 * @param range_length - Number of tracks to move (default 1 if 0)
 * @param uris - Optional: Array of URIs to replace entire playlist (NULL to just reorder)
 * @param uri_count - Number of URIs (only used if uris is not NULL)
 * @param snapshot_id - Optional: playlist snapshot ID for conflict detection
 * @return SpotifyPlaylistResult with new snapshot_id, or NULL on error
 */
SpotifyPlaylistResult* spotify_reorder_playlist_tracks(
    SpotifyToken *token,
    const char *playlist_id,
    int range_start,
    int insert_before,
    int range_length,
    const char **uris,
    int uri_count,
    const char *snapshot_id
);

#endif

