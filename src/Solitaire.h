#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include "Card.h"

// Define debug flag
#define DEBUG 1

// Base constants (original game size)
constexpr int BASE_CARD_WIDTH = 71;
constexpr int BASE_CARD_HEIGHT = 96;
constexpr int BASE_CARD_SPACING = 20;
constexpr int BASE_TABLEAU_SPACING = 100;
constexpr int BASE_WINDOW_WIDTH = 800;
constexpr int BASE_WINDOW_HEIGHT = 600;
constexpr int BASE_MENU_HEIGHT = 30;

// Calculate scaling factor based on height (to maintain aspect ratio)
// Add a small margin (0.85) to account for Windows decorations and taskbar
constexpr float SCALE_FACTOR = 0.85f;  // Will be calculated in constructor

// Scaled constants (will be calculated in constructor)
extern int CARD_WIDTH;
extern int CARD_HEIGHT;
extern int CARD_SPACING;
extern int TABLEAU_SPACING;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern int MENU_HEIGHT;

class Solitaire {
public:
    Solitaire();
    ~Solitaire();

    void handleMouseDown(Vector2 pos);
    void handleMouseUp(Vector2 pos);
    void handleDoubleClick(Vector2 pos);
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
}; 