#include "render_ui/ncurses_ui.h"
#include <stdlib.h>
#include <string.h>

// Search tracks interface
void ui_search_tracks_interface(UIState *ui, SpotifyToken *token) {
    char *query = ui_get_input(ui->main_content, "Enter search query:");

    if (!query || strlen(query) == 0) {
        free(query);
        return;
    }

    ui_show_message(ui->main_content, "Searching...");

    SpotifyTrackList *results = spotify_search_tracks(token, query, 50);
    free(query);

    if (!results || results->count == 0) {
        ui_show_message(ui->main_content, "No tracks found");
        if (results) spotify_free_track_list(results);
        return;
    }

    // Browse results
    int selected = 0;
    int offset = 0;
    int max_y = getmaxy(ui->main_content);
    int visible_items = max_y - 4;
    bool browsing = true;

    while (browsing) {
        ui_draw_track_list(ui->main_content, results, selected, offset);
        ui_draw_footer(ui->footer, "↑↓: Navigate | Enter: Save | b: Back | q: Quit");

        int ch = getch();

        switch (ch) {
            case KEY_UP:
            case 'k':
                if (selected > 0) {
                    selected--;
                    if (selected < offset) {
                        offset--;
                    }
                }
                break;

            case KEY_DOWN:
            case 'j':
                if (selected < results->count - 1) {
                    selected++;
                    if (selected >= offset + visible_items) {
                        offset++;
                    }
                }
                break;

            case '\n':
            case KEY_ENTER:
                // Save track
                {
                    const char *track_id = results->tracks[selected].id;
                    if (spotify_save_tracks(token, &track_id, 1)) {
                        ui_show_message(ui->main_content, "✓ Track saved!");
                    } else {
                        ui_show_message(ui->main_content, "✗ Failed to save track");
                    }
                }
                break;

            case 'b':
            case 'B':
                browsing = false;
                break;

            case 'q':
            case 'Q':
                browsing = false;
                ui->running = false;
                break;
        }
    }

    spotify_free_track_list(results);
}

// Search artists interface
void ui_search_artists_interface(UIState *ui, SpotifyToken *token) {
    char *query = ui_get_input(ui->main_content, "Enter artist name:");

    if (!query || strlen(query) == 0) {
        free(query);
        return;
    }

    ui_show_message(ui->main_content, "Searching...");

    SpotifyArtistList *results = spotify_search_artists(token, query, 20);
    free(query);

    if (!results || results->count == 0) {
        ui_show_message(ui->main_content, "No artists found");
        if (results) spotify_free_artist_list(results);
        return;
    }

    // Browse results
    int selected = 0;
    int offset = 0;
    bool browsing = true;

    while (browsing) {
        ui_draw_artist_list(ui->main_content, results, selected, offset);
        ui_draw_footer(ui->footer, "↑↓: Navigate | Enter: View Tracks | b: Back");

        int ch = getch();

        switch (ch) {
            case KEY_UP:
            case 'k':
                if (selected > 0) selected--;
                break;

            case KEY_DOWN:
            case 'j':
                if (selected < results->count - 1) selected++;
                break;

            case '\n':
            case KEY_ENTER:
                // Show artist's top tracks
                {
                    const char *artist_id = results->artists[selected].id;
                    SpotifyTrackList *tracks = spotify_get_artist_top_tracks(token, artist_id, "US");

                    if (tracks && tracks->count > 0) {
                        int track_sel = 0;
                        int track_off = 0;
                        bool viewing = true;

                        while (viewing) {
                            ui_draw_track_list(ui->main_content, tracks, track_sel, track_off);
                            ui_draw_footer(ui->footer, "↑↓: Navigate | Enter: Save | b: Back");

                            int tch = getch();
                            switch (tch) {
                                case KEY_UP:
                                case 'k':
                                    if (track_sel > 0) track_sel--;
                                    break;
                                case KEY_DOWN:
                                case 'j':
                                    if (track_sel < tracks->count - 1) track_sel++;
                                    break;
                                case '\n':
                                case KEY_ENTER:
                                    {
                                        const char *track_id = tracks->tracks[track_sel].id;
                                        if (spotify_save_tracks(token, &track_id, 1)) {
                                            ui_show_message(ui->main_content, "✓ Track saved!");
                                        }
                                    }
                                    break;
                                case 'b':
                                case 'B':
                                    viewing = false;
                                    break;
                            }
                        }

                        spotify_free_track_list(tracks);
                    }
                }
                break;

            case 'b':
            case 'B':
                browsing = false;
                break;

            case 'q':
            case 'Q':
                browsing = false;
                ui->running = false;
                break;
        }
    }

    spotify_free_artist_list(results);
}

// View saved tracks
void ui_view_saved_tracks(UIState *ui, SpotifyToken *token) {
    ui_show_message(ui->main_content, "Loading saved tracks...");

    SpotifyTrackList *tracks = spotify_get_saved_tracks(token, 50, 0);

    if (!tracks || tracks->count == 0) {
        ui_show_message(ui->main_content, "No saved tracks found");
        if (tracks) spotify_free_track_list(tracks);
        return;
    }

    int selected = 0;
    int offset = 0;
    int max_y = getmaxy(ui->main_content);
    int visible_items = max_y - 4;
    bool viewing = true;

    while (viewing) {
        ui_draw_track_list(ui->main_content, tracks, selected, offset);
        ui_draw_footer(ui->footer, "↑↓: Navigate | b: Back | q: Quit");

        int ch = getch();

        switch (ch) {
            case KEY_UP:
            case 'k':
                if (selected > 0) {
                    selected--;
                    if (selected < offset) offset--;
                }
                break;

            case KEY_DOWN:
            case 'j':
                if (selected < tracks->count - 1) {
                    selected++;
                    if (selected >= offset + visible_items) offset++;
                }
                break;

            case 'b':
            case 'B':
                viewing = false;
                break;

            case 'q':
            case 'Q':
                viewing = false;
                ui->running = false;
                break;
        }
    }

    spotify_free_track_list(tracks);
}

