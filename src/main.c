#include "auth.h"
#include "api.h"
#include "dotenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

// ===== DÉCLARATIONS DE FONCTIONS =====
void add_track_to_playlist_interactive(SpotifyToken *token);
void create_playlist_interactive(SpotifyToken *token);
void manage_playlist_interactive(SpotifyToken *token);
void remove_track_from_playlist_interactive(SpotifyToken *token);
void search_artist_and_view_albums(SpotifyToken *token, const char *query);
void search_artist_and_view_top_tracks(SpotifyToken *token, const char *query);
void unfollow_playlist_interactive(SpotifyToken *token);
void view_and_transfer_devices(SpotifyToken *token);
void view_artist_albums(SpotifyToken *token, const char *artist_id, const char *artist_name);
void view_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *artist_name);
void view_users_playlists(SpotifyToken *token, int limit, int offset);

void add_artist_track_to_queue(SpotifyToken *token) {
    printf("\n=== Add Artist Track to Queue ===\n");
    printf("Enter artist name: ");
    
    char query[256];
    if (!fgets(query, sizeof(query), stdin)) {
        printf("Invalid input.\n");
        return;
    }
    query[strcspn(query, "\n")] = '\0';
    
    // Search for artists
    printf("\nSearching for artists matching '%s'...\n", query);
    SpotifyArtistList *results = spotify_search_artists(token, query, 10);
    
    if (!results || results->count == 0) {
        printf("No artists found.\n");
        if (results) spotify_free_artist_list(results);
        return;
    }
    
    printf("\nFound %d artist(s):\n\n", results->count);
    
    for (int i = 0; i < results->count; i++) {
        spotify_print_artist(&results->artists[i], i + 1);
        printf("\n");
    }
    
    printf("Enter artist number (or 0 to cancel): ");
    int artist_choice;
    if (scanf("%d", &artist_choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_artist_list(results);
        return;
    }
    getchar(); // consume newline
    
    if (artist_choice <= 0 || artist_choice > results->count) {
        spotify_free_artist_list(results);
        return;
    }
    
    const char *artist_id = results->artists[artist_choice - 1].id;
    const char *artist_name = results->artists[artist_choice - 1].name;
    
    spotify_free_artist_list(results);
    
    // Get artist's top tracks
    printf("\nFetching top tracks for %s...\n", artist_name);
    SpotifyTrackList *tracks = spotify_get_artist_top_tracks(token, artist_id, "US");
    
    if (!tracks || tracks->count == 0) {
        printf("No tracks found.\n");
        if (tracks) spotify_free_track_list(tracks);
        return;
    }
    
    printf("\n🎵 Top tracks by %s:\n\n", artist_name);
    
    for (int i = 0; i < tracks->count; i++) {
        spotify_print_track(&tracks->tracks[i], i + 1);
        printf("\n");
    }
    
    printf("Enter track number to add to queue (or 0 to cancel): ");
    int track_choice;
    if (scanf("%d", &track_choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_track_list(tracks);
        return;
    }
    getchar(); // consume newline
    
    if (track_choice > 0 && track_choice <= tracks->count) {
        const char *track_uri = tracks->tracks[track_choice - 1].uri;
        const char *track_name = tracks->tracks[track_choice - 1].name;
        
        printf("Adding '%s' to queue...\n", track_name);
        if (spotify_add_to_queue(token, track_uri, NULL)) {
            printf("✅ Track added to queue successfully!\n");
        } else {
            printf("❌ Failed to add track to queue.\n");
        }
    }
    
    spotify_free_track_list(tracks);
}


void add_track_to_playlist_interactive(SpotifyToken *token) {
    printf("\n=== Add Track to Playlist ===\n");
    
    // First search for a track
    char query[256];
    printf("Search for track: ");
    if (!fgets(query, sizeof(query), stdin)) {
        printf("Invalid input.\n");
        return;
    }
    query[strcspn(query, "\n")] = '\0';
    
    SpotifyTrackList *tracks = spotify_search_tracks(token, query, 10);
    if (!tracks || tracks->count == 0) {
        printf("No tracks found.\n");
        if (tracks) spotify_free_track_list(tracks);
        return;
    }
    
    printf("\nFound tracks:\n\n");
    for (int i = 0; i < tracks->count; i++) {
        spotify_print_track(&tracks->tracks[i], i + 1);
        printf("\n");
    }
    
    printf("Select track (or 0 to cancel): ");
    int track_choice;
    if (scanf("%d", &track_choice) != 1 || track_choice <= 0 || track_choice > tracks->count) {
        spotify_free_track_list(tracks);
        return;
    }
    getchar();
    
    const char *track_uri = tracks->tracks[track_choice - 1].uri;
    const char *track_name = tracks->tracks[track_choice - 1].name;
    
    // Now select playlist
    SpotifyPlaylistList *playlists = spotify_get_user_playlists(token, 20, 0);
    if (!playlists || playlists->count == 0) {
        printf("No playlists found.\n");
        spotify_free_track_list(tracks);
        if (playlists) spotify_free_playlist_list(playlists);
        return;
    }
    
    printf("\nYour playlists:\n\n");
    for (int i = 0; i < playlists->count; i++) {
        spotify_print_playlist(&playlists->playlists[i], i + 1);
        printf("\n");
    }
    
    printf("Select playlist (or 0 to cancel): ");
    int playlist_choice;
    if (scanf("%d", &playlist_choice) != 1 || playlist_choice <= 0 || 
        playlist_choice > playlists->count) {
        spotify_free_track_list(tracks);
        spotify_free_playlist_list(playlists);
        return;
    }
    getchar();
    
    const char *playlist_id = playlists->playlists[playlist_choice - 1].id;
    const char *playlist_name = playlists->playlists[playlist_choice - 1].name;
    
    printf("\nAdding '%s' to '%s'...\n", track_name, playlist_name);
    
    const char *uris[] = { track_uri };
    SpotifyPlaylistResult *result = spotify_add_tracks_to_playlist(token, playlist_id, uris, 1, -1);
    
    if (result && result->success) {
        printf("✅ Track added successfully!\n");
        spotify_free_playlist_result(result);
    } else {
        printf("❌ Failed to add track.\n");
    }
    
    spotify_free_track_list(tracks);
    spotify_free_playlist_list(playlists);
}


void add_track_to_queue_interactive(SpotifyToken *token) {
    printf("\n=== Add Track to Queue ===\n");
    printf("Enter search query: ");
    
    char query[256];
    if (!fgets(query, sizeof(query), stdin)) {
        printf("Invalid input.\n");
        return;
    }
    query[strcspn(query, "\n")] = '\0';
    
    // Search for tracks
    printf("\nSearching for '%s'...\n", query);
    SpotifyTrackList *results = spotify_search_tracks(token, query, 10);
    
    if (!results || results->count == 0) {
        printf("No tracks found.\n");
        if (results) spotify_free_track_list(results);
        return;
    }
    
    printf("\nFound %d results:\n\n", results->count);
    
    for (int i = 0; i < results->count; i++) {
        spotify_print_track(&results->tracks[i], i + 1);
        printf("\n");
    }
    
    printf("Enter track number to add to queue (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_track_list(results);
        return;
    }
    getchar(); // consume newline
    
    if (choice > 0 && choice <= results->count) {
        const char *track_uri = results->tracks[choice - 1].uri;
        const char *track_name = results->tracks[choice - 1].name;
        
        printf("Adding '%s' to queue...\n", track_name);
        if (spotify_add_to_queue(token, track_uri, NULL)) {
            printf("✅ Track added to queue successfully!\n");
        } else {
            printf("❌ Failed to add track to queue.\n");
        }
    }
    
    spotify_free_track_list(results);
}



void create_playlist_interactive(SpotifyToken *token) {
    printf("\n=== Create New Playlist ===\n");
    
    char name[256];
    char description[1024] = {0};
    char choice;
    bool is_public = true;
    bool is_collaborative = false;
    
    // Get playlist name
    printf("Enter playlist name: ");
    if (!fgets(name, sizeof(name), stdin)) {
        printf("Invalid input.\n");
        return;
    }
    name[strcspn(name, "\n")] = '\0';
    
    if (strlen(name) == 0) {
        printf("Playlist name cannot be empty.\n");
        return;
    }
    
    // Get description (optional)
    printf("Enter description (press Enter to skip): ");
    if (fgets(description, sizeof(description), stdin)) {
        description[strcspn(description, "\n")] = '\0';
    }
    
    // Public/Private
    printf("Make playlist public? (y/n) [y]: ");
    if (scanf(" %c", &choice) == 1) {
        is_public = (choice != 'n' && choice != 'N');
    }
    getchar(); // consume newline
    
    // Collaborative (only for private playlists)
    if (!is_public) {
        printf("Make playlist collaborative? (y/n) [n]: ");
        if (scanf(" %c", &choice) == 1) {
            is_collaborative = (choice == 'y' || choice == 'Y');
        }
        getchar();
    }
    
    printf("\nCreating playlist '%s'...\n", name);
    
    SpotifyPlaylistFull *playlist = spotify_create_playlist(
        token, name,
        strlen(description) > 0 ? description : NULL,
        is_public, is_collaborative
    );
    
    if (playlist) {
        printf("✅ Playlist created successfully!\n");
        spotify_print_playlist_full(playlist);
        spotify_free_playlist_full(playlist);
    } else {
        printf("❌ Failed to create playlist.\n");
    }
}


void interactive_mode(SpotifyToken *token) {
    while (1) {
        print_menu();

        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            while (getchar() != '\n'); // clear input buffer
            continue;
        }
        getchar(); // consume newline

        switch (choice) {
            case 1: // EXIT APP
                printf("\nGoodbye!\n");
                return;
            case 2:  // VIEW OPTIONS
                users_options();
                break;
            case 3:  // VIEW SAVED/LIKED TRACKS
                view_saved_tracks(token);
                break;
            case 4:  // SEARCH FOR AN ARTIST FROM QUERY
            {
                printf("\nEnter artist name: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_artists(token, query);
                }
                break;
            }
            case 5:  // SEARCH A SONG FROM QUERY
            {
                printf("\nEnter search query: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_and_save(token, query);
                }
                break;
            }
            case 6:  // VIEW ARTIST'S TOP TRACKS
            {
                printf("\nEnter artist name: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_artist_and_view_top_tracks(token, query);
                }
                break;
            }
            case 7:  // VIEW ARTIST'S ALBUMS
            {
                printf("\nEnter artist name: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_artist_and_view_albums(token, query);
                }
                break;
            }
            case 8:  // VIEW USER'S PLAYLISTS
                view_users_playlists(token, 20, 0);
                break;
            case 9:  // VIEW QUEUE
                view_queue(token);
                break;
            case 10:  // ADD TRACK TO QUEUE
                add_track_to_queue_interactive(token);
                break;
            case 11:  // ADD ARTIST TRACK TO QUEUE
                add_artist_track_to_queue(token);
                break;
            case 12:  // CREATE PLAYLIST
                create_playlist_interactive(token);
                break;
            case 13:  // MANAGE PLAYLIST
                manage_playlist_interactive(token);
                break;
            case 14:  // ADD TRACK TO PLAYLIST
                add_track_to_playlist_interactive(token);
                break;
            case 15:  // REMOVE TRACK FROM PLAYLIST
                remove_track_from_playlist_interactive(token);
                break;
            default:
                printf("Invalid option. Please try again.\n");
        }
    }
}


void manage_playlist_interactive(SpotifyToken *token) {
    printf("\n=== Manage Playlist ===\n");
    
    // First show user's playlists
    SpotifyPlaylistList *playlists = spotify_get_user_playlists(token, 20, 0);
    
    if (!playlists || playlists->count == 0) {
        printf("No playlists found.\n");
        if (playlists) spotify_free_playlist_list(playlists);
        return;
    }
    
    printf("\nYour playlists:\n\n");
    for (int i = 0; i < playlists->count; i++) {
        spotify_print_playlist(&playlists->playlists[i], i + 1);
        printf("\n");
    }
    
    printf("Enter playlist number to manage (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_playlist_list(playlists);
        return;
    }
    getchar();
    
    if (choice <= 0 || choice > playlists->count) {
        spotify_free_playlist_list(playlists);
        return;
    }
    
    const char *playlist_id = playlists->playlists[choice - 1].id;
    
    // Get full playlist details
    SpotifyPlaylistFull *playlist = spotify_get_playlist(token, playlist_id, true, 50);
    spotify_free_playlist_list(playlists);
    
    if (!playlist) {
        printf("Failed to get playlist details.\n");
        return;
    }
    
    spotify_print_playlist_full(playlist);
    
    printf("\nWhat would you like to do?\n");
    printf("1. Change name\n");
    printf("2. Change description\n");
    printf("3. Toggle public/private\n");
    printf("4. Toggle collaborative\n");
    printf("5. Unfollow playlist\n");
    printf("6. Cancel\n");
    printf("Choice: ");
    
    int action;
    if (scanf("%d", &action) != 1) {
        printf("Invalid input.\n");
        spotify_free_playlist_full(playlist);
        return;
    }
    getchar();
    
    SpotifyPlaylistUpdate updates = {0};
    bool do_update = false;
    
    switch (action) {
        case 1: {
            char new_name[256];
            printf("Enter new name: ");
            if (fgets(new_name, sizeof(new_name), stdin)) {
                new_name[strcspn(new_name, "\n")] = '\0';
                if (strlen(new_name) > 0) {
                    updates.name = new_name;
                    do_update = true;
                }
            }
            break;
        }
        case 2: {
            char new_desc[1024];
            printf("Enter new description: ");
            if (fgets(new_desc, sizeof(new_desc), stdin)) {
                new_desc[strcspn(new_desc, "\n")] = '\0';
                updates.description = new_desc;
                do_update = true;
            }
            break;
        }
        case 3: {
            bool new_public = !playlist->is_public;
            updates.is_public = &new_public;
            do_update = true;
            printf("Setting playlist to %s...\n", new_public ? "public" : "private");
            break;
        }
        case 4: {
            bool new_collab = !playlist->is_collaborative;
            updates.is_collaborative = &new_collab;
            do_update = true;
            printf("Setting collaborative to %s...\n", new_collab ? "on" : "off");
            break;
        }
        case 5: {
            
        }
        default:
            break;
    }
    
    if (do_update) {
        if (spotify_update_playlist(token, playlist->id, &updates)) {
            printf("✅ Playlist updated successfully!\n");
        } else {
            printf("❌ Failed to update playlist.\n");
        }
    }
    
    spotify_free_playlist_full(playlist);
}


void print_menu() {
    printf("\n=== spotCLI - Spotify CLI ===\n");
    printf("1. Exit\n");
    printf("2. Options\n");
    printf("3. View saved tracks\n");
    printf("4. Search for artists\n");
    printf("5. Search for tracks\n");
    printf("6. View artist's top tracks\n");
    printf("7. View artist's albums\n");
    printf("8. View your playlists\n");
    printf("9. View playback queue\n");
    printf("10. Add track to queue\n");
    printf("11. Add artist track to queue\n");
    printf("── Playlist Management ──\n");
    printf("12. Create new playlist\n");
    printf("13. Manage playlist (edit details)\n");
    printf("14. Add track to playlist\n");
    printf("15. Remove track from playlist\n");
    printf("Choose an option: ");
}


void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS] \"search query\"\n\n", prog_name);
    printf("Options:\n");
    printf("  -t, --track       Search for tracks (default)\n");
    printf("  -a, --artist      Search for artists\n");
    printf("  -A, --album       Search for albums\n");
    printf("  -p, --playlist    Search for playlists\n");
    printf("  -P, --player      Display the current player state (current music played, time of the music, etc...)\n");
    printf("  -u, --user        Search for users\n");
    printf("  -b, --audiobook   Search for audiobooks\n");
    printf("  -l, --list        List your saved tracks\n");
    printf("  -i, --interactive Interactive mode (menu)\n");
    printf("  -h, --help        Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -t \"PTSMR\"\n", prog_name);
    printf("  %s --artist \"tyler, the creator\"\n", prog_name);
    printf("  %s --list\n", prog_name);
    printf("  %s --interactive\n\n", prog_name);
}


