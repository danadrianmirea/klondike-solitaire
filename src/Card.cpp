#include "Card.h"
#include "Solitaire.h" // For CARD_WIDTH and CARD_HEIGHT
#include <algorithm>
#include <iostream>

// Initialize static member
Texture2D Card::cardBack = {0};

void Card::loadCardBack(const std::string &imagePath) {
  if (cardBack.id == 0) { // Only load if not already loaded
    if (FileExists(imagePath.c_str())) {
      Image img = LoadImage(imagePath.c_str());
      ImageResize(&img, CARD_WIDTH, CARD_HEIGHT);
      cardBack = LoadTextureFromImage(img);
      UnloadImage(img);

      if (cardBack.id == 0) {
        std::cerr << "Failed to load card back texture: " << imagePath
                  << std::endl;
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

Card::Card(const std::string &suit, const std::string &value,
           const std::string &imagePath)
    : suit(suit), value(value), faceUp(false), image({0}) {
  // Print the full path being used for debugging
  std::cout << "Loading card texture from: " << imagePath << std::endl;

  // Check if file exists before loading
  if (FileExists(imagePath.c_str())) {
    // Load the image
    Image img = LoadImage(imagePath.c_str());
    if (img.data == NULL) {
      std::cerr << "Failed to load image: " << imagePath << std::endl;
      return;
    }

    // Scale the image
    ImageResize(&img, CARD_WIDTH, CARD_HEIGHT);

    // Create texture from scaled image
    image = LoadTextureFromImage(img);
    if (image.id == 0) {
      std::cerr << "Failed to create texture from image: " << imagePath
                << std::endl;
    }

    // Clean up
    UnloadImage(img);
  } else {
    std::cerr << "Card image file not found: " << imagePath << std::endl;
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
  // Only unload the texture if this is the last card using it
  // We'll need to track texture usage to do this properly
  // For now, we'll keep the texture loaded
  // UnloadTexture(image);
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