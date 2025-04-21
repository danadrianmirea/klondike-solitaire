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
#if DEBUG == 1
    std::cout << "Starting application initialization..." << std::endl;
    std::cout << "Window dimensions: " << BASE_WINDOW_WIDTH << "x" << BASE_WINDOW_HEIGHT << std::endl;
#endif

    // Initialize window with base dimensions first
#if DEBUG == 1
    std::cout << "Initializing window with base dimensions..." << std::endl;
#endif
    InitWindow(BASE_WINDOW_WIDTH, BASE_WINDOW_HEIGHT, "Solitaire");
    
    if (!IsWindowReady()) {
#if DEBUG == 1
        std::cerr << "Failed to initialize window!" << std::endl;
#endif
        return -1;
    }

#if DEBUG == 1
    std::cout << "Window initialized successfully" << std::endl;
    std::cout << "Initial window size: " << GetScreenWidth() << "x" << GetScreenHeight() << std::endl;
#endif

    // Set up OpenGL context
#if DEBUG == 1
    std::cout << "Setting up OpenGL context..." << std::endl;
#endif

    // Force a frame to ensure OpenGL context is properly initialized
    BeginDrawing();
    ClearBackground(BLACK);
    EndDrawing();

#if DEBUG == 1
    std::cout << "OpenGL context verified" << std::endl;
#endif

    // Set target FPS
#if DEBUG == 1
    std::cout << "Setting target FPS..." << std::endl;
#endif
    SetTargetFPS(144);

    SetExitKey(KEY_NULL);

    // Create game instance
#if DEBUG == 1
    std::cout << "Creating game instance..." << std::endl;
#endif
    try {
        game = new Solitaire();
#if DEBUG == 1
        std::cout << "Game instance created successfully" << std::endl;
#endif
    } catch (const std::exception& e) {
#if DEBUG == 1
        std::cerr << "Failed to create game instance: " << e.what() << std::endl;
#endif
        CloseWindow();
        return -1;
    }

#ifdef __EMSCRIPTEN__
#if DEBUG == 1
    std::cout << "Setting up Emscripten main loop..." << std::endl;
#endif
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
#if DEBUG == 1
    std::cout << "Starting native main loop..." << std::endl;
#endif
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    // Cleanup
#if DEBUG == 1
    std::cout << "Starting cleanup..." << std::endl;
#endif
    if (game) {
        delete game;
        game = nullptr;
    }
    
    CloseWindow();
#if DEBUG == 1
    std::cout << "Cleanup completed, exiting..." << std::endl;
#endif
    return 0;
}