void remove_track_from_playlist_interactive(SpotifyToken *token) {
    printf("\n=== Remove Track from Playlist ===\n");
    
    // Select playlist
    SpotifyPlaylistList *playlists = spotify_get_user_playlists(token, 20, 0);
    if (!playlists || playlists->count == 0) {
        printf("No playlists found.\n");
        if (playlists) spotify_free_playlist_list(playlists);
        return;
    }
    
    printf("\nYour playlists:\n\n");
    for (int i = 0; i < playlists->count; i++) {
        spotify_print_playlist(&playlists->playlists[i], i + 1);
        printf("\n");
    }
    
    printf("Select playlist (or 0 to cancel): ");
    int playlist_choice;
    if (scanf("%d", &playlist_choice) != 1 || playlist_choice <= 0 || 
        playlist_choice > playlists->count) {
        spotify_free_playlist_list(playlists);
        return;
    }
    getchar();
    
    const char *playlist_id = playlists->playlists[playlist_choice - 1].id;
    spotify_free_playlist_list(playlists);
    
    // Get playlist with tracks
    SpotifyPlaylistFull *playlist = spotify_get_playlist(token, playlist_id, true, 50);
    if (!playlist) {
        printf("Failed to get playlist.\n");
        return;
    }
    
    if (playlist->tracks_count == 0) {
        printf("Playlist is empty.\n");
        spotify_free_playlist_full(playlist);
        return;
    }
    
    spotify_print_playlist_full(playlist);
    
    printf("\nSelect track to remove (or 0 to cancel): ");
    int track_choice;
    if (scanf("%d", &track_choice) != 1 || track_choice <= 0 || 
        track_choice > playlist->tracks_count) {
        spotify_free_playlist_full(playlist);
        return;
    }
    getchar();
    
    const char *track_uri = playlist->tracks[track_choice - 1].uri;
    const char *track_name = playlist->tracks[track_choice - 1].name;
    
    printf("\nRemoving '%s' from '%s'...\n", track_name, playlist->name);
    
    const char *uris[] = { track_uri };
    SpotifyPlaylistResult *result = spotify_remove_tracks_from_playlist(
        token, playlist->id, uris, 1, playlist->snapshot_id
    );
    
    if (result && result->success) {
        printf("✅ Track removed successfully!\n");
        spotify_free_playlist_result(result);
    } else {
        printf("❌ Failed to remove track.\n");
    }
    
    spotify_free_playlist_full(playlist);
}


