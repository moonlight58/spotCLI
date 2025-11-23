# spotCLI

A lightweight CLI tool to search Spotify music and add tracks to your liked songs without opening the Spotify app or website.

## Features

- Search for tracks
- Save tracks to your library
- View your saved tracks
- Interactive and command-line modes
- Automatic OAuth authentication
- Token refresh handling

## Installation

### Prerequisites

#### Debian/Ubuntu
```bash
sudo apt install libcurl4-openssl-dev libjson-c-dev build-essential
```

#### Fedora/RHEL
```bash
sudo dnf install libcurl-devel json-c-devel gcc make
```

#### Arch Linux
```bash
sudo pacman -S curl json-c base-devel
```

#### macOS
```bash
brew install curl json-c
```

### Build & Install

```bash
# Clone or download the repository
git clone https://github.com/yourusername/spotCLI.git
cd spotCLI

# Build using Make
make

# Install system-wide (optional)
sudo make install
```

Or build manually:
```bash
gcc src/*.c -o spotCLI -lcurl -ljson-c
sudo mv spotCLI /usr/local/bin/
```

## Setup

### 1. Create Spotify Application

1. Go to [Spotify Developer Dashboard](https://developer.spotify.com/dashboard)
2. Click "Create app"
3. Fill in the details:
   - **App name**: spotCLI (or any name you like)
   - **App description**: CLI music search tool
   - **Redirect URI**: `http://127.0.0.1:8888/callback` since the webAPI doesn't support localhost anymore
4. Save your app and note the **Client ID** and **Client Secret**

### 2. Configure Environment Variables

Create a `.env` file in the project directory:

```bash
cat > .env << EOF
CLIENT_ID=your_spotify_client_id_here
CLIENT_SECRET=your_spotify_client_secret_here
REDIRECT_URI=http://localhost:8888/callback
EOF
```

**Important**: Never commit your `.env` file to version control!

### 3. First Run

```bash
./spotCLI
```

The app will:
1. Display an authorization URL
2. Open a temporary local server on port 8888
3. Wait for you to authorize the app in your browser
4. Automatically save your tokens to `~/.config/spotCLI/token.json`

## Usage

### Interactive Mode (default)

```bash
spotCLI
# or
spotCLI -i
```

Menu options:
- `1` - Exit
- `2` - Users Options
- `3` - View saved tracks
- `4` - Search for artists
- `5` - Search for tracks

### Command Line Mode

#### Search for tracks
```bash
spotCLI "PTSMR"
spotCLI "Bohemian Rhapsody"
spotCLI "tyler, the creator EARFQUAKE"
```

#### List saved tracks
```bash
spotCLI --list
# or
spotCLI -l
```

## Make Commands

```bash
make          # Build the project
make run      # Build and run in interactive mode
make clean    # Remove build files
make rebuild  # Clean and rebuild
make debug    # Build with debug symbols
make install  # Install to /usr/local/bin (requires sudo)
make uninstall # Remove from system
make logout   # Remove authentication token
make help     # Show all available commands
```

## Options

| Option | Short | Description |
|--------|-------|-------------|
| `--track` | `-t` | Search for tracks (default) |
| `--artist` | `-a` | Search for artists |
| `--album` | `-A` | Search for albums |
| `--playlist` | `-p` | Search for playlists |
| `--user` | `-u` | Search for users (coming soon) |
| `--audiobook` | `-b` | Search for audiobooks (coming soon) |
| `--list` | `-l` | List your saved tracks |
| `--interactive` | `-i` | Start interactive mode |
| `--help` | `-h` | Show help message |

## Examples

```bash
# Search for a track and save it
spotCLI "Glimpse of Us"

# Search with artist name
spotCLI "Daft Punk Get Lucky"

# View your library (first 20 tracks)
spotCLI --list

# Interactive menu
spotCLI -i
```

## Project Structure

```
spotCLI/
├── src/
│   ├── spotify/
│   │   ├── spotify_utils.c
│   │   ├── spotify_search.c
│   │   ├── spotify_playlist.c
│   │   ├── spotify_player.c
│   │   ├── spotify_parsers.c
│   │   ├── spotify_library.c
│   │   ├── spotify_http.c
│   │   └── spotify_auth.c
│   ├── main.c
│   ├── dotenv.c
│   ├── callback_server.c
│   └── auth.c
├── include/
│   ├── spotify/
│   │   ├── spotify_search.h
│   │   ├── spotify_playlist.h
│   │   ├── spotify_player.h
│   │   ├── spotify_library.h
│   │   ├── spotify_internal.h
│   │   ├── spotify_auth.h
│   │   └── spotify_api.h
│   ├── dotenv.h
│   ├── auth.h
│   └── api.h
├── spotCLI*
├── README.md
├── Makefile
└── LICENSE
```

## Configuration Files

```
~/.config/spotCLI/
└── token.json  # Stored authentication tokens (auto-generated)
```

To log out and clear tokens:
```bash
make logout
# or manually
rm ~/.config/spotCLI/token.json
```

## Troubleshooting

### "No tracks found" for valid searches

1. Check your token is valid:
   ```bash
   cat ~/.config/spotCLI/token.json
   ```

2. Delete token and re-authenticate:
   ```bash
   make logout
   ./spotCLI
   ```

3. Verify your `.env` file has correct credentials

### "Failed to start callback server"

Port 8888 might be in use. Check with:
```bash
lsof -i :8888
```

### Compilation errors

Make sure all dependencies are installed:
```bash
# Check if libraries are available
pkg-config --libs libcurl json-c
```

## API Scopes

The app requests the following Spotify scopes:
- `user-library-read` - View your saved tracks
- `user-library-modify` - Save tracks to your library
- `playlist-modify-public`
- `playlist-modify-private`
- `user-read-playback-state`
- `user-modify-playback-state`

## Roadmap

- [x] Artist search
- [x] Tracks from an artist search
- [x] Album search
- [ ] Playlist management
  - [x] Create Playlist
  - [x] Delete Playlist
  - [x] Add track into Playlist
  - [x] Remove track from Playlist
  - [x] Change info about Playlist (title/description/visibility/collaborative)
- [ ] Player functionnality
  - [x] Audio preview
  - [x] Start/Resume
  - [x] Pause
  - [x] Toggle Shuffle
  - [x] Skip to Next/Previous
  - [x] Set Playback Volume
  - [x] User's Queue Tracks
  - [x] Change playback device
- [x] Create custom playlists
- [x] Remove tracks from library
- [ ] Album search & detailed album info (tracks in album)
- [ ] User profile info (followers, public playlists)
- [ ] Recommendations songs (get recommendations based on seeds)
- [ ] Audio features (tempo, energy, danceability of tracks)
- [ ] Recently played (get user's listening history)
- [ ] Batch operations
- [ ] Configuration file for preferences
- [ ] Shell completions (bash/zsh/fish)

---

**ncurses would be ideal** for rendering these with keyboard navigation and mouse support!

---

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

MIT License - see LICENSE file for details

## Acknowledgments

- Built with [libcurl](https://curl.se/libcurl/) for HTTP requests
- JSON parsing with [json-c](https://github.com/json-c/json-c)
- [Spotify Web API](https://developer.spotify.com/documentation/web-api/)
