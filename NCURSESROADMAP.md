# SpotCLI - Roadmap vers TUI (ncurses)

## Phase 1: Architecture modulaire (ACTUEL)
- [x] Séparer la logique métier de l'interface
- [x] Créer une structure de menu générique
- [x] Implémenter toutes les fonctionnalités API
- [ ] Finaliser les 4 endpoints manquants
- [ ] Tests de toutes les fonctionnalités en mode CLI

## Phase 2: Couche d'abstraction UI
```c
// include/ui.h - Interface agnostique

typedef struct {
    void (*init)(void);
    void (*cleanup)(void);
    void (*clear)(void);
    void (*display_menu)(Menu *menu);
    void (*display_tracks)(SpotifyTrack *tracks, int count);
    void (*display_player)(SpotifyPlayerState *state);
    void (*get_input)(char *buffer, size_t size);
    int (*get_choice)(Menu *menu);
} UIBackend;

// Deux implémentations:
extern UIBackend CLI_Backend;  // Mode texte actuel
extern UIBackend TUI_Backend;  // Futur ncurses
```

## Phase 3: Composants ncurses de base

### 3.1 Layout système
```
┌─────────────────────────────────────────────┐
│  Header (titre + info utilisateur)         │
├─────────────┬───────────────────────────────┤
│             │                               │
│   Sidebar   │     Main Content Area        │
│   (menu)    │     (listes, formulaires)    │
│             │                               │
│             │                               │
├─────────────┴───────────────────────────────┤
│  Player Bar (now playing + controls)        │
├─────────────────────────────────────────────┤
│  Status Bar (messages, shortcuts)           │
└─────────────────────────────────────────────┘
```

### 3.2 Widgets à créer
- Window manager (gérer les fenêtres)
- List view (afficher tracks/artists/albums)
- Form view (création playlist, recherche)
- Player widget (affichage + contrôles)
- Progress bar (pour le player)
- Modal dialog (confirmations)
- Input field (recherche, saisie texte)

## Phase 4: Fonctionnalités TUI

### Navigation
- `↑/↓` ou `j/k`: Navigation verticale
- `←/→` ou `h/l`: Navigation horizontale / changement de panneau
- `Tab`: Cycle entre les sections
- `Enter`: Sélectionner
- `/`: Recherche rapide
- `q` ou `Esc`: Retour/Quitter
- `?`: Aide contextuelle

### Raccourcis globaux
- `Space`: Play/Pause
- `n`: Next track
- `p`: Previous track
- `s`: Shuffle toggle
- `r`: Repeat cycle
- `+/-`: Volume
- `,/.`: Seek backward/forward

### Views principales
1. **Search View**
   - Input field en haut
   - Tabs: Tracks / Artists / Albums / Playlists
   - Liste des résultats
   - Preview panel (détails sur sélection)

2. **Library View**
   - Tabs: Tracks / Albums / Playlists
   - Tri et filtres
   - Actions rapides

3. **Playlist View**
   - Liste des tracks
   - Drag & drop (simulé)
   - Edit details
   - Add/remove tracks

4. **Player View**
   - Album art (ASCII art)
   - Track info détaillée
   - Progress bar
   - Queue visible
   - Lyrics (futur)

## Phase 5: Fonctionnalités avancées

### Mouse support
```c
// ncurses supporte la souris
mousemask(ALL_MOUSE_EVENTS, NULL);
MEVENT event;
if (getmouse(&event) == OK) {
    // Handle click at event.x, event.y
}
```

### Colors & themes
```c
start_color();
init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Spotify green
init_pair(2, COLOR_WHITE, COLOR_BLACK);
init_pair(3, COLOR_BLACK, COLOR_GREEN);  // Selected
```

### Async updates
- Thread séparé pour les requêtes API
- Refresh automatique du player state
- Notifications non-bloquantes

## Phase 6: Polish & optimisations

- Cache des données (éviter requêtes répétées)
- Lazy loading (pagination intelligente)
- Animations (transitions smooth)
- Configuration via fichier (~/.config/spotCLI/config.toml)
- Themes personnalisables

## Structure de fichiers proposée

```
src/
├── cli/
│   ├── cli_main.c          # Mode CLI actuel
│   └── cli_ui.c            # Backend CLI
├── tui/
│   ├── tui_main.c          # Point d'entrée TUI
│   ├── tui_ui.c            # Backend ncurses
│   ├── tui_layout.c        # Gestion des fenêtres
│   ├── tui_widgets.c       # Composants réutilisables
│   ├── tui_views.c         # Vues principales
│   └── tui_input.c         # Gestion clavier/souris
├── core/
│   ├── menu.c              # Logique menu (partagée)
│   ├── state.c             # Gestion état application
│   └── config.c            # Configuration
└── spotify/                # API Spotify (inchangé)
```

## Makefile targets

```makefile
# Compiler les deux versions
all: spotCLI spotTUI

spotCLI: $(CLI_OBJECTS) $(CORE_OBJECTS) $(SPOTIFY_OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

spotTUI: $(TUI_OBJECTS) $(CORE_OBJECTS) $(SPOTIFY_OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) -lncursesw

# Ou un seul binaire avec option
spotCLI: $(ALL_OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS) -lncursesw
	
# Usage: ./spotCLI --tui
```

## Librairies alternatives à considérer

- **ncurses**: Standard, bien supporté
- **CDK (Curses Development Kit)**: Widgets haut niveau
- **notcurses**: Moderne, support multimedia
- **termbox**: Minimaliste, portable

## Inspiration

- **spotify-tui** (Rust): Interface existante excellente
- **cmus**: Player audio TUI référence
- **ranger**: File manager avec preview
- **htop**: Interface système claire

## Priorités

1. **Must-have**:
   - Navigation intuitive
   - Recherche fonctionnelle
   - Contrôle player complet
   - Gestion playlists

2. **Nice-to-have**:
   - Visualizations audio
   - Lyrics sync
   - Mouse support
   - Custom themes

3. **Future**:
   - Plugins system
   - Social features
   - Local file integration
