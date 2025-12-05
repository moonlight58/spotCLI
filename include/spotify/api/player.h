#ifndef SPOTIFY_PLAYER_H
#define SPOTIFY_PLAYER_H

#include "spotify/spotify_internal.h"

SpotifyPlayerState* spotify_get_player_state(SpotifyToken *token);
bool spotify_pause_playback(SpotifyToken *token, const char *device_id);
bool spotify_resume_playback(SpotifyToken *token, const char *device_id);
bool spotify_start_playback(SpotifyToken *token, const char *device_id, const char *context_uri, const char **uris, int uri_count);
bool spotify_skip_next_playback(SpotifyToken *token, const char *device_id);
bool spotify_skip_previous_playback(SpotifyToken *token, const char *device_id);
bool spotify_toggle_playback(SpotifyToken *token);
bool spotify_toggle_playback_shuffle(SpotifyToken *token, const char *device_id, bool state_shuffle);
bool spotify_toggle_playback_repeat(SpotifyToken *token, const char *device_id);
bool spotify_transfer_playback(SpotifyToken *token, const char *device_id, bool play);
bool spotify_set_playback_volume(SpotifyToken *token, const char *device_id, int volume);
SpotifyDevice* spotify_get_available_devices(SpotifyToken *token, int *device_count);
SpotifyQueue* spotify_get_queue(SpotifyToken *token);
bool spotify_add_to_queue(SpotifyToken *token, const char *uri, const char *device_id);

#endif
