#ifndef NCURSES_UI_H
#define NCURSES_UI_H

#include <ncurses.h>
#include "api.h"

// Window dimensions and positions
#define HEADER_HEIGHT 3
#define FOOTER_HEIGHT 3
#define SIDEBAR_WIDTH 30

// Color pairs
enum {
    COLOR_PAIR_DEFAULT = 1,
    COLOR_PAIR_HEADER,
    COLOR_PAIR_FOOTER,
    COLOR_PAIR_SELECTED,
    COLOR_PAIR_PLAYING,
    COLOR_PAIR_BORDER,
    COLOR_PAIR_ACCENT
};

// UI State
typedef struct {
    WINDOW *header;
    WINDOW *sidebar;
    WINDOW *main_content;
    WINDOW *footer;
    int selected_item;
    int scroll_offset;
    int max_items;
    bool running;
} UIState;

// Menu items
typedef enum {
    MENU_SEARCH_TRACKS,
    MENU_SEARCH_ARTISTS,
    MENU_VIEW_SAVED,
    MENU_VIEW_PLAYLISTS,
    MENU_VIEW_QUEUE,
    MENU_PLAYER_STATE,
    MENU_DEVICES,
    MENU_RECENTLY_PLAYED,
    MENU_QUIT,
    MENU_COUNT
} MenuItem;

// Initialize ncurses UI
UIState* ui_init(void);

// Cleanup ncurses UI
void ui_cleanup(UIState *ui);

// Draw functions
void ui_draw_header(WINDOW *win, const char *title);
void ui_draw_footer(WINDOW *win, const char *help_text);
void ui_draw_sidebar(WINDOW *win, int selected);
void ui_draw_track_list(WINDOW *win, SpotifyTrackList *tracks, int selected, int offset);
void ui_draw_artist_list(WINDOW *win, SpotifyArtistList *artists, int selected, int offset);
void ui_draw_playlist_list(WINDOW *win, SpotifyPlaylistList *playlists, int selected, int offset);
void ui_draw_player_state(WINDOW *win, SpotifyPlayerState *state);
void ui_draw_queue(WINDOW *win, SpotifyQueue *queue, int selected, int offset);

// Input handling
void ui_handle_input(UIState *ui, SpotifyToken *token);

// Helper functions
void ui_show_message(WINDOW *win, const char *message);
void ui_draw_box_with_title(WINDOW *win, const char *title);
void ui_progress_bar(WINDOW *win, int y, int x, int width, float progress);
char* ui_get_input(WINDOW *win, const char *prompt);

// Search interface
void ui_search_tracks_interface(UIState *ui, SpotifyToken *token);
void ui_search_artists_interface(UIState *ui, SpotifyToken *token);

// View interfaces
void ui_view_saved_tracks(UIState *ui, SpotifyToken *token);
void ui_view_playlists(UIState *ui, SpotifyToken *token);
void ui_view_queue(UIState *ui, SpotifyToken *token);
void ui_view_player_state(UIState *ui, SpotifyToken *token);
void ui_view_devices(UIState *ui, SpotifyToken *token);

#endif // NCURSES_UI_H