void search_and_save(SpotifyToken *token, const char *query) {
    printf("\nSearching for '%s'...\n", query);
    SpotifyTrackList *results = spotify_search_tracks(token, query, 10);

    if (!results || results->count == 0) {
        printf("No tracks found.\n");
        if (results) spotify_free_track_list(results);
        return;
    }

    printf("\nFound %d results (total: %d)\n\n", results->count, results->total);

    for (int i = 0; i < results->count; i++) {
        spotify_print_track(&results->tracks[i], i + 1);
        printf("\n");
    }

    printf("Enter track number to save (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_track_list(results);
        return;
    }
    getchar(); // consume newline

    if (choice > 0 && choice <= results->count) {
        const char *track_id = results->tracks[choice - 1].id;

        printf("Saving track...\n");
        if (spotify_save_tracks(token, &track_id, 1)) {
            printf("✅ Track saved to your library!\n");
        } else {
            printf("❌ Failed to save track.\n");
        }
    }

    spotify_free_track_list(results);
}


void search_artist_and_view_albums(SpotifyToken *token, const char *query) {
    printf("\nSearching for artists matching '%s'...\n", query);
    SpotifyArtistList *results = spotify_search_artists(token, query, 10);

    if (!results || results->count == 0) {
        printf("No artists found.\n");
        if (results) spotify_free_artist_list(results);
        return;
    }

    printf("\nFound %d results (total: %d)\n\n", results->count, results->total);

    for (int i = 0; i < results->count; i++) {
        spotify_print_artist(&results->artists[i], i + 1);
        printf("\n");
    }

    printf("Enter artist number to view albums (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_artist_list(results);
        return;
    }
    getchar(); // consume newline

    if (choice > 0 && choice <= results->count) {
        const char *artist_id = results->artists[choice - 1].id;
        const char *artist_name = results->artists[choice - 1].name;

        view_artist_albums(token, artist_id, artist_name);
    }

    spotify_free_artist_list(results);
}


