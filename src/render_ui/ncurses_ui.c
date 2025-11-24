#include "render_ui/ncurses_ui.h"
#include <string.h>
#include <stdlib.h>
#include <locale.h>

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

// Help content structure
typedef struct {
    const char *key;
    const char *description;
} HelpItem;

// Define help content for different sections
static const HelpItem general_help[] = {
    {"↑ / k", "Move selection up"},
    {"↓ / j", "Move selection down"},
    {"Enter", "Select item / Confirm action"},
    {"q", "Quit application"},
    {"b", "Go back to previous screen"},
    {"? / h", "Show/hide this help menu"},
    {"Esc", "Close popup or cancel action"},
};

static const HelpItem track_list_help[] = {
    {"Enter", "Save track to your library"},
    {"Space", "Preview/Play track"},
    {"p", "Add track to playlist"},
    {"i", "Show track information"},
    {"a", "Add to queue"},
};

static const HelpItem artist_help[] = {
    {"Enter", "View artist details"},
    {"t", "View artist's top tracks"},
    {"a", "View artist's albums"},
    {"f", "Follow/Unfollow artist"},
};

static const HelpItem playlist_help[] = {
    {"Enter", "Open playlist"},
    {"e", "Edit playlist details"},
    {"d", "Delete/Unfollow playlist"},
    {"n", "Create new playlist"},
    {"a", "Add tracks to playlist"},
};

static const HelpItem player_help[] = {
    {"Space", "Play/Pause"},
    {"n", "Skip to next track"},
    {"p", "Skip to previous track"},
    {"s", "Toggle shuffle"},
    {"r", "Cycle repeat mode"},
    {"+/-", "Increase/Decrease volume"},
};

#define GENERAL_HELP_COUNT (sizeof(general_help) / sizeof(HelpItem))
#define TRACK_LIST_HELP_COUNT (sizeof(track_list_help) / sizeof(HelpItem))
#define ARTIST_HELP_COUNT (sizeof(artist_help) / sizeof(HelpItem))
#define PLAYLIST_HELP_COUNT (sizeof(playlist_help) / sizeof(HelpItem))
#define PLAYER_HELP_COUNT (sizeof(player_help) / sizeof(HelpItem))

// Initialize ncurses and create windows
UIState* ui_init(void) {
    UIState *ui = malloc(sizeof(UIState));
    if (!ui) return NULL;

    // CRITICAL: Set locale before initializing ncurses for UTF-8 support
    setlocale(LC_ALL, "");

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
        init_pair(COLOR_PAIR_HELP, COLOR_YELLOW, COLOR_BLACK);
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
    werase(win);
    if (title) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
        mvwprintw(win, 0, 2, " %s ", title);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
    }
}

static WINDOW* create_popup_window(int height, int width) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int start_y = (max_y - height) / 2;
    int start_x = (max_x - width) / 2;
    
    // Ensure popup fits on screen
    if (start_y < 0) start_y = 0;
    if (start_x < 0) start_x = 0;
    if (height > max_y) height = max_y;
    if (width > max_x) width = max_x;
    
    return newwin(height, width, start_y, start_x);
}

