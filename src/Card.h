#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <future>

class Card {
private:
    std::string suit;
    std::string value;
    Texture2D image;
    Rectangle rect;
    bool faceUp;
    static Texture2D cardBack;  // Static member for card back texture
    static std::unordered_map<std::string, Texture2D> textureCache;  // Cache for card textures
    static bool texturesLoaded;  // Flag to track if textures are pre-loaded

    // Helper function to load a single texture
    static void loadTexture(const std::string& imagePath);

public:
    Card(const std::string& suit, const std::string& value, const std::string& imagePath);
    Card(const Card& other);  // Copy constructor
    Card& operator=(const Card& other);  // Assignment operator
    ~Card();

    void flip();
    int getValue() const;
    bool isRed() const;
    const std::string& getSuit() const { return suit; }
    bool isFaceUp() const { return faceUp; }
    const Rectangle& getRect() const { return rect; }
    const Texture2D& getImage() const { return image; }
    void setPosition(float x, float y);
    void draw() const;

    static void loadCardBack(const std::string& imagePath);
    static void unloadCardBack();
    static void unloadAllTextures();
    static void preloadTextures();  // New function to pre-load all textures
    static bool areTexturesLoaded() { return texturesLoaded; }  // Check if textures are loaded
    static float getLoadingProgress();
}; 