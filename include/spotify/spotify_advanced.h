#ifndef SPOTIFY_ADVANCED_H
#define SPOTIFY_ADVANCED_H

#include "spotify/spotify_internal.h"
#include <stdbool.h>


// ===== ALBUM FUNCTIONS =====

/**
 * Get detailed album information including all tracks
 * 
 * @param token - Valid Spotify token
 * @param album_id - Spotify album ID
 * @return SpotifyAlbumDetailed struct with tracks and album info, or NULL on error
 */
SpotifyAlbumDetailed* spotify_get_album(SpotifyToken *token, const char *album_id);

/**
 * Search for albums
 * 
 * @param token - Valid Spotify token
 * @param query - Search query
 * @param limit - Max number of results (1-50)
 * @return SpotifyAlbumList or NULL on error
 */
SpotifyAlbumList* spotify_search_albums(SpotifyToken *token, const char *query, int limit);

// ===== USER PROFILE FUNCTIONS =====

/**
 * Get current user's profile information
 * 
 * @param token - Valid Spotify token
 * @return SpotifyUserProfile or NULL on error
 */
SpotifyUserProfile* spotify_get_current_user_profile(SpotifyToken *token);

/**
 * Get another user's profile information
 * 
 * @param token - Valid Spotify token
 * @param user_id - Target user ID
 * @return SpotifyUserProfile or NULL on error
 */
SpotifyUserProfile* spotify_get_user_profile(SpotifyToken *token, const char *user_id);

// ===== AUDIO FEATURES FUNCTIONS =====

/**
 * Get audio features for a single track
 * 
 * @param token - Valid Spotify token
 * @param track_id - Spotify track ID
 * @return SpotifyAudioFeatures or NULL on error
 */
SpotifyAudioFeatures* spotify_get_audio_features(SpotifyToken *token, const char *track_id);

/**
 * Get audio features for multiple tracks (max 100)
 * 
 * @param token - Valid Spotify token
 * @param track_ids - Array of track IDs
 * @param count - Number of track IDs (max 100)
 * @return Array of SpotifyAudioFeatures or NULL on error (must be freed)
 */
SpotifyAudioFeatures* spotify_get_audio_features_batch(SpotifyToken *token, const char **track_ids, int count);

// ===== RECOMMENDATIONS FUNCTIONS =====

/**
 * Get recommendations based on seed tracks, artists, or genres
 * 
 * @param token - Valid Spotify token
 * @param seed_tracks - Array of track IDs (max 5)
 * @param seed_artists - Array of artist IDs (max 5)
 * @param seed_genres - Array of genre strings (max 5)
 * @param seed_count - Total number of seeds (must be 1-5)
 * @param limit - Number of recommendations to return (1-100)
 * @return SpotifyRecommendations or NULL on error
 */
SpotifyRecommendations* spotify_get_recommendations(SpotifyToken *token, const char **seed_tracks, const char **seed_artists, const char **seed_genres, int seed_count, int limit);

// ===== RECENTLY PLAYED FUNCTIONS =====

/**
 * Get user's recently played tracks
 * 
 * @param token - Valid Spotify token
 * @param limit - Number of recent tracks to return (1-50)
 * @return SpotifyRecentlyPlayed or NULL on error
 */
SpotifyRecentlyPlayed* spotify_get_recently_played(SpotifyToken *token, int limit);

// ===== FREE FUNCTIONS =====

void spotify_free_album_detailed(SpotifyAlbumDetailed *album);
void spotify_free_user_profile(SpotifyUserProfile *profile);
void spotify_free_audio_features(SpotifyAudioFeatures *features);
void spotify_free_audio_features_batch(SpotifyAudioFeatures *features, int count);
void spotify_free_recommendations(SpotifyRecommendations *recommendations);
void spotify_free_recently_played(SpotifyRecentlyPlayed *history);

// ===== PRINT FUNCTIONS =====

void spotify_print_album_detailed(SpotifyAlbumDetailed *album);
void spotify_print_user_profile(SpotifyUserProfile *profile);
void spotify_print_audio_features(SpotifyAudioFeatures *features);
void spotify_print_recommendations(SpotifyRecommendations *recommendations);
void spotify_print_recently_played(SpotifyRecentlyPlayed *history);

#endif
