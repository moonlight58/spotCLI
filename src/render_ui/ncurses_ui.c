#include "render_ui/ncurses_ui.h"
#include <string.h>
#include <stdlib.h>

static const char *menu_items[] = {
    "Search Tracks",
    "Search Artists",
    "View Saved Tracks",
    "View Playlists",
    "View Queue",
    "Player State",
    "Devices",
    "Recently Played",
    "Quit"
};

// Initialize ncurses and create windows
UIState* ui_init(void) {
    UIState *ui = malloc(sizeof(UIState));
    if (!ui) return NULL;

    // Initialize ncurses
    initscr();
    if(has_colors() == FALSE){
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Hide cursor

    // Initialize colors
    if (has_colors()) {
        start_color();
        init_pair(COLOR_PAIR_DEFAULT, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_HEADER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_FOOTER, COLOR_CYAN, COLOR_BLACK);
        init_pair(COLOR_PAIR_SELECTED, COLOR_BLACK, COLOR_WHITE);
        init_pair(COLOR_PAIR_PLAYING, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_BORDER, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_PAIR_ACCENT, COLOR_CYAN, COLOR_BLACK);
    }

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Create windows
    ui->header = newwin(HEADER_HEIGHT, max_x, 0, 0);
    ui->sidebar = newwin(max_y - HEADER_HEIGHT - FOOTER_HEIGHT,
                         SIDEBAR_WIDTH, HEADER_HEIGHT, 0);
    ui->main_content = newwin(max_y - HEADER_HEIGHT - FOOTER_HEIGHT,
                              max_x - SIDEBAR_WIDTH,
                              HEADER_HEIGHT, SIDEBAR_WIDTH);
    ui->footer = newwin(FOOTER_HEIGHT, max_x, max_y - FOOTER_HEIGHT, 0);

    ui->selected_item = 0;
    ui->scroll_offset = 0;
    ui->max_items = 0;
    ui->running = true;

    // Enable scrolling and keypad for main content
    scrollok(ui->main_content, TRUE);
    keypad(ui->main_content, TRUE);

    return ui;
}

// Cleanup and restore terminal
void ui_cleanup(UIState *ui) {
    if (!ui) return;

    delwin(ui->header);
    delwin(ui->sidebar);
    delwin(ui->main_content);
    delwin(ui->footer);
    endwin();
    free(ui);
}

// Draw header with title
void ui_draw_header(WINDOW *win, const char *title) {
    werase(win);
    wattron(win, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);

    int width = getmaxx(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "SpoTUI");
    if (title) {
        mvwprintw(win, 1, width - strlen(title) - 2, "%s", title);
    }

    wattroff(win, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
    wrefresh(win);
}

// Draw footer with help text
void ui_draw_footer(WINDOW *win, const char *help_text) {
    werase(win);
    wattron(win, COLOR_PAIR(COLOR_PAIR_FOOTER));

    int width = getmaxx(win);
    box(win, 0, 0);

    if (!help_text) {
        help_text = "↑↓: Navigate | Enter: Select | q: Quit | ?: Help";
    }
    mvwprintw(win, 1, 2, "%s", help_text);

    wattroff(win, COLOR_PAIR(COLOR_PAIR_FOOTER));
    wrefresh(win);
}

// Draw sidebar menu
void ui_draw_sidebar(WINDOW *win, int selected) {
    werase(win);

    wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));

    mvwprintw(win, 0, 2, " MENU ");

    int max_y = getmaxy(win);

    for (int i = 0; i < MENU_COUNT && i < max_y - 2; i++) {
        if (i == selected) {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
            mvwprintw(win, i + 2, 1, " ▶ %s", menu_items[i]);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        } else {
            mvwprintw(win, i + 2, 1, "   %s", menu_items[i]);
        }
    }

    wrefresh(win);
}

// Draw box with title
void ui_draw_box_with_title(WINDOW *win, const char *title) {
    wattron(win, COLOR_PAIR(COLOR_PAIR_BORDER));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(COLOR_PAIR_BORDER));

    if (title) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
        mvwprintw(win, 0, 2, " %s ", title);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
    }
}

