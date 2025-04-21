#include "Card.h"
#include "Solitaire.h"
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>

// Initialize static members
Texture2D Card::cardBack = {0};
std::unordered_map<std::string, Texture2D> Card::textureCache;
bool Card::texturesLoaded = false;
bool Card::isMobile = false;  // Initialize isMobile to false

// Add new static member to track loading progress
static std::atomic<int> loadedTexturesCount(0);
static std::atomic<bool> loadingInProgress(false);

void Card::loadTexture(const std::string& imagePath) {
    if (!FileExists(imagePath.c_str())) {
        return;
    }

    Image img = LoadImage(imagePath.c_str());
    if (img.data == NULL) {
        return;
    }

    // Scale the image based on Solitaire's current scaleFactor
    int scaledWidth = static_cast<int>(BASE_CARD_WIDTH * Solitaire::scaleFactor);
    int scaledHeight = static_cast<int>(BASE_CARD_HEIGHT * Solitaire::scaleFactor);
    
    // Resize the image to match the scaled dimensions
    ImageResize(&img, scaledWidth, scaledHeight);

    // Create texture from scaled image
    Texture2D texture = LoadTextureFromImage(img);
    if (texture.id == 0) {
        UnloadImage(img);
        return;
    }

    // Cache the texture
    textureCache[imagePath] = texture;

    // Clean up
    UnloadImage(img);
}

void Card::preloadTextures() {
    if (texturesLoaded || loadingInProgress) {
        return;
    }

    loadingInProgress = true;
    loadedTexturesCount = 0;

    const std::string suits[] = {"hearts", "diamonds", "clubs", "spades"};
    const std::string values[] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};

    // Get the current working directory
    std::string currentDir = GetWorkingDirectory();

    std::vector<std::string> imagePaths;

    // Collect all image paths
    for (const auto& suit : suits) {
        for (const auto& value : values) {
            std::string imagePath = "assets/cards/" + value + "_of_" + suit + ".png";
            if (!FileExists(imagePath.c_str())) {
                imagePath = currentDir + "/assets/cards/" + value + "_of_" + suit + ".png";
            }
            imagePaths.push_back(imagePath);
        }
    }

#ifdef __EMSCRIPTEN__
    // For web builds, load textures sequentially
    for (const auto& path : imagePaths) {
        loadTexture(path);
        loadedTexturesCount++;
    }
    texturesLoaded = true;
    loadingInProgress = false;
#else
    // For native builds, use a background thread
    std::thread([imagePaths]() {
        for (const auto& path : imagePaths) {
            loadTexture(path);
            loadedTexturesCount++;
        }
        texturesLoaded = true;
        loadingInProgress = false;
    }).detach();
#endif
}

float Card::getLoadingProgress() {
    if (!loadingInProgress) return 1.0f;
    return static_cast<float>(loadedTexturesCount) / 52.0f; // 52 cards total
}

void Card::loadCardBack(const std::string &imagePath) {
    if (cardBack.id == 0) { // Only load if not already loaded
        if (FileExists(imagePath.c_str())) {
            Image img = LoadImage(imagePath.c_str());
            if (img.data == NULL) {
                return;
            }

            // Scale the image based on Solitaire's current scaleFactor
            int scaledWidth = static_cast<int>(BASE_CARD_WIDTH * Solitaire::scaleFactor);
            int scaledHeight = static_cast<int>(BASE_CARD_HEIGHT * Solitaire::scaleFactor);
            
            ImageResize(&img, scaledWidth, scaledHeight);
            cardBack = LoadTextureFromImage(img);
            UnloadImage(img);
        }
    }
}

void Card::unloadCardBack() {
    if (cardBack.id != 0) {
        UnloadTexture(cardBack);
        cardBack = {0};
    }
}

void Card::unloadAllTextures() {
    // Unload all cached textures
    for (auto& pair : textureCache) {
        if (pair.second.id != 0) {
            UnloadTexture(pair.second);
        }
    }
    textureCache.clear();
    
    // Unload card back texture
    unloadCardBack();
    
    texturesLoaded = false;
}

Card::Card(const std::string &suit, const std::string &value,
           const std::string &imagePath)
    : suit(suit), value(value), faceUp(false), image({0}) {
    // Check if texture is already in cache
    auto it = textureCache.find(imagePath);
    if (it != textureCache.end()) {
        image = it->second;
    } else {
        // If textures aren't pre-loaded, load this one
        loadTexture(imagePath);
        image = textureCache[imagePath];
    }

    rect = {0, 0, (float)CARD_WIDTH, (float)CARD_HEIGHT};
}

// Copy constructor - don't unload the original texture
Card::Card(const Card &other)
    : suit(other.suit), value(other.value), faceUp(other.faceUp),
      image(other.image), rect(other.rect) {}

// Assignment operator - don't unload the original texture
Card &Card::operator=(const Card &other) {
  if (this != &other) {
    suit = other.suit;
    value = other.value;
    faceUp = other.faceUp;
    image = other.image;
    rect = other.rect;
  }
  return *this;
}

Card::~Card() {
  // Textures are now managed by the cache, no need to unload here
}

void Card::flip() { faceUp = !faceUp; }

int Card::getValue() const {
  if (value == "ace")
    return 1;
  if (value == "jack")
    return 11;
  if (value == "queen")
    return 12;
  if (value == "king")
    return 13;
  return std::stoi(value);
}

bool Card::isRed() const { return suit == "hearts" || suit == "diamonds"; }

void Card::setPosition(float x, float y) {
  rect.x = x;
  rect.y = y;
}

void Card::draw() const {
    if (faceUp) {
        if (image.id != 0) {
            // Validate texture dimensions
            if (image.width <= 0 || image.height <= 0) {
                DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, BLUE);
                DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, BLACK);
                return;
            }
            DrawTexture(image, (int)rect.x, (int)rect.y, WHITE);
        } else {
            // Fallback if texture failed to load
            DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, BLUE);
            DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, BLACK);
        }
    } else {
        if (cardBack.id != 0) {
            DrawTexture(cardBack, (int)rect.x, (int)rect.y, WHITE);
        } else {
            // Fallback if card back texture failed to load
            DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, RED);
            DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height, BLACK);
        }
    }
}