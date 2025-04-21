#include "Solitaire.h"
#include <iostream>
#include <raylib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// Forward declare the global constants
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

// Global game instance
Solitaire* game = nullptr;

void UpdateDrawFrame(void) {
    if (!game) return;
    
    game->update();
    
    BeginDrawing();
    ClearBackground(BLACK);
    game->draw();
    EndDrawing();
}

int main(void) {
    // Initialize window with base dimensions first
    InitWindow(BASE_WINDOW_WIDTH, BASE_WINDOW_HEIGHT, "Solitaire");
    
    if (!IsWindowReady()) {
        return -1;
    }

    // Force a frame to ensure OpenGL context is properly initialized
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

    // Set target FPS
    SetTargetFPS(60);

    SetExitKey(KEY_NULL);

    // Create game instance
    try {
        game = new Solitaire();
    } catch (const std::exception& e) {
        CloseWindow();
        return -1;
    }

#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    // Cleanup
    if (game) {
        delete game;
        game = nullptr;
    }
    
    CloseWindow();
    return 0;
}