#ifndef SPOTIFY_ENDPOINTS_H
#define SPOTIFY_ENDPOINTS_H

// Base URL
#define SPOTIFY_API_BASE "https://api.spotify.com/v1"

// Album endpoints
#define ENDPOINT_ALBUM              SPOTIFY_API_BASE "/albums/%s"
#define ENDPOINT_ALBUMS             SPOTIFY_API_BASE "/albums"
#define ENDPOINT_ALBUM_TRACKS       SPOTIFY_API_BASE "/albums/%s/tracks"
#define ENDPOINT_USER_ALBUMS        SPOTIFY_API_BASE "/me/albums"
#define ENDPOINT_CHECK_ALBUMS       SPOTIFY_API_BASE "/me/albums/contains"
#define ENDPOINT_NEW_RELEASES       SPOTIFY_API_BASE "/browse/new-releases"

// Artist endpoints
#define ENDPOINT_ARTIST             SPOTIFY_API_BASE "/artists/%s"
#define ENDPOINT_ARTISTS            SPOTIFY_API_BASE "/artists"
#define ENDPOINT_ARTIST_ALBUMS      SPOTIFY_API_BASE "/artists/%s/albums"
#define ENDPOINT_ARTIST_TOP_TRACKS  SPOTIFY_API_BASE "/artists/%s/top-tracks"
#define ENDPOINT_ARTIST_RELATED     SPOTIFY_API_BASE "/artists/%s/related-artists"

// Track endpoints
#define ENDPOINT_TRACK              SPOTIFY_API_BASE "/tracks/%s"
#define ENDPOINT_TRACKS             SPOTIFY_API_BASE "/tracks"
#define ENDPOINT_USER_TRACKS        SPOTIFY_API_BASE "/me/tracks"
#define ENDPOINT_CHECK_TRACKS       SPOTIFY_API_BASE "/me/tracks/contains"
#define ENDPOINT_AUDIO_FEATURES     SPOTIFY_API_BASE "/audio-features/%s"
#define ENDPOINT_AUDIO_FEATURES_BATCH SPOTIFY_API_BASE "/audio-features"
#define ENDPOINT_AUDIO_ANALYSIS     SPOTIFY_API_BASE "/audio-analysis/%s"

// Player endpoints
#define ENDPOINT_PLAYER             SPOTIFY_API_BASE "/me/player"
#define ENDPOINT_PLAYER_PLAY        SPOTIFY_API_BASE "/me/player/play"
#define ENDPOINT_PLAYER_PAUSE       SPOTIFY_API_BASE "/me/player/pause"
#define ENDPOINT_PLAYER_NEXT        SPOTIFY_API_BASE "/me/player/next"
#define ENDPOINT_PLAYER_PREVIOUS    SPOTIFY_API_BASE "/me/player/previous"
#define ENDPOINT_PLAYER_SEEK        SPOTIFY_API_BASE "/me/player/seek"
#define ENDPOINT_PLAYER_REPEAT      SPOTIFY_API_BASE "/me/player/repeat"
#define ENDPOINT_PLAYER_VOLUME      SPOTIFY_API_BASE "/me/player/volume"
#define ENDPOINT_PLAYER_SHUFFLE     SPOTIFY_API_BASE "/me/player/shuffle"
#define ENDPOINT_PLAYER_DEVICES     SPOTIFY_API_BASE "/me/player/devices"
#define ENDPOINT_PLAYER_QUEUE       SPOTIFY_API_BASE "/me/player/queue"
#define ENDPOINT_CURRENTLY_PLAYING  SPOTIFY_API_BASE "/me/player/currently-playing"
#define ENDPOINT_RECENTLY_PLAYED    SPOTIFY_API_BASE "/me/player/recently-played"

// Playlist endpoints
#define ENDPOINT_PLAYLIST           SPOTIFY_API_BASE "/playlists/%s"
#define ENDPOINT_PLAYLIST_TRACKS    SPOTIFY_API_BASE "/playlists/%s/tracks"
#define ENDPOINT_USER_PLAYLISTS     SPOTIFY_API_BASE "/me/playlists"
#define ENDPOINT_USER_PLAYLISTS_BY_ID SPOTIFY_API_BASE "/users/%s/playlists"
#define ENDPOINT_PLAYLIST_FOLLOWERS SPOTIFY_API_BASE "/playlists/%s/followers"

// Search endpoint
#define ENDPOINT_SEARCH             SPOTIFY_API_BASE "/search"

// User endpoints
#define ENDPOINT_CURRENT_USER       SPOTIFY_API_BASE "/me"
#define ENDPOINT_USER               SPOTIFY_API_BASE "/users/%s"
#define ENDPOINT_USER_TOP_ITEMS     SPOTIFY_API_BASE "/me/top/%s"
#define ENDPOINT_USER_FOLLOWING     SPOTIFY_API_BASE "/me/following"

// Category endpoints
#define ENDPOINT_CATEGORIES         SPOTIFY_API_BASE "/browse/categories"
#define ENDPOINT_CATEGORY           SPOTIFY_API_BASE "/browse/categories/%s"
#define ENDPOINT_CATEGORY_PLAYLISTS SPOTIFY_API_BASE "/browse/categories/%s/playlists"

// Recommendations
#define ENDPOINT_RECOMMENDATIONS    SPOTIFY_API_BASE "/recommendations"
#define ENDPOINT_RECOMMENDATION_GENRES SPOTIFY_API_BASE "/recommendations/available-genre-seeds"

// API Limits
#define SPOTIFY_MAX_LIMIT_TRACKS        50
#define SPOTIFY_MAX_LIMIT_ALBUMS        50
#define SPOTIFY_MAX_LIMIT_ARTISTS       50
#define SPOTIFY_MAX_LIMIT_PLAYLISTS     50
#define SPOTIFY_MAX_LIMIT_QUEUE         50
#define SPOTIFY_MAX_LIMIT_SEARCH        50
#define SPOTIFY_MAX_LIMIT_RECOMMENDATIONS 100

#define SPOTIFY_MAX_BATCH_TRACKS        100
#define SPOTIFY_MAX_BATCH_ALBUMS        50
#define SPOTIFY_MAX_BATCH_ARTISTS       50
#define SPOTIFY_MAX_SEEDS               5

#endif // SPOTIFY_ENDPOINTS_H
