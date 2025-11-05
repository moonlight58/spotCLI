#include "auth.h"
#include "api.h"
#include "dotenv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS] \"search query\"\n\n", prog_name);
    printf("Options:\n");
    printf("  -t, --track       Search for tracks (default)\n");
    printf("  -a, --artist      Search for artists\n");
    printf("  -A, --album       Search for albums\n");
    printf("  -p, --playlist    Search for playlists\n");
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

void print_menu() {
    printf("\n=== findSpot - Spotify CLI ===\n");
    printf("1. Exit\n");
    printf("2. Options\n");
    printf("3. View saved tracks\n");
    printf("4. Search for artists\n");
    printf("5. Search for tracks\n");
    printf("Choose an option: ");
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

void users_options() {
    // TODO: Should print every options available like (reload token)
    printf("still in development");
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
            case 4: { // SEARCH FOR AN ARTIST FROM QUERY
                printf("\nEnter artist name: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_artists(token, query);
                }
                break;
            }
            case 5: { // SEARCH A SONG FROM QUERY
                printf("\nEnter search query: ");
                char query[256];
                if (fgets(query, sizeof(query), stdin)) {
                    query[strcspn(query, "\n")] = '\0';
                    search_and_save(token, query);
                }
                break;
            }
            default:
                printf("Invalid option. Please try again.\n");
        }
    }
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
    int interactive = 0;
    char *search_type = "track";
    
    static struct option long_options[] = {
        {"track",       no_argument, 0, 't'},
        {"artist",      no_argument, 0, 'a'},
        {"album",       no_argument, 0, 'A'},
        {"playlist",    no_argument, 0, 'p'},
        {"user",        no_argument, 0, 'u'},
        {"audiobook",   no_argument, 0, 'b'},
        {"list",        no_argument, 0, 'l'},
        {"interactive", no_argument, 0, 'i'},
        {"help",        no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "taApublih", long_options, &option_index)) != -1) {
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
    } else if (strcmp(search_type, "album") == 0 || 
            strcmp(search_type, "playlist") == 0 ||
            strcmp(search_type, "user") == 0 ||
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