void search_artist_and_view_top_tracks(SpotifyToken *token, const char *query) {
    printf("\nSearching for artists matching '%s'...\n", query);
    SpotifyArtistList *results = spotify_search_artists(token, query, 10);

    if (!results || results->count == 0) {
        printf("No artists found.\n");
        if (results) spotify_free_artist_list(results);
        return;
    }

    printf("\nFound %d results (total: %d)\n\n", results->count, results->total);

    for (int i = 0; i < results->count; i++) {
        spotify_print_artist(&results->artists[i], i + 1);
        printf("\n");
    }

    printf("Enter artist number to view top tracks (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_artist_list(results);
        return;
    }
    getchar(); // consume newline

    if (choice > 0 && choice <= results->count) {
        const char *artist_id = results->artists[choice - 1].id;
        const char *artist_name = results->artists[choice - 1].name;

        view_artist_top_tracks(token, artist_id, artist_name);
    }

    spotify_free_artist_list(results);
}


void search_artists(SpotifyToken *token, const char *query) {
    printf("\nSearching for artists matching '%s'...\n", query);
    SpotifyArtistList *results = spotify_search_artists(token, query, 10);

    if (!results || results->count == 0) {
        printf("No artists found.\n");
        if (results) spotify_free_artist_list(results);
        return;
    }

    printf("\nFound %d results (total: %d)\n\n", results->count, results->total);

    for (int i = 0; i < results->count; i++) {
        spotify_print_artist(&results->artists[i], i + 1);
        printf("\n");
    }

    spotify_free_artist_list(results);
}


