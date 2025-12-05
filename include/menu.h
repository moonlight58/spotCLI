#ifndef MENU_H
#define MENU_H

#include "api.h"
#include <stdbool.h>

// Types de menu
typedef enum {
    MENU_TYPE_MAIN,
    MENU_TYPE_SEARCH,
    MENU_TYPE_LIBRARY,
    MENU_TYPE_PLAYER,
    MENU_TYPE_PLAYLIST,
    MENU_TYPE_SETTINGS
} MenuType;

// Structure pour une entrée de menu
typedef struct {
    int id;
    const char *label;
    const char *description;
    void (*action)(SpotifyToken *token);
    bool enabled;
} MenuItem;

// Structure pour un menu complet
typedef struct {
    MenuType type;
    const char *title;
    MenuItem *items;
    int item_count;
    int selected_index;
} Menu;

// Fonctions de gestion de menu
Menu* menu_create(MenuType type, const char *title);
void menu_add_item(Menu *menu, int id, const char *label, const char *description, 
                   void (*action)(SpotifyToken *token));
void menu_display(Menu *menu);
void menu_navigate(Menu *menu, int direction);
void menu_execute_selected(Menu *menu, SpotifyToken *token);
void menu_free(Menu *menu);

// Actions de menu (à implémenter)
void action_search_tracks(SpotifyToken *token);
void action_search_artists(SpotifyToken *token);
void action_view_saved_tracks(SpotifyToken *token);
void action_view_playlists(SpotifyToken *token);
void action_player_control(SpotifyToken *token);
void action_view_queue(SpotifyToken *token);
void action_create_playlist(SpotifyToken *token);
void action_manage_playlist(SpotifyToken *token);

#endif // MENU_H
