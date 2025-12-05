#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Menu* menu_create(MenuType type, const char *title) {
    Menu *menu = malloc(sizeof(Menu));
    if (!menu) return NULL;
    
    menu->type = type;
    menu->title = title;
    menu->items = NULL;
    menu->item_count = 0;
    menu->selected_index = 0;
    
    return menu;
}

void menu_add_item(Menu *menu, int id, const char *label, const char *description,
                   void (*action)(SpotifyToken *token)) {
    if (!menu) return;
    
    menu->item_count++;
    menu->items = realloc(menu->items, sizeof(MenuItem) * menu->item_count);
    
    MenuItem *item = &menu->items[menu->item_count - 1];
    item->id = id;
    item->label = label;
    item->description = description;
    item->action = action;
    item->enabled = true;
}

void menu_display(Menu *menu) {
    if (!menu) return;
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║  %-52s  ║\n", menu->title);
    printf("╠════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < menu->item_count; i++) {
        MenuItem *item = &menu->items[i];
        
        const char *marker = (i == menu->selected_index) ? "►" : " ";
        const char *status = item->enabled ? "" : "[DISABLED]";
        
        printf("║ %s %2d. %-42s %6s ║\n", 
               marker, item->id, item->label, status);
               
        if (item->description && strlen(item->description) > 0) {
            printf("║      └─ %-44s  ║\n", item->description);
        }
    }
    
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n[↑/↓] Navigate  [Enter] Select  [q] Back/Quit\n");
}

void menu_navigate(Menu *menu, int direction) {
    if (!menu || menu->item_count == 0) return;
    
    menu->selected_index += direction;
    
    // Wrap around
    if (menu->selected_index < 0) {
        menu->selected_index = menu->item_count - 1;
    } else if (menu->selected_index >= menu->item_count) {
        menu->selected_index = 0;
    }
}

void menu_execute_selected(Menu *menu, SpotifyToken *token) {
    if (!menu || menu->selected_index < 0 || 
        menu->selected_index >= menu->item_count) {
        return;
    }
    
    MenuItem *item = &menu->items[menu->selected_index];
    
    if (item->enabled && item->action) {
        item->action(token);
    }
}

void menu_free(Menu *menu) {
    if (!menu) return;
    
    if (menu->items) {
        free(menu->items);
    }
    free(menu);
}