// Draw progress bar
void ui_progress_bar(WINDOW *win, int y, int x, int width, float progress) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    int filled = (int)(width * progress);

    mvwaddch(win, y, x, '[');
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            wattron(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
            waddch(win, '#');
            wattroff(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
        } else {
            waddch(win, '.');
        }
    }
    waddch(win, ']');
}

// Draw track list
void ui_draw_track_list(WINDOW *win, SpotifyTrackList *tracks, int selected, int offset) {
    werase(win);
    ui_draw_box_with_title(win, "Tracks");

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (!tracks || tracks->count == 0) {
        mvwprintw(win, max_y / 2, (max_x - 15) / 2, "No tracks found");
        wrefresh(win);
        return;
    }

    int visible_items = max_y - 4;

    for (int i = 0; i < visible_items && (i + offset) < tracks->count; i++) {
        int idx = i + offset;
        SpotifyTrack *track = &tracks->tracks[idx];

        if (idx == selected) {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }

        // Track number and name
        mvwprintw(win, i + 2, 2, "%3d. ", idx + 1);
        wprintw(win, "%.35s", track->name);

        // Artist
        mvwprintw(win, i + 2, 45, "%.20s", track->artist);

        // Duration
        int min = track->duration_ms / 60000;
        int sec = (track->duration_ms / 1000) % 60;
        mvwprintw(win, i + 2, max_x - 10, "%d:%02d", min, sec);

        if (idx == selected) {
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }
    }

    // Scroll indicator
    if (tracks->count > visible_items) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_ACCENT));
        mvwprintw(win, max_y - 1, max_x - 20, "[%d/%d]",
                  selected + 1, tracks->count);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_ACCENT));
    }

    wrefresh(win);
}

// Draw artist list
void ui_draw_artist_list(WINDOW *win, SpotifyArtistList *artists, int selected, int offset) {
    werase(win);
    ui_draw_box_with_title(win, "Artists");

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (!artists || artists->count == 0) {
        mvwprintw(win, max_y / 2, (max_x - 17) / 2, "No artists found");
        wrefresh(win);
        return;
    }

    int visible_items = max_y - 4;

    for (int i = 0; i < visible_items && (i + offset) < artists->count; i++) {
        int idx = i + offset;
        SpotifyArtist *artist = &artists->artists[idx];

        if (idx == selected) {
            wattron(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }

        mvwprintw(win, i + 2, 2, "%3d. ", idx + 1);
        wprintw(win, "%.40s", artist->name);

        // Followers and popularity
        mvwprintw(win, i + 3, 7, "Followers: %d | Popularity: %d/100",
                  artist->followers, artist->popularity);

        if (strlen(artist->genres) > 0) {
            mvwprintw(win, i + 4, 7, "Genres: %.50s", artist->genres);
        }

        if (idx == selected) {
            wattroff(win, COLOR_PAIR(COLOR_PAIR_SELECTED) | A_BOLD);
        }

        i += 2;  // Extra spacing
    }

    wrefresh(win);
}

// Draw player state
void ui_draw_player_state(WINDOW *win, SpotifyPlayerState *state) {
    werase(win);
    ui_draw_box_with_title(win, "Now Playing");

    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    if (!state) {
        mvwprintw(win, max_y / 2, (max_x - 20) / 2, "No active playback");
        wrefresh(win);
        return;
    }

    // Track info
    wattron(win, A_BOLD);
    mvwprintw(win, 2, 2, "Track:");
    wattroff(win, A_BOLD);
    mvwprintw(win, 2, 10, "%s", state->track_name);

    wattron(win, A_BOLD);
    mvwprintw(win, 3, 2, "Artist:");
    wattroff(win, A_BOLD);
    mvwprintw(win, 3, 10, "%s", state->artist_name);

    wattron(win, A_BOLD);
    mvwprintw(win, 4, 2, "Album:");
    wattroff(win, A_BOLD);
    mvwprintw(win, 4, 10, "%s", state->album_name);

    // Progress
    int progress_sec = state->progress_ms / 1000;
    int duration_sec = state->duration_ms / 1000;

    mvwprintw(win, 6, 2, "%d:%02d", progress_sec / 60, progress_sec % 60);
    ui_progress_bar(win, 6, 10, max_x - 25,
                    (float)state->progress_ms / state->duration_ms);
    mvwprintw(win, 6, max_x - 10, "%d:%02d",
              duration_sec / 60, duration_sec % 60);

    // Status
    wattron(win, COLOR_PAIR(state->is_playing ? COLOR_PAIR_PLAYING : COLOR_PAIR_DEFAULT));
    mvwprintw(win, 8, 2, "Status: %s", state->is_playing ? "▶ Playing" : "⏸ Paused");
    wattroff(win, COLOR_PAIR(state->is_playing ? COLOR_PAIR_PLAYING : COLOR_PAIR_DEFAULT));

    mvwprintw(win, 9, 2, "Shuffle: %s | Repeat: %s",
              state->shuffle_state ? "On" : "Off", state->repeat_state);

    // Device info
    mvwprintw(win, 11, 2, "Device: %s (%s)",
              state->device.device_name, state->device.device_type);
    mvwprintw(win, 12, 2, "Volume: %d%%", state->device.volume_percent);

    wrefresh(win);
}

// Show message in window
void ui_show_message(WINDOW *win, const char *message) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    werase(win);
    ui_draw_box_with_title(win, NULL);

    wattron(win, A_BOLD);
    mvwprintw(win, max_y / 2, (max_x - strlen(message)) / 2, "%s", message);
    wattroff(win, A_BOLD);

    wrefresh(win);
    napms(1500);  // Show for 1.5 seconds
}

