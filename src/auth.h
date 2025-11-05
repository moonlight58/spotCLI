#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>
#include <stdlib.h>

#define TOKEN_DIR ".config/spotCLI"
#define TOKEN_FILENAME "token.json"

typedef struct {
    char access_token[512];
    char refresh_token[512];
    long expires_in;
    time_t obtained_at;
} SpotifyToken;

bool spotify_is_authenticated();
bool spotify_authorize(SpotifyToken *token);
bool spotify_refresh_token(SpotifyToken *token);
bool spotify_load_token(SpotifyToken *token);
bool spotify_save_token(SpotifyToken *token);
bool spotify_get_access_token(SpotifyToken *token);
char* get_token_path();

#endif
