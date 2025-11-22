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

## Potentials layout

### spotCLI TUI Layout Examples

#### Layout 1: Classic Three-Column Dashboard

```
╭──spotCLI v1.0 - Now Playing: "Glimpse of Us" by Joji────────────────────────────╮
│                                                                                 │
│ ╭──────────────────────────┬──────────────────────────┬──────────────────────╮  │
│ │ SEARCH                   │ RESULTS                  │ NOW PLAYING          │  │
│ ├──────────────────────────┼──────────────────────────┼──────────────────────┤  │
│ │ Mode: [Track]            │  ✓ Glimpse of Us         │  🎵 Glimpse of Us    │  │
│ │       [Artist]           │    Joji                  │     Joji             │  │
│ │       [Playlist]         │                          │                      │  │
│ │                          │  [ ] PTSMR               │  Album: Nectar       │  │
│ │ Query:                   │    Tyler, The Creator    │  Duration: 3:56      │  │
│ │ ╭───────────────────────╮│                          │                      │  │
│ │ │ tyler the creator ___ ││  [ ] Bohemian Rhapsody   │  Progress: ━━━●───── │  │
│ │ ╰───────────────────────╯│    Queen                 │  0:45 / 3:56         │  │
│ │                          │                          │                      │  │
│ │ [Search]  [Clear]        │  [ ] Blinding Lights     │  Volume: ███░░░░░░░  │  │
│ │                          │    The Weeknd            │  Device: Speakers    │  │
│ │ QUICK ACTIONS            │                          │                      │  │
│ │ [Save Track] [Add Queue] │                          │  [⏮] [⏸] [⏭] [🔀]    │  │
│ │                          │  [ ] Shape of You        │                      │  │
│ │ LIBRARY                  │    Ed Sheeran            │  [Queue] [History]   │  │
│ │ [Liked Songs]            │                          │                      │  │
│ │ [Playlists]              │ ↓ scroll for more        │                      │  │
│ │ [Followed Artists]       │                          │                      │  │
│ │                          │                          │                      │  │
│ ╰──────────────────────────┴──────────────────────────┴──────────────────────╯  │
│                                                                                 │
├─────────────────────────────────────────────────────────────────────────────────┤
│ [q] Quit  [?] Help  [s] Search  [l] Library  [p] Playlists  [d] Devices         │
╰─────────────────────────────────────────────────────────────────────────────────╯
```

---

#### Layout 2: Compact Single Panel with Sidebar

```
╭──────────────────────────────────────────────────────────────────╮
│ spotCLI                                                          │
├──────┬───────────────────────────────────────────────────────────┤
│      │ SEARCH RESULTS                                            │
│ MENU │                                                           │
│      │ Query: tyler the creator          [CLEAR]                 │
│ ╭──╮ │                                                           │
│ │🔍│ │ ✓ 1. Tyler, The Creator (Artist)                          │
│ ├──┤ │    Followers: 50.2M  |  Popularity: 95/100                │
│ │♥ │ │                                                           │
│ ├──┤ │ [ ] 2. IGOR (Album)                                       │
│ │🎵│ │      6 tracks  |  2019                                    │
│ ├──┤ │                                                           │
│ │➕│ │ [ ] 3. EARFQUAKE (Track)                                  │
│ ├──┤ │      Tyler, The Creator  |  3:34                          │
│ │⚙️│ │                                                           │
│ ├──┤ │ [ ] 4. See You Again (Track)                              │
│ │⏮ │ │      Tyler, The Creator ft. Kali U.  |  2:47              │
│ ├──┤ │                                                           │
│ │⏸ │ │ [ ] 5. Lumberjack (Track)                                 │
│ ├──┤ │      Tyler, The Creator  |  2:23                          │
│ │⏭ │ │                                                           │
│ ├──┤ │ [ ] 6. Yonkers (Track)                                    │
│ │🔀│ │     Tyler, The Creator  |  3:12                           │
│ ├──┤ │                                                           │
│ │📋│ │ ↓ scroll: ↑↓  select: Enter  multi: Space  back: Esc      │
│ ├──┤ │                                                           │
│ │❌│ │                                                           │
│ ╰──╯ │                                                           │
│      │                                                           │
│      │ [Save] [Add to Queue] [Add to Playlist]                   │
│      │                                                           │
╰──────┴───────────────────────────────────────────────────────────╯
```

---

#### Layout 3: Playlist Manager View

