#include "Solitaire.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>
#include <fstream>

#ifndef EMSCRIPTEN_BUILD
#include <nlohmann/json.hpp>

using json = nlohmann::json;
#endif

// Define the scaled constants
int CARD_WIDTH;
int CARD_HEIGHT;
int CARD_SPACING;
int TABLEAU_SPACING;
int WINDOW_WIDTH;
int WINDOW_HEIGHT;
int MENU_HEIGHT;
int MENU_FILE_X;
int MENU_FILE_WIDTH;
int MENU_DROPDOWN_HEIGHT;
int MENU_TEXT_PADDING;
int MENU_ITEM_HEIGHT;

Solitaire::Solitaire() {
    auto now = std::chrono::system_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    srand(static_cast<unsigned int>(millis));
    
    // Calculate scaling factor based on screen size
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    float scaleFactor = std::min(
        static_cast<float>(screenHeight) / BASE_WINDOW_HEIGHT,
        static_cast<float>(screenWidth) / BASE_WINDOW_WIDTH
    ) * SCALE_FACTOR;
    
    // Calculate scaled constants
    CARD_WIDTH = static_cast<int>(BASE_CARD_WIDTH * scaleFactor);
    CARD_HEIGHT = static_cast<int>(BASE_CARD_HEIGHT * scaleFactor);
    CARD_SPACING = static_cast<int>(BASE_CARD_SPACING * scaleFactor);
    TABLEAU_SPACING = static_cast<int>(BASE_TABLEAU_SPACING * scaleFactor);
    WINDOW_WIDTH = static_cast<int>(BASE_WINDOW_WIDTH * scaleFactor);
    WINDOW_HEIGHT = static_cast<int>(BASE_WINDOW_HEIGHT * scaleFactor);
    MENU_HEIGHT = static_cast<int>(BASE_MENU_HEIGHT * scaleFactor);
    MENU_FILE_X = static_cast<int>(BASE_MENU_FILE_X * scaleFactor);
    MENU_FILE_WIDTH = static_cast<int>(BASE_MENU_FILE_WIDTH * scaleFactor);
    MENU_DROPDOWN_HEIGHT = static_cast<int>(BASE_MENU_DROPDOWN_HEIGHT * scaleFactor);
    MENU_TEXT_PADDING = static_cast<int>(BASE_MENU_TEXT_PADDING * scaleFactor);
    MENU_ITEM_HEIGHT = static_cast<int>(BASE_MENU_ITEM_HEIGHT * scaleFactor);
    
    // Set window size
    SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Center window on screen
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    SetWindowPosition(
        (monitorWidth - WINDOW_WIDTH) / 2,
        (monitorHeight - WINDOW_HEIGHT) / 2
    );
    
    // Show initial loading screen
    BeginDrawing();
    ClearBackground(GREEN);
    DrawText("Loading textures...", WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 20, 20, WHITE);
    EndDrawing();
    
    // Load card back texture
    Card::loadCardBack("assets/cards/card_back_red.png");
    
    // Preload all card textures in parallel
    Card::preloadTextures();
    
    // Keep showing loading screen until textures are fully loaded
    while (!Card::areTexturesLoaded()) {
        BeginDrawing();
        ClearBackground(GREEN);
        DrawText("Loading textures...", WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2 - 20, 20, WHITE);
        EndDrawing();
    }
    
    // Now that textures are loaded, initialize the game
    resetGame();

    // Initialize menu state
    menuOpen = false;
}

Solitaire::~Solitaire() {
    // Clean up all textures
    Card::unloadAllTextures();
}

void Solitaire::resetGame() {
    // Clear all piles but keep the textures
    for (auto& pile : tableau) {
        pile.clear();
    }
    for (auto& pile : foundations) {
        pile.clear();
    }
    stock.clear();
    waste.clear();
    draggedCards.clear();
    draggedSourcePile = nullptr;
    gameWon = false;

    // Initialize tableau and foundations
    tableau.resize(7);
    foundations.resize(4);

    // Create a new deck by reusing existing textures
    const std::string suits[] = {"hearts", "diamonds", "clubs", "spades"};
    const std::string values[] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};

    for (const auto& suit : suits) {
        for (const auto& value : values) {
            std::string imagePath = "assets/cards/" + value + "_of_" + suit + ".png";
            stock.emplace_back(suit, value, imagePath);
        }
    }

    // Shuffle the deck
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(stock.begin(), stock.end(), g);

    dealCards();
}