void unfollow_playlist_interactive(SpotifyToken *token) {
    printf("\n=== Unfollowing Playlist ===\n");

    bool *playlist
}


void users_options() {
    // TODO: Should print every options available like (reload token)
    printf("\n");
}


void view_and_transfer_devices(SpotifyToken *token) {
    printf("\nFetching available devices...\n");

    int device_count = 0;
    SpotifyDevice *devices = spotify_get_available_devices(token, &device_count);

    if (!devices || device_count == 0) {
        printf("No devices found. Make sure Spotify is open on at least one device.\n");
        if (devices) free(devices);
        return;
    }

    printf("\nFound %d device(s):\n\n", device_count);

    for (int i = 0; i < device_count; i++) {
        spotify_print_device(&devices[i], i + 1);
        printf("\n");
    }

    printf("Enter device number to transfer playback (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        free(devices);
        return;
    }
    getchar(); // consume newline

    if (choice > 0 && choice <= device_count) {
        const char *device_id = devices[choice - 1].device_id;
        const char *device_name = devices[choice - 1].device_name;

        printf("Transfer playback to '%s'? Start playing? (y/n): ", device_name);
        char start_play;
        scanf(" %c", &start_play);
        getchar();

        bool play = (start_play == 'y' || start_play == 'Y');

        printf("Transferring playback to %s...\n", device_name);
        if (spotify_transfer_playback(token, device_id, play)) {
            printf("✅ Playback transferred successfully!\n");
        } else {
            printf("❌ Failed to transfer playback.\n");
        }
    }

    free(devices);
}