```
╭─────────────────────────────────────────────────────────────────────╮
│ spotCLI - Playlist Manager                                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ ╭───────────────────────────┬───────────────────────────────────╮   │
│ │ YOUR PLAYLISTS            │ PLAYLIST DETAILS                  │   │
│ ├───────────────────────────┼───────────────────────────────────┤   │
│ │                           │                                   │   │
│ │ [NEW]                     │ My Favorites                      │   │
│ │                           │ Owner: You                        │   │
│ │ [ ] My Favorites          │ Tracks: 47                        │   │
│ │     47 tracks             │ Public: Yes  |  Collaborative: No │   │
│ │                           │                                   │   │
│ │ [x] Summer Vibes          │ Description:                      │   │
│ │     23 tracks             │ Songs that make me happy          │   │
│ │                           │                                   │   │
│ │ [ ] Workout Mix           │ ╭─────────────────────────────╮   │   │
│ │     15 tracks             │ │ TRACKS IN PLAYLIST          │   │   │
│ │                           │ ├─────────────────────────────┤   │   │
│ │ [ ] Chill Beats           │ │ ✓ 1. Glimpse of Us          │   │   │
│ │     31 tracks             │ │    Joji  |  3:56            │   │   │
│ │                           │ │                             │   │   │
│ │ [ ] Late Night Study      │ │ [ ] 2. Blinding Lights      │   │   │
│ │     8 tracks              │ │    The Weeknd  |  3:20      │   │   │
│ │                           │ │                             │   │   │
│ │                           │ │ [ ] 3. Shape of You         │   │   │
│ │ [DELETE] [EDIT] [SHUFFLE] │ │    Ed Sheeran  |  3:53      │   │   │
│ │                           │ │                             │   │   │
│ │ ↑↓ select  Space: multi   │ │ [ ] 4. As It Was            │   │   │
│ │ Enter: detail  Del: remove│ │    Harry Styles  |  2:42    │   │   │
│ │                           │ │                             │   │   │
│ │                           │ │ ↓ scroll for more           │   │   │
│ │                           │ ╰─────────────────────────────╯   │   │
│ │                           │                                   │   │
│ ╰───────────────────────────┴───────────────────────────────────╯   │
│                                                                     │
├─────────────────────────────────────────────────────────────────────┤
│ [a] Add Track  [r] Remove  [e] Edit  [n] New  [q] Quit              │
╰─────────────────────────────────────────────────────────────────────╯
```

---

#### Layout 4: Queue & Now Playing Focus

```
╭────────────────────────────────────────────────────────────────────────╮
│ spotCLI - Now Playing                                       [Vol: 70%] │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│                          🎵 NOW PLAYING 🎵                             │
│                                                                        │
│                 ╭───────────────────────────────╮                      │
│                 │                               │                      │
│                 │      Glimpse of Us            │                      │
│                 │      by Joji                  │                      │
│                 │                               │                      │
│                 │      Album: Nectar            │                      │
│                 │      Duration: 3:56           │                      │
│                 │                               │                      │
│                 ╰───────────────────────────────╯                      │
│                                                                        │
│  Progress: ══════════●─────────────────────────────                    │
│            0:45 / 3:56                                                 │
│                                                                        │
│  [⏮ Previous]  [⏸ Pause]  [⏭ Next]  [🔀 Shuffle]  [🔁 Repeat: off]     │
│                                                                        │
│ ╭──────────────────────────────────────────────────────────────────╮   │
│ │ UP NEXT - QUEUE (15 songs)                                       │   │
│ ├──────────────────────────────────────────────────────────────────┤   │
│ │                                                                  │   │
│ │ 1. Blinding Lights          The Weeknd            3:20  [+] [✕]  │   │
│ │ 2. Shape of You             Ed Sheeran            3:53  [+] [✕]  │   │
│ │ 3. As It Was                Harry Styles          2:42  [+] [✕]  │   │
│ │ 4. Anti-Hero                Taylor Swift          3:21  [+] [✕]  │   │
│ │ 5. Heat Waves               Glass Animals         3:59  [+] [✕]  │   │
│ │                                                                  │   │
│ │ ↓ 10 more items                                                  │   │
│ │                                                                  │   │
│ ╰──────────────────────────────────────────────────────────────────╯   │
│                                                                        │
├────────────────────────────────────────────────────────────────────────┤
│ [a] Add to Queue  [c] Clear Queue  [s] Save to Playlist  [q] Quit      │
╰────────────────────────────────────────────────────────────────────────╯
```

---

#### Layout 5: Split View - Search & Library

