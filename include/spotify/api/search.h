#ifndef SPOTIFY_SEARCH_H
#define SPOTIFY_SEARCH_H

#include "spotify/spotify_internal.h"

SpotifyAlbumList* spotify_get_artist_albums(SpotifyToken *token, const char *artist_id);
SpotifyArtist* spotify_get_artist(SpotifyToken *token, const char *artist_id);
SpotifyArtistList* spotify_get_artists(SpotifyToken *token, const char **artist_ids, int count);
SpotifyArtistList* spotify_search_artists(SpotifyToken *token, const char *query, int limit);
SpotifyTrackList* spotify_get_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *market);
SpotifyTrackList* spotify_search_tracks(SpotifyToken *token, const char *query, int limit);

#endif

