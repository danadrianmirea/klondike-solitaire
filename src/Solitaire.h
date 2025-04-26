#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <chrono>
#include "Card.h"

// Define debug flag
#define DEBUG 1

// Base dimensions (unscaled)
const int baseCardWidth = 71;
const int baseCardHeight = 96;
const int baseCardSpacing = 20;
const int baseTableauSpacing = 100;
const int gameScreenWidth = 800;
const int gameScreenHeight = 600;
const int baseMenuHeight = 30;
const int baseMenuFileX = 160;
const int baseMenuFileWidth = 100;
const int baseMenuHelpX = 260;  // Position of Help menu
const int baseMenuHelpWidth = 100;  // Width of Help menu
const int baseMenuItemHeight = 25;
const int baseMenuTextPadding = 5;
const int baseMenuDropdownHeight = baseMenuItemHeight * 4;  // 4 menu items
const int baseMenuHelpDropdownHeight = baseMenuItemHeight * 1;  // 1 menu item for Help

// Scaled dimensions (extern)
extern int cardWidth;
extern int cardHeight;
extern int cardSpacing;
extern int tableauSpacing;
extern int menuHeight;
extern int menuFileX;
extern int menuFileWidth;
extern int menuHelpX;
extern int menuHelpWidth;
extern int menuItemHeight;
extern int menuTextPadding;
extern int menuDropdownHeight;
extern int menuHelpDropdownHeight;

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
    bool shouldExit() const { return shouldClose; }  // New getter method

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
    bool helpMenuOpen;  // New state for Help menu
    bool shouldClose;
    bool aboutDialogOpen;  // New state for About dialog

    void handleMenuClick(Vector2 pos);
    void showAboutDialog();  // New method to show About dialog

    // Helper methods
    void resetGame();
    void loadCards();
    void dealCards();
    void returnDraggedCards(); // Helper to return dragged cards to original position
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
    bool saveGame();
    bool loadGame();
}; 