void view_artist_albums(SpotifyToken *token, const char *artist_id, const char *artist_name) {
    if (!token || !artist_id || !artist_name) {
        printf("Invalid parameters\n");
        return;
    }

    printf("Fetching albums for %s...\n", artist_name);

    SpotifyAlbumList *albums = spotify_get_artist_albums(token, artist_id);

    if (!albums) {
        printf("Failed to fetch albums.\n");
        return;
    }

    if (albums->count == 0) {
        printf("No albums found.\n");
        spotify_free_album_list(albums);
        return;
    }

    printf("\nFound %d album(s) by %s:\n\n", albums->count, artist_name);

    for (int i = 0; i < albums->count; i++) {
        spotify_print_album(&albums->albums[i], i + 1);
        printf("\n");
    }

    spotify_free_album_list(albums);
}


void view_artist_top_tracks(SpotifyToken *token, const char *artist_id, const char *artist_name) {
    printf("\nFetching top tracks for %s...\n", artist_name);

    SpotifyTrackList *tracks = spotify_get_artist_top_tracks(token, artist_id, "US");

    if (!tracks || tracks->count == 0) {
        printf("No tracks found.\n");
        if (tracks) spotify_free_track_list(tracks);
        return;
    }

    printf("\n🎵 Top %d tracks by %s:\n\n", tracks->count, artist_name);

    for (int i = 0; i < tracks->count; i++) {
        spotify_print_track(&tracks->tracks[i], i + 1);
        printf("\n");
    }

    // Option to save a track
    printf("Enter track number to save (or 0 to cancel): ");
    int choice;
    if (scanf("%d", &choice) != 1) {
        printf("Invalid input.\n");
        spotify_free_track_list(tracks);
        return;
    }
    getchar(); // consume newline

    if (choice > 0 && choice <= tracks->count) {
        const char *track_id = tracks->tracks[choice - 1].id;

        printf("Saving track...\n");
        if (spotify_save_tracks(token, &track_id, 1)) {
            printf("✅ Track saved to your library!\n");
        } else {
            printf("❌ Failed to save track.\n");
        }
    }

    spotify_free_track_list(tracks);
}


