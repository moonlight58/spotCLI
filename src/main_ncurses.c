/* 
 * Example main file using ncurses UI
 * Replace or modify your existing main.c with this
 */

#include "auth.h"
#include "dotenv.h"
#include "render_ui/ncurses_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void print_usage(const char *prog_name);

void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  -c, --cli         Use command-line interface (original)\n");
    printf("  -n, --ncurses     Use ncurses TUI (default)\n");
    printf("  -h, --help        Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s              # Start ncurses TUI\n", prog_name);
    printf("  %s --ncurses    # Start ncurses TUI\n", prog_name);
    printf("  %s --cli        # Use original CLI\n\n", prog_name);
}

int main(int argc, char *argv[]) {
    // Load environment variables
    load_dotenv(".env");

    SpotifyToken token;

    // Authenticate first
    if (!spotify_get_access_token(&token)) {
        fprintf(stderr, "Failed to get access token.\n");
        return 1;
    }

    // Parse command line options
    int use_ncurses = 1;  // Default to ncurses
    int opt;

    static struct option long_options[] = {
        {"cli",     no_argument, 0, 'c'},
        {"ncurses", no_argument, 0, 'n'},
        {"help",    no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "cnh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                use_ncurses = 0;
                break;
            case 'n':
                use_ncurses = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    printf("✅ Authenticated successfully!\n");

    if (use_ncurses) {
        // Initialize ncurses UI
        UIState *ui = ui_init();
        if (!ui) {
            fprintf(stderr, "Failed to initialize ncurses UI\n");
            return 1;
        }
        refresh();
        // Run the UI
        ui_handle_input(ui, &token);

        // Cleanup
        ui_cleanup(ui);
    } else {
        // Use original CLI interface
        printf("CLI mode not implemented in this example\n");
        printf("Use your original interactive_mode() function here\n");
        // interactive_mode(&token);
    }

    return 0;
}
