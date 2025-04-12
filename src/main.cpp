#include <raylib.h>
#include "Solitaire.h"

// Forward declare the global constants
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

int main() {
    // Initialize window
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Solitaire");
    SetTargetFPS(60);

    // Create game instance
    Solitaire game;

    // Main game loop
    while (!WindowShouldClose()) {
        // Handle input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            game.handleMouseDown(GetMousePosition());
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            game.handleMouseUp(GetMousePosition());
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            game.handleDoubleClick(GetMousePosition());
        }

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