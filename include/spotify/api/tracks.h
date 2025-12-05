#ifndef SPOTIFY_TRACKS_H
#define SPOTIFY_TRACKS_H

#include "spotify/spotify_internal.h"

// ===== TRACK RETRIEVAL FUNCTIONS =====

/**
 * Get detailed information about a single track
 * 
 * @param token - Valid Spotify token
 * @param track_id - Spotify track ID
 * @param market - Optional: ISO 3166-1 alpha-2 country code (NULL for no market filter)
 * @return SpotifyTrack or NULL on error (must be freed with spotify_free_track)
 * 
 * Example:
 *   SpotifyTrack *track = spotify_get_track(token, "3n3Ppam7vgaVa1iaRUc9Lp", "US");
 *   if (track) {
 *       spotify_print_track(track, 1);
 *       spotify_free_track(track);
 *   }
 */
SpotifyTrack* spotify_get_track(SpotifyToken *token, const char *track_id, const char *market);

/**
 * Get detailed information about multiple tracks (max 50)
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs
 * @param count - Number of track IDs (max 50)
 * @param market - Optional: ISO 3166-1 alpha-2 country code (NULL for no market filter)
 * @return SpotifyTrackList or NULL on error
 * 
 * Example:
 *   const char *ids[] = {"track_id_1", "track_id_2", "track_id_3"};
 *   SpotifyTrackList *tracks = spotify_get_tracks(token, ids, 3, "US");
 *   if (tracks) {
 *       for (int i = 0; i < tracks->count; i++) {
 *           spotify_print_track(&tracks->tracks[i], i + 1);
 *       }
 *       spotify_free_track_list(tracks);
 *   }
 */
SpotifyTrackList* spotify_get_tracks(SpotifyToken *token, const char **track_ids, int count, const char *market);

/**
 * Get user's saved tracks (from library)
 * 
 * @param token - Valid Spotify token
 * @param limit - Number of tracks to return (1-50, default 20)
 * @param offset - Index of first track to return (default 0)
 * @return SpotifyTrackList or NULL on error
 * 
 * Note: For pagination, use offset to get next page of results
 */
SpotifyTrackList* spotify_get_saved_tracks(SpotifyToken *token, int limit, int offset);

// ===== LIBRARY MANAGEMENT FUNCTIONS =====

/**
 * Save tracks to current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to save
 * @param count - Number of track IDs (max 50)
 * @return true if successful, false otherwise
 * 
 * Example:
 *   const char *ids[] = {"track_id_1", "track_id_2"};
 *   if (spotify_save_tracks(token, ids, 2)) {
 *       printf("Tracks saved successfully!\n");
 *   }
 */
bool spotify_save_tracks(SpotifyToken *token, const char **track_ids, int count);

/**
 * Remove tracks from current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to remove
 * @param count - Number of track IDs (max 50)
 * @return true if successful, false otherwise
 * 
 * Example:
 *   const char *ids[] = {"track_id_1"};
 *   if (spotify_remove_tracks(token, ids, 1)) {
 *       printf("Track removed from library!\n");
 *   }
 */
bool spotify_remove_tracks(SpotifyToken *token, const char **track_ids, int count);

/**
 * Check if one or more tracks are saved in current user's 'Your Music' library
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of Spotify track IDs to check
 * @param count - Number of track IDs (max 50)
 * @param result_count - Output parameter for number of results returned
 * @return Array of booleans (true = saved, false = not saved), or NULL on error
 *         Caller must free the returned array
 * 
 * Example:
 *   const char *ids[] = {"track_id_1", "track_id_2"};
 *   int result_count;
 *   bool *results = spotify_check_saved_tracks(token, ids, 2, &result_count);
 *   if (results) {
 *       for (int i = 0; i < result_count; i++) {
 *           printf("Track %d is %s\n", i + 1, results[i] ? "saved" : "not saved");
 *       }
 *       free(results);
 *   }
 */
bool* spotify_check_saved_tracks(SpotifyToken *token, const char **track_ids, int count, int *result_count);

/**
 * Check if a single track is saved - convenience wrapper
 * 
 * @param token - Valid Spotify token
 * @param track_id - Spotify track ID to check
 * @return true if saved, false otherwise
 * 
 * Example:
 *   if (spotify_is_track_saved(token, "3n3Ppam7vgaVa1iaRUc9Lp")) {
 *       printf("This track is in your library!\n");
 *   }
 */
bool spotify_is_track_saved(SpotifyToken *token, const char *track_id);

// ===== FREE FUNCTIONS =====

/**
 * Free a single track struct
 */
void spotify_free_track(SpotifyTrack *track);

#endif
