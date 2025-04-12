#pragma once
#include <raylib.h>
#include <string>

class Card {
private:
    std::string suit;
    std::string value;
    Texture2D image;
    Rectangle rect;
    bool faceUp;
    static Texture2D cardBack;  // Static member for card back texture

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
}; 