void Solitaire::loadCards() {
    const std::string suits[] = {"hearts", "diamonds", "clubs", "spades"};
    const std::string values[] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};

    // Get the current working directory
    std::string currentDir = GetWorkingDirectory();
    std::cout << "Current working directory: " << currentDir << std::endl;

    for (const auto& suit : suits) {
        for (const auto& value : values) {
            // Try both possible paths
            std::string imagePath = "assets/cards/" + value + "_of_" + suit + ".png";
            if (!FileExists(imagePath.c_str())) {
                // Try alternative path
                imagePath = currentDir + "/assets/cards/" + value + "_of_" + suit + ".png";
                if (!FileExists(imagePath.c_str())) {
                    std::cerr << "Could not find card image for: " << value << " of " << suit << std::endl;
                    continue;
                }
            }
            stock.emplace_back(suit, value, imagePath);
        }
    }

    // Shuffle the deck
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(stock.begin(), stock.end(), g);
}

void Solitaire::dealCards() {
    // Deal cards to tableau piles
    for (int i = 0; i < 7; i++) {
        for (int j = i; j < 7; j++) {
            if (!stock.empty()) {
                Card card = stock.back();
                stock.pop_back();
                // The first card added to each pile (when j == i) should be face up
                if (j == i) {
                    card.flip();
                }
                // Add to back of vector (top of pile)
                tableau[j].push_back(card);
            }
        }
    }
}

