#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void spotify_free_track_list(SpotifyTrackList *list) {
    if (!list) return;
    free(list->tracks);
    free(list);
}

void spotify_free_artist_list(SpotifyArtistList *list) {
    if (!list) return;
    free(list->artists);
    free(list);
}

void spotify_free_album_list(SpotifyAlbumList *list) {
    if (!list) return;
    free(list->albums);
    free(list);
}

void spotify_free_playlist_list(SpotifyPlaylistList *list) {
    if (!list) return;
    free(list->playlists);
    free(list);
}

void spotify_free_player_state(SpotifyPlayerState *state) {
    if (!state) return;
    free(state);
}

void spotify_free_queue(SpotifyQueue *queue) {
    if (!queue) return;
    if (queue->queue) {
        free(queue->queue);
    }
    free(queue);
}

void spotify_print_track(SpotifyTrack *track, int index) {
    printf("%d. %s\n", index, track->name);
    printf("   Artist: %s\n", track->artist);
    printf("   Album: %s\n", track->album);
    printf("   Duration: %d:%02d\n", 
           track->duration_ms / 60000, 
           (track->duration_ms / 1000) % 60);
    printf("   ID: %s\n", track->id);
}

void spotify_print_artist(SpotifyArtist *artist, int index) {
    printf("%d. %s\n", index, artist->name);
    printf("   Followers: %d\n", artist->followers);
    printf("   Popularity: %d/100\n", artist->popularity);
    if (strlen(artist->genres) > 0) {
        printf("   Genres: %s\n", artist->genres);
    }
    printf("   ID: %s\n", artist->id);
}

void spotify_print_album(SpotifyAlbum *album, int index) {
    printf("%d. %s\n", index, album->name);
    printf("   Artist: %s\n", album->artist);
    printf("   ID: %s\n", album->id);
}

void spotify_print_playlist(SpotifyPlaylist *playlist, int index) {
    printf("%d. %s\n", index, playlist->name);
    printf("   Tracks: %d\n", playlist->count_tracks);
    printf("   Public: %s\n", playlist->is_public ? "Yes" : "No");
    printf("   ID: %s\n", playlist->id);
}

void spotify_print_player_state(SpotifyPlayerState *state) {
    if (!state) {
        printf("No playback state available\n");
        return;
    }
    
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                      SPOTIFY PLAYER STATE                      ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    
    // Track info
    printf("║ Track: %s\n", state->track_name);
    printf("║ Artist: %s\n", state->artist_name);
    printf("║ Album: %s\n", state->album_name);
    
    // Progress
    int progress_sec = state->progress_ms / 1000;
    int duration_sec = state->duration_ms / 1000;
    printf("║ Progress: %d:%02d / %d:%02d\n", 
           progress_sec / 60, progress_sec % 60,
           duration_sec / 60, duration_sec % 60);
    
    // Progress bar
    int bar_width = 50;
    int filled = (state->duration_ms > 0) ? 
                 (state->progress_ms * bar_width / state->duration_ms) : 0;
    printf("║ [");
    for (int i = 0; i < bar_width; i++) {
        printf(i < filled ? "#" : ".");
    }
    printf("]\n");
    
    // Playback state
    printf("║ Status: %s\n", state->is_playing ? "▶ Playing" : "⏸ Paused");
    printf("║ Shuffle: %s\n", state->shuffle_state ? "On" : "Off");
    printf("║ Repeat: %s\n", state->repeat_state);
    
    // Context
    if (strlen(state->context_type) > 0) {
        printf("║ Context: %s\n", state->context_type);
    }
    
    // Device info
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ Device: %s (%s)\n", state->device.device_name, state->device.device_type);
    printf("║ Volume: %d%%\n", state->device.volume_percent);
    printf("║ Active: %s\n", state->device.is_active ? "Yes" : "No");
    
    printf("╚════════════════════════════════════════════════════════════════╝\n");
}

void spotify_print_queue(SpotifyQueue *queue) {
    if (!queue) {
        printf("No queue information available\n");
        return;
    }
    
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                      PLAYBACK QUEUE                            ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    
    // Currently playing
    if (strlen(queue->currently_playing.name) > 0) {
        printf("║ 🎵 NOW PLAYING:\n");
        printf("║    %s\n", queue->currently_playing.name);
        printf("║    by %s\n", queue->currently_playing.artist);
        printf("║    from %s\n", queue->currently_playing.album);
        printf("╠════════════════════════════════════════════════════════════════╣\n");
    }
    
    // Queue
    if (queue->queue_count > 0) {
        printf("║ 📋 NEXT IN QUEUE (%d track%s):\n", 
               queue->queue_count, 
               queue->queue_count == 1 ? "" : "s");
        printf("║\n");
        
        for (int i = 0; i < queue->queue_count; i++) {
            printf("║ %d. %s\n", i + 1, queue->queue[i].name);
            printf("║    by %s", queue->queue[i].artist);
            
            // Duration
            int duration_min = queue->queue[i].duration_ms / 60000;
            int duration_sec = (queue->queue[i].duration_ms / 1000) % 60;
            printf(" (%d:%02d)\n", duration_min, duration_sec);
            
            if (i < queue->queue_count - 1) {
                printf("║\n");
            }
        }
    } else {
        printf("║ 📋 Queue is empty\n");
    }
    
    printf("╚════════════════════════════════════════════════════════════════╝\n");
}

void spotify_print_device(SpotifyDevice *device, int index) {
    printf("%d. %s\n", index, device->device_name);
    printf("   Type: %s\n", device->device_type);
    printf("   Volume: %d%%\n", device->volume_percent);
    printf("   Active: %s\n", device->is_active ? "✓ Yes" : "No");
    printf("   ID: %s\n", device->device_id);
}

void spotify_free_playlist_full(SpotifyPlaylistFull *playlist) {
    if (!playlist) return;
    if (playlist->tracks) {
        free(playlist->tracks);
    }
    free(playlist);
}

void spotify_free_playlist_result(SpotifyPlaylistResult *result) {
    if (!result) return;
    free(result);
}

void spotify_print_playlist_full(SpotifyPlaylistFull *playlist) {
    if (!playlist) {
        printf("No playlist data available\n");
        return;
    }
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                      PLAYLIST DETAILS                          ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    
    printf("║ Name: %s\n", playlist->name);
    
    if (strlen(playlist->description) > 0) {
        printf("║ Description: %s\n", playlist->description);
    }
    
    printf("║ Owner: %s\n", playlist->owner_name);
    printf("║ Public: %s\n", playlist->is_public ? "Yes" : "No");
    printf("║ Collaborative: %s\n", playlist->is_collaborative ? "Yes" : "No");
    printf("║ Total tracks: %d\n", playlist->tracks_count);
    printf("║ ID: %s\n", playlist->id);
    
    if (playlist->tracks && playlist->tracks_count > 0) {
        printf("╠════════════════════════════════════════════════════════════════╣\n");
        printf("║ TRACKS:\n");
        printf("║\n");
        
        for (int i = 0; i < playlist->tracks_count; i++) {
            printf("║ %d. %s\n", i + 1, playlist->tracks[i].name);
            printf("║    by %s", playlist->tracks[i].artist);
            
            // Duration
            int duration_min = playlist->tracks[i].duration_ms / 60000;
            int duration_sec = (playlist->tracks[i].duration_ms / 1000) % 60;
            printf(" (%d:%02d)\n", duration_min, duration_sec);
            
            if (i < playlist->tracks_count - 1) {
                printf("║\n");
            }
        }
    }
    
    printf("╚════════════════════════════════════════════════════════════════╝\n");
}
