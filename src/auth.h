#ifndef AUTH_H
#define AUTH_H

#include <stdbool.h>

#define CLIENT_ID "your_client_id"
#define CLIENT_SECRET "your_client_secret"
#define REDIRECT_URI "http://127.0.0.1:8888/callback"
#define TOKEN_DIR ".config/findSpot"
#define TOKEN_FILENAME "token.json"

typedef struct {
    char access_token[512];
    char refresh_token[512];
    long expires_in;
} SpotifyToken;

bool spotify_is_authenticated();
bool spotify_authorize(SpotifyToken *token);
bool spotify_refresh_token(SpotifyToken *token);
bool spotify_load_token(SpotifyToken *token);
bool spotify_save_token(SpotifyToken *token);
bool spotify_get_access_token(SpotifyToken *token);
char* get_token_path();

#endif
