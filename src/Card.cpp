#include "Card.h"
#include "Solitaire.h" // For CARD_WIDTH and CARD_HEIGHT
#include <algorithm>
#include <iostream>
#include <filesystem>

// Initialize static members
Texture2D Card::cardBack = {0};
std::unordered_map<std::string, Texture2D> Card::textureCache;
bool Card::texturesLoaded = false;

void Card::loadTexture(const std::string& imagePath) {
    if (FileExists(imagePath.c_str())) {
        Image img = LoadImage(imagePath.c_str());
        if (img.data == NULL) {
            std::cerr << "Failed to load image: " << imagePath << std::endl;
            return;
        }

        // Scale the image
        ImageResize(&img, CARD_WIDTH, CARD_HEIGHT);

        // Create texture from scaled image
        Texture2D texture = LoadTextureFromImage(img);
        if (texture.id == 0) {
            std::cerr << "Failed to create texture from image: " << imagePath << std::endl;
            UnloadImage(img);
            return;
        }

        // Cache the texture
        textureCache[imagePath] = texture;

        // Clean up
        UnloadImage(img);
    } else {
        std::cerr << "Card image file not found: " << imagePath << std::endl;
    }
}

void Card::preloadTextures() {
    if (texturesLoaded) return;

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

    // Load textures sequentially to avoid WebGL context issues
    for (const auto& imagePath : imagePaths) {
        loadTexture(imagePath);
    }

    texturesLoaded = true;
}

void Card::loadCardBack(const std::string &imagePath) {
    if (cardBack.id == 0) { // Only load if not already loaded
        if (FileExists(imagePath.c_str())) {
            Image img = LoadImage(imagePath.c_str());
            if (img.data == NULL) {
                std::cerr << "Failed to load card back image: " << imagePath << std::endl;
                return;
            }

            ImageResize(&img, CARD_WIDTH, CARD_HEIGHT);
            cardBack = LoadTextureFromImage(img);
            UnloadImage(img);

            if (cardBack.id == 0) {
                std::cerr << "Failed to load card back texture: " << imagePath << std::endl;
            }
        } else {
            std::cerr << "Card back image file not found: " << imagePath << std::endl;
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
      DrawTexture(image, (int)rect.x, (int)rect.y, WHITE);
    } else {
      // Fallback if texture failed to load
      DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height,
                    BLUE);
      DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width,
                         (int)rect.height, BLACK);
    }
  } else {
    // Draw card back texture
    if (cardBack.id != 0) {
      DrawTexture(cardBack, (int)rect.x, (int)rect.y, WHITE);
    } else {
      // Fallback to simple rectangle if texture not loaded
      DrawRectangle((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height,
                    RED);
      DrawRectangleLines((int)rect.x, (int)rect.y, (int)rect.width,
                         (int)rect.height, BLACK);
    }
  }
}