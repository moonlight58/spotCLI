# findSpot

A CLI tool to search Spotify music and add tracks to your liked songs without opening the Spotify app or website.

## Features

- 🔍 Search for tracks
- ❤️ Save tracks to your library
- 📋 View your saved tracks
- 🎵 Interactive and command-line modes

## Installation

### Prerequisites

```bash
# Debian/Ubuntu
sudo apt install libcurl4-openssl-dev libjson-c-dev

# Fedora
sudo dnf install libcurl-devel json-c-devel

# macOS
brew install curl json-c
```

### Build

```bash
gcc src/*.c -o findSpot -lcurl -ljson-c
sudo mv findSpot /usr/local/bin/
```

## Usage

### Interactive Mode (default)

```bash
findSpot
# or
findSpot --interactive
```

### Search for Tracks

```bash
findSpot "PTSMR"
findSpot -t "Bohemian Rhapsody"
findSpot --track "tyler, the creator"
```

### List Saved Tracks

```bash
findSpot --list
findSpot -l
```

## Options

| Option | Short | Description |
|--------|-------|-------------|
| `--track` | `-t` | Search for tracks (default) |
| `--artist` | `-a` | Search for artists (coming soon) |
| `--album` | `-A` | Search for albums (coming soon) |
| `--playlist` | `-p` | Search for playlists (coming soon) |
| `--user` | `-u` | Search for users (coming soon) |
| `--audiobook` | `-b` | Search for audiobooks (coming soon) |
| `--list` | `-l` | List your saved tracks |
| `--interactive` | `-i` | Start interactive mode |
| `--help` | `-h` | Show help message |

## Examples

```bash
# Search for a track
findSpot "Glimpse of Us"

# Search with artist name
findSpot "tyler, the creator EARFQUAKE"

# View your library
findSpot --list

# Interactive menu
findSpot -i
```

## Authentication

On first run, findSpot will:
1. Open your browser for Spotify authentication
2. Save your access token to `~/.config/findSpot/token.json`
3. Automatically refresh tokens when needed

## Configuration

The app uses the following directory structure:

```
~/.config/findSpot/
└── token.json  # Stored authentication tokens
```

## Spotify API Setup

This app requires a Spotify Developer account. The current implementation uses embedded credentials, but for production use, you should:

1. Create an app at [Spotify Developer Dashboard](https://developer.spotify.com/dashboard)
2. Set redirect URI to `http://127.0.0.1:8888/callback`
3. Update `CLIENT_ID` and `CLIENT_SECRET` in `src/auth.h`

## Roadmap

- [ ] Artist search
- [ ] Album search
- [ ] Playlist management
- [ ] Audio preview player (premium account only [view here](https://developer.spotify.com/documentation/web-api/reference/start-a-users-playback))
- [ ] Create custom playlists
- [ ] Remove tracks from library
- [ ] Search filters (year, genre, etc.)

## License

MIT

## Contributing

Contributions welcome! Feel free to open issues or submit pull requests.