void view_queue(SpotifyToken *token) {
    printf("\nFetching playback queue...\n");
    
    SpotifyQueue *queue = spotify_get_queue(token);
    
    if (!queue) {
        printf("Failed to get queue. Make sure Spotify is playing on an active device.\n");
        return;
    }
    
    spotify_print_queue(queue);
    spotify_free_queue(queue);
}


void view_saved_tracks(SpotifyToken *token) {
    printf("\nFetching your saved tracks...\n");

    SpotifyTrackList *saved = spotify_get_saved_tracks(token, 20, 0);

    if (!saved || saved->count == 0) {
        printf("No saved tracks found.\n");
        if (saved) spotify_free_track_list(saved);
        return;
    }

    printf("\nYou have %d saved tracks (showing first %d)\n\n", 
            saved->total, saved->count);

    for (int i = 0; i < saved->count; i++) {
        spotify_print_track(&saved->tracks[i], i + 1);
        printf("\n");
    }

    spotify_free_track_list(saved);
}

void view_users_playlists(SpotifyToken *token, int limit, int offset) {
    if (!token) {
        printf("Invalid token\n");
        return;
    }
    
    printf("\nFetching your playlists...\n");
    
    SpotifyPlaylistList *playlist = spotify_get_user_playlists(token, limit, offset);
    
    if (!playlist || playlist->count == 0) {
        printf("No playlist found.\n");
        if (playlist) spotify_free_playlist_list(playlist);
        return;
    }

    printf("\nYou have %d playlists (showing first %d)\n\n",
            playlist->total, playlist->count);

    for (int i = 0; i < playlist->count; i++) {
        spotify_print_playlist(&playlist->playlists[i], i + 1);
        printf("\n");
    }
    
    spotify_free_playlist_list(playlist);
}