// View playlists
void ui_view_playlists(UIState *ui, SpotifyToken *token) {
    ui_show_message(ui->main_content, "Loading playlists...");

    SpotifyPlaylistList *playlists = spotify_get_user_playlists(token, 50, 0);

    if (!playlists || playlists->count == 0) {
        ui_show_message(ui->main_content, "No playlists found");
        if (playlists) spotify_free_playlist_list(playlists);
        return;
    }

    ui_draw_playlist_list(ui->main_content, playlists, 0, 0);
    ui_draw_footer(ui->footer, "Press any key to continue...");
    getch();

    spotify_free_playlist_list(playlists);
}

// Draw playlist list
void ui_draw_playlist_list(WINDOW *win, SpotifyPlaylistList *playlists, int selected, int offset) {
    werase(win);
    ui_draw_box_with_title(win, "Your Playlists");

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (!playlists || playlists->count == 0) {
        mvwprintw(win, max_y / 2, (max_x - 19) / 2, "No playlists found");
        wrefresh(win);
        return;
    }

    int visible_items = max_y - 4;

    for (int i = 0; i < visible_items && (i + offset) < playlists->count; i++) {
        int idx = i + offset;
        SpotifyPlaylist *playlist = &playlists->playlists[idx];

        if (idx == selected) {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }

        mvwprintw(win, i + 2, 2, "%3d. ", idx + 1);
        wprintw(win, "%.40s", playlist->name);
        mvwprintw(win, i + 2, 50, "(%d tracks)", playlist->count_tracks);

        if (idx == selected) {
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }
    }

    wrefresh(win);
}

// View queue
void ui_view_queue(UIState *ui, SpotifyToken *token) {
    ui_show_message(ui->main_content, "Loading queue...");

    SpotifyQueue *queue = spotify_get_queue(token);

    if (!queue) {
        ui_show_message(ui->main_content, "No active playback");
        return;
    }

    ui_draw_queue(ui->main_content, queue, 0, 0);
    ui_draw_footer(ui->footer, "Press any key to continue...");
    getch();

    spotify_free_queue(queue);
}

// Draw queue
void ui_draw_queue(WINDOW *win, SpotifyQueue *queue, int selected, int offset) {
    werase(win);
    ui_draw_box_with_title(win, "Playback Queue");

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    int line = 2;

    if (!queue) {
        mvwprintw(win, max_y / 2, (max_x - 20) / 2, "No active playback");
        wrefresh(win);
        return;
    }

    // Currently playing
    wattron(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
    mvwprintw(win, line++, 2, "🎵 NOW PLAYING:");
    wattroff(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);

    mvwprintw(win, line++, 4, "%s", queue->currently_playing.name);
    mvwprintw(win, line++, 4, "by %s", queue->currently_playing.artist);
    line++;

    // Queue
    if (queue->queue_count > 0) {
        wattron(win, A_BOLD);
        mvwprintw(win, line++, 2, "NEXT IN QUEUE:");
        wattroff(win, A_BOLD);
        line++;

        for (int i = 0; i < queue->queue_count && line < max_y - 2; i++) {
            mvwprintw(win, line++, 4, "%d. %s", i + 1, queue->queue[i].name);
            mvwprintw(win, line++, 7, "by %s", queue->queue[i].artist);
        }
    }

    wrefresh(win);
}

// View player state
void ui_view_player_state(UIState *ui, SpotifyToken *token) {
    SpotifyPlayerState *state = spotify_get_player_state(token);

    if (!state) {
        ui_show_message(ui->main_content, "No active playback");
        return;
    }

    ui_draw_player_state(ui->main_content, state);
    ui_draw_footer(ui->footer, "Press any key to continue...");
    getch();

    spotify_free_player_state(state);
}

// View devices
void ui_view_devices(UIState *ui, SpotifyToken *token) {
    ui_show_message(ui->main_content, "Loading devices...");

    int device_count = 0;
    SpotifyDevice *devices = spotify_get_available_devices(token, &device_count);

    if (!devices || device_count == 0) {
        ui_show_message(ui->main_content, "No devices found");
        if (devices) free(devices);
        return;
    }

    werase(ui->main_content);
    ui_draw_box_with_title(ui->main_content, "Available Devices");

    for (int i = 0; i < device_count && i < 10; i++) {
        mvwprintw(ui->main_content, i + 2, 2, "%d. %s", i + 1, devices[i].device_name);
        mvwprintw(ui->main_content, i + 3, 5, "Type: %s | Volume: %d%% | Active: %s",
                  devices[i].device_type, devices[i].volume_percent,
                  devices[i].is_active ? "Yes" : "No");
    }

    wrefresh(ui->main_content);
    ui_draw_footer(ui->footer, "Press any key to continue...");
    getch();

    free(devices);
}
