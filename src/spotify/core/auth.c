#include "spotify/spotify_internal.h"
#include <string.h>
#include <stdlib.h>

char* spotify_get_current_user_id(SpotifyToken *token) {
    const char *url = "https://api.spotify.com/v1/me";

    struct json_object *root = spotify_api_get(token, url);
    if (!root) return NULL;

    struct json_object *id_obj;
    if (!json_object_object_get_ex(root, "id", &id_obj)) {
        json_object_put(root);
        return NULL;
    }

    char *user_id = strdup(json_object_get_string(id_obj));
    json_object_put(root);

    return user_id;
}
