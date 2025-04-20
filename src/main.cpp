#include <raylib.h>
#include "Solitaire.h"

// Forward declare the global constants
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

int main() {
    // Initialize window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Solitaire");
    SetTargetFPS(60);

    // Disable ESC as exit key
    SetExitKey(0);

    // Create game instance
    Solitaire game;

    // Main game loop
    while (!WindowShouldClose()) {
        // Update game state
        game.update();

        // Draw
        BeginDrawing();
        game.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
};