// Get input from user
char* ui_get_input(WINDOW *win, const char *prompt) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);

    werase(win);
    ui_draw_box_with_title(win, "Input");

    mvwprintw(win, max_y / 2 - 1, 2, "%s", prompt);
    wrefresh(win);

    // Show cursor and enable echo
    curs_set(1);
    echo();

    char *input = malloc(256);
    mvwgetnstr(win, max_y / 2, 2, input, 255);

    // Hide cursor and disable echo
    noecho();
    curs_set(0);

    return input;
}

// Main input handling loop
void ui_handle_input(UIState *ui, SpotifyToken *token) {
    while (ui->running) {
        ui_draw_header(ui->header, "Spotify TUI");
        ui_draw_sidebar(ui->sidebar, ui->selected_item);
        ui_draw_footer(ui->footer, NULL);

        int ch = getch();

        switch (ch) {
            case KEY_UP:
            case 'k':
                if (ui->selected_item > 0) {
                    ui->selected_item--;
                }
                break;

            case KEY_DOWN:
            case 'j':
                if (ui->selected_item < MENU_COUNT - 1) {
                    ui->selected_item++;
                }
                break;

            case '\n':
            case KEY_ENTER:
                // Handle menu selection
                switch (ui->selected_item) {
                    case MENU_SEARCH_TRACKS:
                        ui_search_tracks_interface(ui, token);
                        break;
                    case MENU_SEARCH_ARTISTS:
                        ui_search_artists_interface(ui, token);
                        break;
                    case MENU_VIEW_SAVED:
                        ui_view_saved_tracks(ui, token);
                        break;
                    case MENU_VIEW_PLAYLISTS:
                        ui_view_playlists(ui, token);
                        break;
                    case MENU_VIEW_QUEUE:
                        ui_view_queue(ui, token);
                        break;
                    case MENU_PLAYER_STATE:
                        ui_view_player_state(ui, token);
                        break;
                    case MENU_DEVICES:
                        ui_view_devices(ui, token);
                        break;
                    case MENU_QUIT:
                        ui->running = false;
                        break;
                }
                break;

            case 'q':
            case 'Q':
                ui->running = false;
                break;
        }
    }
}