int main(int argc, char *argv[]) {
    // Load environment variables from .env file
    load_dotenv(".env");

    SpotifyToken token;

    // Authenticate first
    if (!spotify_get_access_token(&token)) {
        fprintf(stderr, "Failed to get access token.\n");
        return 1;
    }

    // No arguments - show usage
    if (argc < 2) {
        printf("✅ Authenticated successfully!\n");
        printf("Starting interactive mode...\n");
        interactive_mode(&token);
        return 0;
    }

    // Parse command line options
    int opt;
    int list_mode = 0;
    int player_state = 0;
    int interactive = 0;
    char *search_type = "track";

    static struct option long_options[] = {
        {"track",       no_argument, 0, 't'},
        {"artist",      no_argument, 0, 'a'},
        {"album",       no_argument, 0, 'A'},
        {"playlist",    no_argument, 0, 'p'},
        {"player",      no_argument, 0, 'P'},
        {"user",        no_argument, 0, 'u'},
        {"audiobook",   no_argument, 0, 'b'},
        {"list",        no_argument, 0, 'l'},
        {"interactive", no_argument, 0, 'i'},
        {"help",        no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "taApPublih", long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                search_type = "track";
                break;
            case 'a':
                search_type = "artist";
                break;
            case 'A':
                search_type = "album";
                break;
            case 'p':
                search_type = "playlist";
                break;
            case 'P':
                player_state = 1;
                break;
            case 'u':
                search_type = "user";
                break;
            case 'b':
                search_type = "audiobook";
                break;
            case 'l':
                list_mode = 1;
                break;
            case 'i':
                interactive = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    printf("✅ Authenticated successfully!\n");

    // Interactive mode
    if (interactive) {
        interactive_mode(&token);
        return 0;
    }

    // List mode
    if (list_mode) {
        view_saved_tracks(&token);
        return 0;
    }

    if (player_state) {
        SpotifyPlayerState *state = spotify_get_player_state(&token);
        spotify_print_player_state(state);
        spotify_free_player_state(state);
        return 0;
    }

    // Search mode - need a query
    if (optind >= argc) {
        fprintf(stderr, "Error: Search query required.\n");
        print_usage(argv[0]);
        return 1;
    }

    char *query = argv[optind];

    // Handle different search types
    if (strcmp(search_type, "track") == 0) {
        search_and_save(&token, query);
    } else if (strcmp(search_type, "artist") == 0) {
        search_artists(&token, query);
    } else if (strcmp(search_type, "album") == 0) {
        search_artist_and_view_albums(&token, query);
    } else if (strcmp(search_type, "playlist") == 0) {
        view_users_playlists(&token, limit=10, offset=0);
    } else if (strcmp(search_type, "user") == 0 ||
            strcmp(search_type, "audiobook") == 0) {
        printf("⚠️  Search type '%s' not yet implemented.\n", search_type);
        printf("Currently supported: track (-t), artist (-a)\n");
        return 1;
    } else {
        fprintf(stderr, "Error: Unknown search type '%s'\n", search_type);
        return 1;
    }

    return 0;
}