std::vector<Card>* Solitaire::getPileAtPos(Vector2 pos) {
    // Check tableau piles (moved down by MENU_HEIGHT)
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR + MENU_HEIGHT;  // Add MENU_HEIGHT
        
        // Check if click is within the pile's x-range
        if (x <= pos.x && pos.x <= x + CARD_WIDTH) {
            // If pile has cards, check each card's position
            if (!tableau[i].empty()) {
                float cardY = y;
                for (size_t j = 0; j < tableau[i].size(); j++) {
                    if (tableau[i][j].isFaceUp()) {
                        tableau[i][j].setPosition(x, cardY);
                        if (CheckCollisionPointRec(pos, tableau[i][j].getRect())) {
                            return &tableau[i];
                        }
                    }
                    cardY += CARD_SPACING;
                }
            } else {
                // Empty tableau pile - check if click is in the empty space
                Rectangle emptyRect = { x, y, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
                if (CheckCollisionPointRec(pos, emptyRect)) {
                    return &tableau[i];
                }
            }
        }
    }

    // Check foundation piles (moved down by MENU_HEIGHT)
    for (int i = 0; i < 4; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 10 * SCALE_FACTOR + MENU_HEIGHT;  // Add MENU_HEIGHT
        
        // Check if click is within the pile's x-range and y-range
        if (x <= pos.x && pos.x <= x + CARD_WIDTH && y <= pos.y && pos.y <= y + CARD_HEIGHT) {
            if (!foundations[i].empty()) {
                foundations[i].back().setPosition(x, y);
                if (CheckCollisionPointRec(pos, foundations[i].back().getRect())) {
                    return &foundations[i];
                }
            } else {
                // Empty foundation pile
                Rectangle emptyRect = { x, y, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
                if (CheckCollisionPointRec(pos, emptyRect)) {
                    return &foundations[i];
                }
            }
        }
    }

    // Check stock pile (no change needed)
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    Rectangle stockRect = { stockX, stockY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
    if (CheckCollisionPointRec(pos, stockRect)) {
        return &stock;
    }

    // Check waste pile (no change needed)
    float wasteX = stockX + TABLEAU_SPACING;
    float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    if (!waste.empty()) {
        waste.back().setPosition(wasteX, wasteY);
        if (CheckCollisionPointRec(pos, waste.back().getRect())) {
            return &waste;
        }
    } else {
        // Empty waste pile
        Rectangle emptyRect = { wasteX, wasteY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
        if (CheckCollisionPointRec(pos, emptyRect)) {
            return &waste;
        }
    }

    return nullptr;
}

bool Solitaire::canMoveToTableau(const Card& card, const std::vector<Card>& targetPile) {
    if (targetPile.empty()) {
        // Only kings can be placed on empty tableau
#if DEBUG == 1
        std::cout << "Checking if card can be placed on empty tableau. Card value: " << card.getValue() 
                  << ", Card string value: " << card.getSuit() << " " << card.getValue() << std::endl;
#endif
        return card.getValue() == 13;  // 13 represents king
    }
    
    const Card& topCard = targetPile.back();
    
#if DEBUG == 1
    std::cout << "\nTableau move check details:" << std::endl;
    std::cout << "Moving card: " << card.getSuit() << " " << card.getValue() << " (value: " << card.getValue() << ")" << std::endl;
    std::cout << "Target card: " << topCard.getSuit() << " " << topCard.getValue() << " (value: " << topCard.getValue() << ")" << std::endl;
    std::cout << "Card colors - Moving: " << (card.isRed() ? "red" : "black") 
              << ", Target: " << (topCard.isRed() ? "red" : "black") << std::endl;
    std::cout << "Value check - Moving card value: " << card.getValue() 
              << ", Target card value - 1: " << (topCard.getValue() - 1) << std::endl;
#endif
    
    // Check if colors are different and values are in sequence
    bool colorsDifferent = (card.isRed() != topCard.isRed());
    bool valuesInSequence = (card.getValue() == topCard.getValue() - 1);
    
#if DEBUG == 1
    std::cout << "Move validation:" << std::endl;
    std::cout << "Colors different: " << (colorsDifferent ? "yes" : "no") << std::endl;
    std::cout << "Values in sequence: " << (valuesInSequence ? "yes" : "no") << std::endl;
    std::cout << "Final result: " << (colorsDifferent && valuesInSequence ? "valid" : "invalid") << std::endl;
#endif
    
    return colorsDifferent && valuesInSequence;
}

std::string Solitaire::getNextValue(const std::string& value) {
    if (value == "king") return "queen";
    if (value == "queen") return "jack";
    if (value == "jack") return "10";
    if (value == "10") return "9";
    if (value == "9") return "8";
    if (value == "8") return "7";
    if (value == "7") return "6";
    if (value == "6") return "5";
    if (value == "5") return "4";
    if (value == "4") return "3";
    if (value == "3") return "2";
    if (value == "2") return "ace";
    return "";  // No next value for ace
}

bool Solitaire::canMoveToFoundation(const Card& card, const std::vector<Card>& targetPile) {
    if (targetPile.empty()) {
        return card.getValue() == 1; // Only aces can start a foundation
    }
    
    const Card& topCard = targetPile.back();
    return (card.getSuit() == topCard.getSuit()) && (card.getValue() == topCard.getValue() + 1);
}

bool Solitaire::moveCards(std::vector<Card>& sourcePile, std::vector<Card>& targetPile, 
                         int startIndex, int endIndex) {
#if DEBUG == 1
    std::cout << "\nAttempting to move cards:" << std::endl;
    std::cout << "Source pile size: " << sourcePile.size() << std::endl;
    std::cout << "Target pile size: " << targetPile.size() << std::endl;
    std::cout << "Start index: " << startIndex << std::endl;
    std::cout << "End index: " << endIndex << std::endl;
    if (!sourcePile.empty()) {
        std::cout << "Top card of source pile: " << sourcePile.back().getSuit() << " " << sourcePile.back().getValue() << std::endl;
    }
    if (!targetPile.empty()) {
        std::cout << "Top card of target pile: " << targetPile.back().getSuit() << " " << targetPile.back().getValue() << std::endl;
    }
#endif

    if (startIndex < 0 || startIndex >= sourcePile.size()) {
#if DEBUG == 1
        std::cout << "Invalid start index, move failed" << std::endl;
#endif
        return false;
    }

    if (endIndex == -1) {
        endIndex = sourcePile.size() - 1;
    }

#if DEBUG == 1
    std::cout << "Moving " << (endIndex - startIndex + 1) << " cards" << std::endl;
#endif

    // Move cards
    for (int i = startIndex; i <= endIndex; i++) {
        targetPile.push_back(sourcePile[i]);
    }
    sourcePile.erase(sourcePile.begin() + startIndex, sourcePile.begin() + endIndex + 1);

    // Flip the new top card of the source pile if it exists
    if (!sourcePile.empty() && !sourcePile.back().isFaceUp()) {
        sourcePile.back().flip();
    }

#if DEBUG == 1
    std::cout << "Move completed successfully" << std::endl;
    std::cout << "New source pile size: " << sourcePile.size() << std::endl;
    std::cout << "New target pile size: " << targetPile.size() << std::endl;
#endif

    return true;
}

void Solitaire::handleMouseDown(Vector2 pos) {
    // Check stock pile first (no change needed)
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    Rectangle stockRect = { stockX, stockY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
    if (CheckCollisionPointRec(pos, stockRect)) {
#if DEBUG_STOCKPILE == 1
        // Debug print stock and waste state
        std::cout << "\nBefore stock pile interaction:" << std::endl;
        std::cout << "Stock pile (" << stock.size() << " cards): ";
        for (const auto& card : stock) {
            std::cout << card.getValue() << " of " << card.getSuit() << " ";
        }
        std::cout << std::endl;
        std::cout << "Waste pile (" << waste.size() << " cards): ";
        for (const auto& card : waste) {
            std::cout << card.getValue() << " of " << card.getSuit() << " ";
        }
        std::cout << std::endl;
#endif

        if (stock.empty() && !waste.empty()) {
            // Only restore waste cards if stock is empty and waste is not empty
            while (!waste.empty()) {
                Card card = waste.back();
                waste.pop_back();
                card.flip();  // Flip face down
                stock.push_back(card);
            }
            lastDrawnCard = nullptr;  // Reset last drawn card when recycling waste
#if DEBUG_STOCKPILE == 1
            std::cout << "Restored waste cards to stock" << std::endl;
#endif
            return;  // Return here to prevent any further handling
        }
        
        // Only deal a card if stock is not empty
        if (!stock.empty()) {
            Card card = stock.back();
            stock.pop_back();
            card.flip();  // Flip face up
            waste.push_back(card);
            lastDrawnCard = &waste.back();  // Track the last drawn card
            lastDealTime = GetTime();
#if DEBUG_STOCKPILE == 1
            std::cout << "Dealt card: " << card.getValue() << " of " << card.getSuit() << std::endl;
#endif
        }

#if DEBUG_STOCKPILE == 1
        // Debug print stock and waste state after interaction
        std::cout << "\nAfter stock pile interaction:" << std::endl;
        std::cout << "Stock pile (" << stock.size() << " cards): ";
        for (const auto& card : stock) {
            std::cout << card.getValue() << " of " << card.getSuit() << " ";
        }
        std::cout << std::endl;
        std::cout << "Waste pile (" << waste.size() << " cards): ";
        for (const auto& card : waste) {
            std::cout << card.getValue() << " of " << card.getSuit() << " ";
        }
        std::cout << std::endl << std::endl;
#endif

        return;  // Return after handling stock pile
    }

    // Find the card that was clicked (account for MENU_HEIGHT in foundation and tableau)
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR + MENU_HEIGHT;  // Add MENU_HEIGHT
        
        // Check if click is within the pile's x-range
        if (x <= pos.x && pos.x <= x + CARD_WIDTH) {
            // Find the actual card that was clicked by checking the y-position
            float baseY = y;
            float clickedY = pos.y;
            
            // Calculate which card was actually clicked based on y-position
            int clickedIndex = (clickedY - baseY) / CARD_SPACING;
            if (clickedIndex < 0) clickedIndex = 0;
            if (clickedIndex >= tableau[i].size()) clickedIndex = tableau[i].size() - 1;
            
            // Only select if the card at this position is face up and we're actually clicking on its rectangle
            if (tableau[i][clickedIndex].isFaceUp()) {
                // Set the card's position to check for collision
                tableau[i][clickedIndex].setPosition(x, baseY + clickedIndex * CARD_SPACING);
                if (CheckCollisionPointRec(pos, tableau[i][clickedIndex].getRect())) {
                    draggedCards.clear();
                    for (int j = clickedIndex; j < tableau[i].size(); j++) {
                        draggedCards.push_back(tableau[i][j]);
                    }
                    draggedStartIndex = clickedIndex;
                    draggedSourcePile = &tableau[i];
                    
                    // Calculate offset from mouse position to card position
                    dragOffset = {
                        pos.x - x,
                        pos.y - (baseY + clickedIndex * CARD_SPACING)
                    };
                    break;
                }
            }
        }
        if (!draggedCards.empty()) break;
    }

    // Check foundation piles if no card was found in tableau
    if (draggedCards.empty()) {
        for (int i = 0; i < 4; i++) {
            float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
            float y = 10 * SCALE_FACTOR + MENU_HEIGHT;
            
            if (!foundations[i].empty()) {
                foundations[i].back().setPosition(x, y);
                if (CheckCollisionPointRec(pos, foundations[i].back().getRect())) {
                    draggedCards.clear();
                    draggedCards.push_back(foundations[i].back());
                    draggedStartIndex = foundations[i].size() - 1;
                    draggedSourcePile = &foundations[i];
                    
                    // Calculate offset from mouse position to card position
                    dragOffset = {
                        pos.x - x,
                        pos.y - y
                    };
                    break;
                }
            }
        }
    }

    // Check waste pile if no card was found in tableau or foundation
    if (draggedCards.empty() && !waste.empty()) {
        float wasteX = stockX + TABLEAU_SPACING;
        float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
        waste.back().setPosition(wasteX, wasteY);
        if (CheckCollisionPointRec(pos, waste.back().getRect())) {
            draggedCards.clear();
            draggedCards.push_back(waste.back());
            draggedStartIndex = 0;
            draggedSourcePile = &waste;
            
            // Calculate offset from mouse position to card position
            dragOffset = {
                pos.x - wasteX,
                pos.y - wasteY
            };
        }
    }
}

void Solitaire::handleMouseUp(Vector2 pos) {
    if (draggedCards.empty()) return;

    std::vector<Card>* targetPile = getPileAtPos(pos);
    if (!targetPile) {
#if DEBUG == 1
        std::cout << "\nNo target pile found at position" << std::endl;
#endif
        // Return cards to original position
        if (draggedSourcePile == &waste) {
            float stockX = 50 * SCALE_FACTOR;
            float wasteX = stockX + TABLEAU_SPACING;
            float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
            draggedCards[0].setPosition(wasteX, wasteY);
        } else if (draggedSourcePile == &foundations[0] || draggedSourcePile == &foundations[1] || 
                  draggedSourcePile == &foundations[2] || draggedSourcePile == &foundations[3]) {
            // Find the original foundation pile
            for (int i = 0; i < 4; i++) {
                if (&foundations[i] == draggedSourcePile) {
                    float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
                    float y = 10 * SCALE_FACTOR + MENU_HEIGHT;
                    draggedCards[0].setPosition(x, y);
                    break;
                }
            }
        } else {
            // Find the original tableau pile
            for (int i = 0; i < 7; i++) {
                if (&tableau[i] == draggedSourcePile) {
                    float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
                    float y = 200 * SCALE_FACTOR + draggedStartIndex * CARD_SPACING;
                    draggedCards[0].setPosition(x, y);
                    break;
                }
            }
        }
        draggedCards.clear();
        draggedSourcePile = nullptr;
        return;
    }

    if (targetPile == draggedSourcePile) {
#if DEBUG == 1
        std::cout << "\nTarget pile is same as source pile, cancelling move" << std::endl;
#endif
        draggedCards.clear();
        draggedSourcePile = nullptr;
        return;
    }

    // Check if target pile is a foundation pile
    bool isFoundationPile = false;
    for (auto& foundation : foundations) {
        if (targetPile == &foundation) {
            isFoundationPile = true;
            break;
        }
    }

    // Check if we can move to tableau
    bool canMove = canMoveToTableau(draggedCards[0], *targetPile);
#if DEBUG == 1
    std::cout << "\nMove validation result: " << (canMove ? "valid" : "invalid") << std::endl;
    std::cout << "Target pile is " << (isFoundationPile ? "foundation" : "tableau") << std::endl;
#endif

    if (canMove) {
#if DEBUG == 1
        std::cout << "Attempting to move cards to tableau" << std::endl;
#endif
        // If dragging from waste pile or foundation pile, only move the top card
        if (draggedSourcePile == &waste || 
            draggedSourcePile == &foundations[0] || draggedSourcePile == &foundations[1] || 
            draggedSourcePile == &foundations[2] || draggedSourcePile == &foundations[3]) {
            moveCards(*draggedSourcePile, *targetPile, draggedSourcePile->size() - 1);
        } else {
            moveCards(*draggedSourcePile, *targetPile, draggedStartIndex);
        }
    }
    // Check if we can move to foundation (only single cards can be moved to foundation)
    else if (isFoundationPile && draggedCards.size() == 1 && canMoveToFoundation(draggedCards[0], *targetPile)) {
#if DEBUG == 1
        std::cout << "Attempting to move card to foundation" << std::endl;
#endif
        moveCards(*draggedSourcePile, *targetPile, draggedSourcePile->size() - 1);
    } else {
#if DEBUG == 1
        std::cout << "Invalid move, returning cards to original position" << std::endl;
#endif
        // Return cards to original position
        if (draggedSourcePile == &waste) {
            float stockX = 50 * SCALE_FACTOR;
            float wasteX = stockX + TABLEAU_SPACING;
            float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
            draggedCards[0].setPosition(wasteX, wasteY);
        } else if (draggedSourcePile == &foundations[0] || draggedSourcePile == &foundations[1] || 
                  draggedSourcePile == &foundations[2] || draggedSourcePile == &foundations[3]) {
            // Find the original foundation pile
            for (int i = 0; i < 4; i++) {
                if (&foundations[i] == draggedSourcePile) {
                    float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
                    float y = 10 * SCALE_FACTOR + MENU_HEIGHT;
                    draggedCards[0].setPosition(x, y);
                    break;
                }
            }
        } else {
            // Find the original tableau pile
            for (int i = 0; i < 7; i++) {
                if (&tableau[i] == draggedSourcePile) {
                    float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
                    float y = 200 * SCALE_FACTOR + draggedStartIndex * CARD_SPACING;
                    draggedCards[0].setPosition(x, y);
                    break;
                }
            }
        }
    }

    draggedCards.clear();
    draggedSourcePile = nullptr;
}

void Solitaire::handleDoubleClick(Vector2 pos) {
    std::vector<Card>* pile = getPileAtPos(pos);
    if (!pile || pile->empty()) return;

    // Don't allow double-clicking on waste pile if the card was just dealt
    if (pile == &waste && GetTime() - lastDealTime < 0.5) {  // 500ms delay
        return;
    }

    Card& card = pile->back();
    // Only allow double-clicking on face-up cards
    if (!card.isFaceUp()) return;

    std::vector<Card>* foundationPile = findValidFoundationPile(card);
    if (foundationPile) {
        moveCards(*pile, *foundationPile, pile->size() - 1);
    }
}

std::vector<Card>* Solitaire::findValidFoundationPile(const Card& card) {
    for (auto& foundation : foundations) {
        if (canMoveToFoundation(card, foundation)) {
            return &foundation;
        }
    }
    return nullptr;
}

bool Solitaire::checkWin() {
    for (const auto& foundation : foundations) {
        if (foundation.empty() || foundation.back().getValue() != 13) {
            return false;
        }
    }
    return true;
}

void Solitaire::saveGame() {
#ifndef EMSCRIPTEN_BUILD
    json gameState;
    
    // Save tableau piles
    for (const auto& pile : tableau) {
        json pileData;
        for (const auto& card : pile) {
            json cardData;
            cardData["suit"] = card.getSuit();
            cardData["value"] = card.getValue();
            cardData["faceUp"] = card.isFaceUp();
            pileData.push_back(cardData);
        }
        gameState["tableau"].push_back(pileData);
    }
    
    // Save foundation piles
    for (const auto& pile : foundations) {
        json pileData;
        for (const auto& card : pile) {
            json cardData;
            cardData["suit"] = card.getSuit();
            cardData["value"] = card.getValue();
            cardData["faceUp"] = card.isFaceUp();
            pileData.push_back(cardData);
        }
        gameState["foundations"].push_back(pileData);
    }
    
    // Save stock pile
    json stockData;
    for (const auto& card : stock) {
        json cardData;
        cardData["suit"] = card.getSuit();
        cardData["value"] = card.getValue();
        cardData["faceUp"] = card.isFaceUp();
        stockData.push_back(cardData);
    }
    gameState["stock"] = stockData;
    
    // Save waste pile
    json wasteData;
    for (const auto& card : waste) {
        json cardData;
        cardData["suit"] = card.getSuit();
        cardData["value"] = card.getValue();
        cardData["faceUp"] = card.isFaceUp();
        wasteData.push_back(cardData);
    }
    gameState["waste"] = wasteData;
    
    // Save game state to file
    std::ofstream file("solitaire_save.txt");
    if (file.is_open()) {
        file << gameState.dump(4);
        file.close();
#if DEBUG == 1
        std::cout << "Game saved successfully" << std::endl;
#endif
    } else {
#if DEBUG == 1
        std::cerr << "Failed to save game" << std::endl;
#endif
    }
#endif
}

bool Solitaire::loadGame() {
#ifndef EMSCRIPTEN_BUILD
    std::ifstream file("solitaire_save.txt");
    if (!file.is_open()) {
#if DEBUG == 1
        std::cerr << "No save file found" << std::endl;
#endif
        return false;
    }
    
    try {
        json gameState;
        file >> gameState;
        file.close();
        
        // Clear current game state
        tableau.clear();
        foundations.clear();
        stock.clear();
        waste.clear();
        draggedCards.clear();
        draggedSourcePile = nullptr;
        gameWon = false;
        
        // Initialize tableau and foundations
        tableau.resize(7);
        foundations.resize(4);
        
        // Load tableau piles
        for (size_t i = 0; i < gameState["tableau"].size(); i++) {
            for (const auto& cardData : gameState["tableau"][i]) {
                std::string suit = cardData["suit"];
                int value = cardData["value"];
                bool faceUp = cardData["faceUp"];
                
                // Convert value back to string
                std::string valueStr;
                if (value == 1) valueStr = "ace";
                else if (value == 11) valueStr = "jack";
                else if (value == 12) valueStr = "queen";
                else if (value == 13) valueStr = "king";
                else valueStr = std::to_string(value);
                
                // Create card
                std::string imagePath = "assets/cards/" + valueStr + "_of_" + suit + ".png";
                Card card(suit, valueStr, imagePath);
                if (faceUp) card.flip();
                tableau[i].push_back(card);
            }
        }
        
        // Load foundation piles
        for (size_t i = 0; i < gameState["foundations"].size(); i++) {
            for (const auto& cardData : gameState["foundations"][i]) {
                std::string suit = cardData["suit"];
                int value = cardData["value"];
                bool faceUp = cardData["faceUp"];
                
                // Convert value back to string
                std::string valueStr;
                if (value == 1) valueStr = "ace";
                else if (value == 11) valueStr = "jack";
                else if (value == 12) valueStr = "queen";
                else if (value == 13) valueStr = "king";
                else valueStr = std::to_string(value);
                
                // Create card
                std::string imagePath = "assets/cards/" + valueStr + "_of_" + suit + ".png";
                Card card(suit, valueStr, imagePath);
                if (faceUp) card.flip();
                foundations[i].push_back(card);
            }
        }
        
        // Load stock pile
        for (const auto& cardData : gameState["stock"]) {
            std::string suit = cardData["suit"];
            int value = cardData["value"];
            bool faceUp = cardData["faceUp"];
            
            // Convert value back to string
            std::string valueStr;
            if (value == 1) valueStr = "ace";
            else if (value == 11) valueStr = "jack";
            else if (value == 12) valueStr = "queen";
            else if (value == 13) valueStr = "king";
            else valueStr = std::to_string(value);
            
            // Create card
            std::string imagePath = "assets/cards/" + valueStr + "_of_" + suit + ".png";
            Card card(suit, valueStr, imagePath);
            if (faceUp) card.flip();
            stock.push_back(card);
        }
        
        // Load waste pile
        for (const auto& cardData : gameState["waste"]) {
            std::string suit = cardData["suit"];
            int value = cardData["value"];
            bool faceUp = cardData["faceUp"];
            
            // Convert value back to string
            std::string valueStr;
            if (value == 1) valueStr = "ace";
            else if (value == 11) valueStr = "jack";
            else if (value == 12) valueStr = "queen";
            else if (value == 13) valueStr = "king";
            else valueStr = std::to_string(value);
            
            // Create card
            std::string imagePath = "assets/cards/" + valueStr + "_of_" + suit + ".png";
            Card card(suit, valueStr, imagePath);
            if (faceUp) card.flip();
            waste.push_back(card);
        }
        
#if DEBUG == 1
        std::cout << "Game loaded successfully" << std::endl;
#endif
        return true;
    } catch (const std::exception& e) {
#if DEBUG == 1
        std::cerr << "Error loading game: " << e.what() << std::endl;
#endif
        return false;
    }
#endif
}

void Solitaire::handleMenuClick(Vector2 pos) {
    // Check if clicking on File menu
    if (pos.y <= MENU_HEIGHT && 
        pos.x >= MENU_FILE_X && 
        pos.x <= MENU_FILE_X + MENU_FILE_WIDTH) {
        menuOpen = !menuOpen;
        return;
    }

    // If menu is open, check menu items
    if (menuOpen) {
        if (pos.y >= MENU_HEIGHT && 
            pos.y <= MENU_HEIGHT + MENU_DROPDOWN_HEIGHT && 
            pos.x >= MENU_FILE_X && 
            pos.x <= MENU_FILE_X + MENU_FILE_WIDTH) {
            int itemIndex = (pos.y - MENU_HEIGHT) / MENU_ITEM_HEIGHT;
            switch (itemIndex) {
                case 0: // New Game
                    resetGame();
                    break;
                case 1: // Save
                    saveGame();
                    break;
                case 2: // Load
                    loadGame();
                    break;
                case 3: // Quit
                    // Clean up resources
                    Card::unloadCardBack();
                    // Close the window
                    CloseWindow();
                    // Exit the application
                    exit(0);
                    break;
            }
            menuOpen = false;
        } else {
            menuOpen = false;
        }
    }
}

void Solitaire::handleRightClick(Vector2 pos) {
    // Check if clicking on stock pile area
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    Rectangle stockRect = { stockX, stockY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
    
    if (CheckCollisionPointRec(pos, stockRect) && lastDrawnCard != nullptr && !waste.empty() && lastDrawnCard == &waste.back()) {
        // Move the last drawn card back to stock
        Card card = waste.back();
        waste.pop_back();
        card.flip();  // Flip face down
        stock.push_back(card);
        lastDrawnCard = nullptr;  // Reset last drawn card after undo

        // If there are more cards in waste, make sure the new top card is face up
        if (!waste.empty() && !waste.back().isFaceUp()) {
            waste.back().flip();
        }
#if DEBUG == 1
        std::cout << "Undo: Moved last drawn card back to stock pile" << std::endl;
#endif
    }
}

void Solitaire::update() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 pos = GetMousePosition();
        
        // Check menu first
        handleMenuClick(pos);
        
        // Then check game interactions
        if (!menuOpen) {
            handleMouseDown(pos);
        }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        handleMouseUp(GetMousePosition());
    }
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && GetMouseDelta().x == 0 && GetMouseDelta().y == 0) {
        // Check for double click (mouse hasn't moved between clicks)
        static double lastClickTime = 0;
        double currentTime = GetTime();
        if (currentTime - lastClickTime < 0.3) {  // 300ms threshold for double click
            handleDoubleClick(GetMousePosition());
        }
        lastClickTime = currentTime;
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        handleRightClick(GetMousePosition());
    }

    if (checkWin()) {
        gameWon = true;
    }
}

void Solitaire::draw() {
    // Check if window is still open
    if (!IsWindowReady()) {
        return;
    }

    // Draw background
    ClearBackground(GREEN);

    // Draw foundation piles (moved down by MENU_HEIGHT)
    for (int i = 0; i < 4; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 10 * SCALE_FACTOR + MENU_HEIGHT;  // Add MENU_HEIGHT
        if (!foundations[i].empty()) {
            // If this foundation pile is the source of the dragged card, show the card underneath
            if (draggedSourcePile == &foundations[i] && foundations[i].size() > 1) {
                foundations[i][foundations[i].size() - 2].setPosition(x, y);
                foundations[i][foundations[i].size() - 2].draw();
            } else if (draggedSourcePile != &foundations[i]) {
                // Otherwise show the top card if it's not being dragged
                foundations[i].back().setPosition(x, y);
                foundations[i].back().draw();
            }
        } else {
            // Draw empty foundation slot
            DrawRectangle(x, y, CARD_WIDTH, CARD_HEIGHT, WHITE);
            DrawRectangleLines(x, y, CARD_WIDTH, CARD_HEIGHT, BLACK);
        }
    }

    // Draw tableau piles (moved down by MENU_HEIGHT)
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR + MENU_HEIGHT;  // Add MENU_HEIGHT
         
        for (size_t j = 0; j < tableau[i].size(); j++) {
            // Skip drawing cards that are being dragged
            if (draggedSourcePile == &tableau[i] && j >= draggedStartIndex) {
                continue;
            }
            tableau[i][j].setPosition(x, y + j * CARD_SPACING);
            tableau[i][j].draw();
        }
    }

    // Draw stock pile
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    if (!stock.empty()) {
        // Skip drawing the stock card if it's being dragged
        if (draggedSourcePile != &stock) {
            // Draw a stack of cards for the stock pile
            int numCards = stock.size();
            int maxVisibleCards = 5;  // Maximum number of cards to show in the stack
            int cardsToShow = std::min(numCards, maxVisibleCards);
            
            for (int i = 0; i < cardsToShow; i++) {
                // Calculate offset for each card in the stack
                float offsetX = i * 2;  // Small horizontal offset
                float offsetY = i * 2;  // Small vertical offset
                
                // Get the card from the end of the stock pile
                Card& card = stock[stock.size() - 1 - i];
                card.setPosition(stockX + offsetX, stockY + offsetY);
                card.draw();
            }
            
            // Always show the total number of cards
            DrawText(TextFormat("%d", numCards), 
                    stockX + CARD_WIDTH - 55, stockY + CARD_HEIGHT - 20, 20, BLACK);
        }
    }

    // Draw waste pile (no change needed)
    float wasteX = stockX + TABLEAU_SPACING;
    float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    if (!waste.empty()) {
        // Skip drawing the waste card if it's being dragged
        if (draggedSourcePile != &waste) {
            waste.back().setPosition(wasteX, wasteY);
            waste.back().draw();
        }
    }

    // Draw dragged cards
    if (!draggedCards.empty()) {
        Vector2 mousePos = GetMousePosition();
        for (size_t i = 0; i < draggedCards.size(); i++) {
            // Apply the drag offset to maintain the relative position
            draggedCards[i].setPosition(
                mousePos.x - dragOffset.x,
                mousePos.y - dragOffset.y + i * CARD_SPACING
            );
            draggedCards[i].draw();
        }
    }

    // Draw all UI elements last
    // Draw menu bar
    DrawRectangle(0, 0, WINDOW_WIDTH, MENU_HEIGHT, DARKGRAY);
    DrawText("File", MENU_FILE_X + MENU_TEXT_PADDING, MENU_TEXT_PADDING, 20, WHITE);
    
    // Draw menu items when File is clicked
    if (menuOpen) {
        DrawRectangle(MENU_FILE_X, MENU_HEIGHT, MENU_FILE_WIDTH, MENU_DROPDOWN_HEIGHT, DARKGRAY);
        DrawText("New Game", MENU_FILE_X + MENU_TEXT_PADDING, MENU_HEIGHT + MENU_TEXT_PADDING, 20, WHITE);
        DrawText("Save", MENU_FILE_X + MENU_TEXT_PADDING, MENU_HEIGHT + MENU_ITEM_HEIGHT + MENU_TEXT_PADDING, 20, WHITE);
        DrawText("Load", MENU_FILE_X + MENU_TEXT_PADDING, MENU_HEIGHT + MENU_ITEM_HEIGHT * 2 + MENU_TEXT_PADDING, 20, WHITE);
        DrawText("Quit", MENU_FILE_X + MENU_TEXT_PADDING, MENU_HEIGHT + MENU_ITEM_HEIGHT * 3 + MENU_TEXT_PADDING, 20, WHITE);
    }

    if (gameWon) {
        DrawText("You Win!", WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, 40, WHITE);
    }
} 