```
╭─────────────────────────────────────────────────────────────────────────────╮
│ spotCLI                                                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│ ╭─SEARCH───────────────────────────┬─LIBRARY──────────────────────────────╮ │
│ │ Type:  ◉ Track  ○ Artist         │ ╭──────────────────────────────────╮ │ │
│ │        ○ Album  ○ Playlist       │ │ LIKED SONGS (247 songs)          │ │ │
│ │                                  │ ├──────────────────────────────────┤ │ │
│ │ Search: ┌──────────────────────┐ │ │ 1. Glimpse of Us - Joji      3:56│ │ │
│ │         │ joji _______________ │ │ │ 2. Blinding Lights - The...  3:20│ │ │
│ │         └──────────────────────┘ │ │ 3. Shape of You - Ed Shera...3:53│ │ │
│ │                                  │ │ 4. As It Was - Harry Style...2:42│ │ │
│ │ ╭──────────────────────────────╮ │ │                                  │ │ │
│ │ │ RESULTS (8 tracks)           │ │ │ [Save] [Add Queue] [Playlist]    │ │ │
│ │ ├──────────────────────────────┤ │ ╰──────────────────────────────────╯ │ │
│ │ │  ✓  1. Glimpse of Us    3:56 │ │                                      │ │
│ │ │ [ ] 2. Slow It Down     2:47 │ │ ╭──────────────────────────────────╮ │ │
│ │ │ [ ] 3. Will He          2:33 │ │ │ YOUR PLAYLISTS                   │ │ │
│ │ │ [ ] 4. Lemon Tree       2:41 │ │ ├──────────────────────────────────┤ │ │
│ │ │ [ ] 5. Drunk            2:59 │ │ │ My Favorites (47)                │ │ │
│ │ │ [ ] 6. Test Drive       2:38 │ │ │ Summer Vibes (23)                │ │ │
│ │ │ [ ] 7. Falling          2:41 │ │ │ Workout Mix (15)                 │ │ │
│ │ │ [ ] 8. 777              2:44 │ │ │ Late Night Study (8)             │ │ │
│ │ ╰──────────────────────────────╯ │ ╰──────────────────────────────────╯ │ │
│ │                                  │                                      │ │
│ │ Space: select  Tab: switch view  │                                      │ │
│ │                                  │                                      │ │
│ ╰──────────────────────────────────┴──────────────────────────────────────╯ │
├─────────────────────────────────────────────────────────────────────────────┤
│ [↹] Switch Panel  [s] Save  [+] Add Queue  [p] Playlist  [q] Quit           │
╰─────────────────────────────────────────────────────────────────────────────╯
```

---

#### Layout 6: Modal Dialog Example (for batch operations)

```
╭─────────────────────────────────────────────────────────────────────────────╮
│ spotCLI - Search Results                                                    │
│                                                                             │
│ (background content dimmed...)                                              │
│                                                                             │
│ ┌─────────────────────────────────────────────────────────────────────────┐ │
│ │                                                                         │ │
│ │  ╭───────────────────────────────────────────────────────────────────╮  │ │
│ │  │ ADD TO PLAYLIST - SELECT BATCH MODE                               │  │ │
│ │  ├───────────────────────────────────────────────────────────────────┤  │ │
│ │  │                                                                   │  │ │
│ │  │ You have selected 4 tracks:                                       │  │ │
│ │  │  ✓ Glimpse of Us                                                  │  │ │
│ │  │  ✓ Shape of You                                                   │  │ │
│ │  │  ✓ As It Was                                                      │  │ │
│ │  │  ✓ Blinding Lights                                                │  │ │
│ │  │                                                                   │  │ │
│ │  │ Add to which playlist?                                            │  │ │
│ │  │                                                                   │  │ │
│ │  │ [ ] My Favorites                                                  │  │ │
│ │  │ [x] Summer Vibes                  ← Current Selection             │  │ │
│ │  │ [ ] Workout Mix                                                   │  │ │
│ │  │ [ ] Late Night Study                                              │  │ │
│ │  │ [ ] Create New Playlist                                           │  │ │
│ │  │                                                                   │  │ │
│ │  │                                                                   │  │ │
│ │  │  [Cancel]              [Add 4 Tracks]                             │  │ │
│ │  │                                                                   │  │ │
│ │  ╰───────────────────────────────────────────────────────────────────╯  │ │
│ │                                                                         │ │
│ └─────────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
╰─────────────────────────────────────────────────────────────────────────────╯
```

---

#### Key Features in These Layouts

✓ **Multi-select with checkboxes** for batch operations  
✓ **Sidebar navigation** for quick menu access  
✓ **Real-time playback info** displayed prominently  
✓ **Keyboard shortcuts** shown at bottom  
✓ **Split views** for simultaneous library + search  
✓ **Modal dialogs** for confirmations and selections  
✓ **Visual hierarchy** with boxes and spacing  
✓ **Scroll indicators** (↓ more items, ↑↓ navigation)  

---

#### Recommended for Implementation

**Start with Layout 1 or 5** - they provide good balance of:
- Information density
- Navigation clarity
- Space for future features
- Batch operation support

**ncurses would be ideal** for rendering these with keyboard navigation and mouse support!

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
