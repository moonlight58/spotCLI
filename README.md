# findSpot

A lightweight CLI tool to search Spotify music and add tracks to your liked songs without opening the Spotify app or website.

## Features

- 🔍 Search for tracks
- ❤️ Save tracks to your library
- 📋 View your saved tracks
- 🎵 Interactive and command-line modes
- 🔐 Automatic OAuth authentication
- 🔄 Token refresh handling

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
git clone https://github.com/yourusername/findSpot.git
cd findSpot

# Build using Make
make

# Install system-wide (optional)
sudo make install
```

Or build manually:
```bash
gcc src/*.c -o findSpot -lcurl -ljson-c
sudo mv findSpot /usr/local/bin/
```

## Setup

### 1. Create Spotify Application

1. Go to [Spotify Developer Dashboard](https://developer.spotify.com/dashboard)
2. Click "Create app"
3. Fill in the details:
   - **App name**: findSpot (or any name you like)
   - **App description**: CLI music search tool
   - **Redirect URI**: `http://localhost:8888/callback`
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
./findSpot
```

The app will:
1. Display an authorization URL
2. Open a temporary local server on port 8888
3. Wait for you to authorize the app in your browser
4. Automatically save your tokens to `~/.config/findSpot/token.json`

## Usage

### Interactive Mode (default)

```bash
findSpot
# or
findSpot -i
```

Menu options:
- `1` - Search for tracks
- `2` - View saved tracks
- `3` - Exit

### Command Line Mode

#### Search for tracks
```bash
findSpot "PTSMR"
findSpot "Bohemian Rhapsody"
findSpot "tyler, the creator EARFQUAKE"
```

#### List saved tracks
```bash
findSpot --list
# or
findSpot -l
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
# Search for a track and save it
findSpot "Glimpse of Us"

# Search with artist name
findSpot "Daft Punk Get Lucky"

# View your library (first 20 tracks)
findSpot --list

# Interactive menu
findSpot -i
```

## Project Structure

```
findSpot/
├── src/
│   ├── api.c              # Spotify API calls
│   ├── api.h
│   ├── auth.c             # OAuth authentication
│   ├── auth.h
│   ├── callback_server.c  # Local HTTP server for OAuth callback
│   ├── dotenv.c           # Environment variable loader
│   ├── dotenv.h
│   └── main.c             # Main program and CLI interface
├── Makefile
├── .env                   # Your credentials (not in git!)
├── .gitignore
└── README.md
```

## Configuration Files

```
~/.config/findSpot/
└── token.json  # Stored authentication tokens (auto-generated)
```

To log out and clear tokens:
```bash
make logout
# or manually
rm ~/.config/findSpot/token.json
```

## Troubleshooting

### "No tracks found" for valid searches

1. Check your token is valid:
   ```bash
   cat ~/.config/findSpot/token.json
   ```

2. Delete token and re-authenticate:
   ```bash
   make logout
   ./findSpot
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

## Roadmap

- [ ] Artist search
- [ ] Album search
- [ ] Playlist management
- [ ] Audio preview player
- [ ] Create custom playlists
- [ ] Remove tracks from library
- [ ] Search filters (year, genre, popularity)
- [ ] Batch operations
- [ ] Configuration file for preferences
- [ ] Shell completions (bash/zsh/fish)

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
