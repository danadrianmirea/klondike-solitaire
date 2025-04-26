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

// Global render texture for the game
RenderTexture2D gameTarget;

// Function to lock orientation in landscape mode
void LockOrientation() {
#ifdef EMSCRIPTEN_BUILD
    EM_ASM(
        // Try to lock screen orientation to landscape
        function attemptLockOrientation() {
            if (screen.orientation && screen.orientation.lock) {
                screen.orientation.lock('landscape').catch(function(error) {
                    console.log('Orientation lock failed: ', error);
                });
            } else if (screen.lockOrientation) {
                // For older browsers
                screen.lockOrientation('landscape');
            } else if (screen.mozLockOrientation) {
                screen.mozLockOrientation('landscape');
            } else if (screen.msLockOrientation) {
                screen.msLockOrientation('landscape');
            }
        }
        
        // Initial lock attempt
        attemptLockOrientation();
        
        // Add event listeners for visibility and fullscreen changes
        if (!window.orientationHandler) {
            window.orientationHandler = true;
            
            // When page becomes visible again
            document.addEventListener('visibilitychange', function() {
                if (!document.hidden) {
                    setTimeout(attemptLockOrientation, 100);
                }
            });
            
            // When fullscreen changes
            document.addEventListener('fullscreenchange', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            document.addEventListener('webkitfullscreenchange', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            document.addEventListener('mozfullscreenchange', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            document.addEventListener('MSFullscreenChange', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            
            // Handle orientation change
            window.addEventListener('orientationchange', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            
            // Handle resize event (might happen when itch.io restores the game)
            window.addEventListener('resize', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            
            // Run orientation lock at regular intervals for the first few seconds
            // This helps catch restoration from itch.io's interface
            var intervalId = setInterval(attemptLockOrientation, 500);
            setTimeout(function() { clearInterval(intervalId); }, 5000);
            
            // Handle focus events
            window.addEventListener('focus', function() {
                setTimeout(attemptLockOrientation, 100);
            });
            
            // Create a MutationObserver to watch for DOM changes that might 
            // indicate the game is being restored from itch.io
            if (window.MutationObserver) {
                var observer = new MutationObserver(function(mutations) {
                    setTimeout(attemptLockOrientation, 100);
                });
                
                observer.observe(document.body, {
                    childList: true,
                    subtree: true,
                    attributes: true
                });
            }
        }
    );
#endif
}

void UpdateDrawFrame(void) {
    if (!game) return;
    
    game->update();
    
    // Begin rendering to the game target
    BeginTextureMode(gameTarget);
    ClearBackground(BLACK);
    game->draw();
    EndTextureMode();

    // Draw the game target to the screen
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(gameTarget.texture, 
                  (Rectangle){ 0.0f, 0.0f, (float)gameTarget.texture.width, (float)-gameTarget.texture.height },
                  (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
                  (Vector2){ 0, 0 }, 0.0f, WHITE);
    EndDrawing();
}

int main(void) {
    // Initialize window with base dimensions first
    InitWindow(BASE_WINDOW_WIDTH, BASE_WINDOW_HEIGHT, "Solitaire");
    
    if (!IsWindowReady()) {
        return -1;
    }

    // Lock orientation to landscape for mobile
    LockOrientation();

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

    // Create render texture for the game
    gameTarget = LoadRenderTexture(BASE_WINDOW_WIDTH, BASE_WINDOW_HEIGHT);
    SetTextureFilter(gameTarget.texture, TEXTURE_FILTER_BILINEAR);

#ifdef EMSCRIPTEN_BUILD
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose() && !game->shouldExit()) {
        UpdateDrawFrame();
    }
#endif

    // Cleanup
    if (game) {
        delete game;
        game = nullptr;
    }
    
    UnloadRenderTexture(gameTarget);
    CloseWindow();
    return 0;
}