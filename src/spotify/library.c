#include "spotify/spotify_internal.h"
#include "spotify/spotify_library.h"
#include <stdio.h>

SpotifyPlaylistList* spotify_get_user_playlists(SpotifyToken *token, int limit, int offset) {
    char url[256];
    snprintf(url, sizeof(url),
             "https://api.spotify.com/v1/me/playlists?limit=%d&offset=%d",
             limit, offset);

    struct json_object *root = spotify_api_get(token, url);
    if (!root) return NULL;

    struct json_object *items;
    if (!json_object_object_get_ex(root, "items", &items)) {
        json_object_put(root);
        return NULL;
    }

    int count = json_object_array_length(items);
    SpotifyPlaylistList *list = malloc(sizeof(SpotifyPlaylistList));
    list->playlists = malloc(sizeof(SpotifyPlaylist) * count);
    list->count = count;

    struct json_object *total_obj;
    if (json_object_object_get_ex(root, "total", &total_obj)) {
        list->total = json_object_get_int(total_obj);
    } else {
        list->total = count;
    }

    for (int i = 0; i < count; i++) {
        struct json_object *item = json_object_array_get_idx(items, i);
        parse_playlist_json(item, &list->playlists[i]);
    }

    json_object_put(root);
    return list;
}
