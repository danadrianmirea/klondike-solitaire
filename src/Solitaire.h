#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <chrono>
#include "Card.h"

// Define debug flag
#define DEBUG 1

// Define base constants
const int BASE_CARD_WIDTH = 71;
const int BASE_CARD_HEIGHT = 96;
const int BASE_CARD_SPACING = 20;
const int BASE_TABLEAU_SPACING = 100;
const int BASE_WINDOW_WIDTH = 800;
const int BASE_WINDOW_HEIGHT = 600;
const int BASE_MENU_HEIGHT = 30;

// Base menu constants
const int BASE_MENU_FILE_X = 160;
const int BASE_MENU_FILE_WIDTH = 100;
const int BASE_MENU_ITEM_HEIGHT = 25;
const int BASE_MENU_TEXT_PADDING = 5;
const int BASE_MENU_DROPDOWN_HEIGHT = BASE_MENU_ITEM_HEIGHT * 4;  // 4 menu items

const float SCALE_FACTOR = 1.5f;

// Scaled constants (will be calculated in constructor)
extern int CARD_WIDTH;
extern int CARD_HEIGHT;
extern int CARD_SPACING;
extern int TABLEAU_SPACING;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern int MENU_HEIGHT;
extern int MENU_FILE_X;
extern int MENU_FILE_WIDTH;
extern int MENU_ITEM_HEIGHT;
extern int MENU_TEXT_PADDING;
extern int MENU_DROPDOWN_HEIGHT;

class Solitaire {
public:
    Solitaire();
    ~Solitaire();

    void handleMouseDown(Vector2 pos);
    void handleMouseUp(Vector2 pos);
    void handleDoubleClick(Vector2 pos);
    void handleRightClick(Vector2 pos);
    void update();
    void draw();

private:
    // Game state
    std::vector<std::vector<Card>> tableau;
    std::vector<std::vector<Card>> foundations;
    std::vector<Card> stock;
    std::vector<Card> waste;
    std::vector<Card> draggedCards;
    int draggedStartIndex;
    std::vector<Card>* draggedSourcePile;
    bool gameWon;
    Vector2 dragOffset;  // Track the offset between mouse and card position during drag
    double lastDealTime;  // Track when the last card was dealt to waste
    Card* lastDrawnCard;  // Track the last card that was drawn from stock

    // Menu state
    bool menuOpen;
    void handleMenuClick(Vector2 pos);

    // Helper methods
    void resetGame();
    void loadCards();
    void dealCards();
    std::vector<Card>* getPileAtPos(Vector2 pos);
    bool canMoveToTableau(const Card& card, const std::vector<Card>& targetPile);
    bool canMoveToFoundation(const Card& card, const std::vector<Card>& targetPile);
    bool moveCards(std::vector<Card>& sourcePile, std::vector<Card>& targetPile, 
                  int startIndex, int endIndex = -1);
    std::vector<Card>* findValidFoundationPile(const Card& card);
    bool checkWin();

    // Helper method to get the next value in sequence
    std::string getNextValue(const std::string& value);

    // Save and load game methods
    void saveGame();
    bool loadGame();
}; 