static int draw_help_section(WINDOW *win, int start_line, int max_line, 
                             const char *title, const HelpItem *items, 
                             size_t count, int scroll_offset) {
    int line = start_line;
    int screen_line = 2;
    
    // Section title
    if (line >= scroll_offset && screen_line < max_line) {
        wattron(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD | A_UNDERLINE);
        mvwprintw(win, screen_line, 2, "%s", title);
        wattroff(win, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD | A_UNDERLINE);
        screen_line++;
    }
    line++;
    
    // Empty line after title
    if (line >= scroll_offset && screen_line < max_line) {
        screen_line++;
    }
    line++;
    
    // Help items
    for (size_t i = 0; i < count; i++) {
        if (line >= scroll_offset && screen_line < max_line) {
            // Key binding (green/cyan)
            wattron(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
            mvwprintw(win, screen_line, 4, "%-12s", items[i].key);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
            
            // Description
            wattron(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            mvwprintw(win, screen_line, 17, "%s", items[i].description);
            wattroff(win, COLOR_PAIR(COLOR_PAIR_DEFAULT));
            
            screen_line++;
        }
        line++;
    }
    
    // Empty line after section
    if (line >= scroll_offset && screen_line < max_line) {
        screen_line++;
    }
    line++;
    
    return line;
}

// Show scrollable help popup
void ui_show_scrollable_help(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Calculate popup size
    int popup_height = (max_y > 32) ? 32 : max_y - 4;
    int popup_width = (max_x > 75) ? 75 : max_x - 4;
    
    if (popup_height < 10 || popup_width < 40) {
        // Screen too small for popup
        return;
    }
    
    WINDOW *popup = create_popup_window(popup_height, popup_width);
    if (!popup) return;
    
    // Calculate total content lines
    int total_lines = 0;
    total_lines += 3; // General + spacing
    total_lines += GENERAL_HELP_COUNT + 2;
    total_lines += 3; // Track List + spacing
    total_lines += TRACK_LIST_HELP_COUNT + 2;
    total_lines += 3; // Artist + spacing
    total_lines += ARTIST_HELP_COUNT + 2;
    total_lines += 3; // Playlist + spacing
    total_lines += PLAYLIST_HELP_COUNT + 2;
    total_lines += 3; // Player + spacing
    total_lines += PLAYER_HELP_COUNT + 2;
    
    int visible_lines = popup_height - 4; // Minus borders and footer
    int max_scroll = (total_lines > visible_lines) ? (total_lines - visible_lines) : 0;
    int scroll_offset = 0;
    
    bool done = false;
    
    // Enable keypad for arrow keys
    keypad(popup, TRUE);
    
    while (!done) {
        werase(popup);
        
        // Draw border
        wattron(popup, COLOR_PAIR(COLOR_PAIR_BORDER) | A_BOLD);
        box(popup, 0, 0);
        wattroff(popup, COLOR_PAIR(COLOR_PAIR_BORDER) | A_BOLD);
        
        // Title with icon
        wattron(popup, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
        int title_x = (popup_width - 20) / 2;
        mvwprintw(popup, 0, title_x, " 📖 KEYBOARD SHORTCUTS ");
        wattroff(popup, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
        
        // Draw content sections
        int line = 0;
        line = draw_help_section(popup, line, popup_height - 2, 
                                 "GENERAL CONTROLS", general_help, 
                                 GENERAL_HELP_COUNT, scroll_offset);
        
        line = draw_help_section(popup, line, popup_height - 2, 
                                 "TRACK LIST", track_list_help, 
                                 TRACK_LIST_HELP_COUNT, scroll_offset);
        
        line = draw_help_section(popup, line, popup_height - 2, 
                                 "ARTIST VIEW", artist_help, 
                                 ARTIST_HELP_COUNT, scroll_offset);
        
        line = draw_help_section(popup, line, popup_height - 2, 
                                 "PLAYLIST VIEW", playlist_help, 
                                 PLAYLIST_HELP_COUNT, scroll_offset);
        
        line = draw_help_section(popup, line, popup_height - 2, 
                                 "PLAYER CONTROLS", player_help, 
                                 PLAYER_HELP_COUNT, scroll_offset);
        
        // Scroll indicators
        if (max_scroll > 0) {
            if (scroll_offset > 0) {
                wattron(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
                mvwprintw(popup, 1, popup_width - 5, " ▲ ");
                wattroff(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
            }
            if (scroll_offset < max_scroll) {
                wattron(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
                mvwprintw(popup, popup_height - 2, popup_width - 5, " ▼ ");
                wattroff(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
            }
            
            // Scroll position indicator
            if (max_scroll > 0) {
                int scroll_pos = (scroll_offset * 100) / max_scroll;
                mvwprintw(popup, popup_height - 1, popup_width - 10, "%3d%%", scroll_pos);
            }
        }
        
        // Footer with instructions
        wattron(popup, COLOR_PAIR(COLOR_PAIR_FOOTER));
        const char *footer = "↑↓/j/k: Scroll | q/?/Esc: Close";
        mvwprintw(popup, popup_height - 1, (popup_width - strlen(footer)) / 2, "%s", footer);
        wattroff(popup, COLOR_PAIR(COLOR_PAIR_FOOTER));
        
        wrefresh(popup);
        
        // Handle input
        int ch = wgetch(popup);
        switch (ch) {
            case KEY_UP:
            case 'k':
                if (scroll_offset > 0) scroll_offset--;
                break;
                
            case KEY_DOWN:
            case 'j':
                if (scroll_offset < max_scroll) scroll_offset++;
                break;
                
            case KEY_PPAGE: // Page Up
                scroll_offset -= 5;
                if (scroll_offset < 0) scroll_offset = 0;
                break;
                
            case KEY_NPAGE: // Page Down
                scroll_offset += 5;
                if (scroll_offset > max_scroll) scroll_offset = max_scroll;
                break;
                
            case KEY_HOME:
                scroll_offset = 0;
                break;
                
            case KEY_END:
                scroll_offset = max_scroll;
                break;
                
            case 'q':
            case 'Q':
            case 27: // ESC
            case '?':
            case 'h':
            case 'H':
                done = true;
                break;
        }
    }
    
    // Cleanup
    delwin(popup);
    
    // Force complete redraw of all windows
    touchwin(stdscr);
    refresh();
}

// Simple non-scrolling help popup (for quick reference)
void ui_show_help_popup(void) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int popup_height = 20;
    int popup_width = 60;
    
    WINDOW *popup = create_popup_window(popup_height, popup_width);
    if (!popup) return;
    
    werase(popup);
    
    // Border
    wattron(popup, COLOR_PAIR(COLOR_PAIR_BORDER) | A_BOLD);
    box(popup, 0, 0);
    wattroff(popup, COLOR_PAIR(COLOR_PAIR_BORDER) | A_BOLD);
    
    // Title
    wattron(popup, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
    mvwprintw(popup, 0, (popup_width - 15) / 2, " 📖 QUICK HELP ");
    wattroff(popup, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
    
    int line = 2;
    
    // Just show general help
    wattron(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
    mvwprintw(popup, line++, 2, "GENERAL CONTROLS:");
    wattroff(popup, COLOR_PAIR(COLOR_PAIR_ACCENT) | A_BOLD);
    line++;
    
    for (size_t i = 0; i < GENERAL_HELP_COUNT && line < popup_height - 3; i++) {
        wattron(popup, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
        mvwprintw(popup, line, 4, "%-12s", general_help[i].key);
        wattroff(popup, COLOR_PAIR(COLOR_PAIR_PLAYING) | A_BOLD);
        
        mvwprintw(popup, line, 17, "%s", general_help[i].description);
        line++;
    }
    
    // Footer
    wattron(popup, COLOR_PAIR(COLOR_PAIR_FOOTER));
    mvwprintw(popup, popup_height - 1, (popup_width - 25) / 2, "Press any key to close");
    wattroff(popup, COLOR_PAIR(COLOR_PAIR_FOOTER));
    
    wrefresh(popup);
    
    // Wait for key press
    nodelay(popup, FALSE);
    wgetch(popup);
    
    // Cleanup
    delwin(popup);
    touchwin(stdscr);
    refresh();
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
            
            case '?':  // NEW: Help key
            case 'h':  // Also allow 'h' for help
                ui_show_scrollable_help();
                // Redraw all windows after popup closes
                ui_draw_header(ui->header, "Spotify TUI");
                ui_draw_sidebar(ui->sidebar, ui->selected_item);
                ui_draw_footer(ui->footer, NULL);
                break;
                
            case 'q':
            case 'Q':
                ui->running = false;
                break;
        }
    }
}
