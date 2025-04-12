#include "Solitaire.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <iostream>

// Define the scaled constants
int CARD_WIDTH;
int CARD_HEIGHT;
int CARD_SPACING;
int TABLEAU_SPACING;
int WINDOW_WIDTH;
int WINDOW_HEIGHT;
int MENU_HEIGHT;

Solitaire::Solitaire() {
    srand(time(nullptr));
    
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
    
    // Load card back texture
    Card::loadCardBack("assets/cards/card_back_red.png");
    resetGame();
}

Solitaire::~Solitaire() {
    // Clean up resources
    Card::unloadCardBack();
}

void Solitaire::resetGame() {
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

    loadCards();
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
    // Check tableau piles
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR;
        
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

    // Check foundation piles
    for (int i = 0; i < 4; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 10 * SCALE_FACTOR;
        
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

    // Check stock pile
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    Rectangle stockRect = { stockX, stockY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
    if (CheckCollisionPointRec(pos, stockRect)) {
        return &stock;
    }

    // Check waste pile
    float wasteX = stockX + TABLEAU_SPACING;  // Use TABLEAU_SPACING instead of fixed value
    float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;  // 20 pixels from bottom
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
#ifdef DEBUG
        std::cout << "Checking if card can be placed on empty tableau. Card value: " << card.getValue() 
                  << ", Card string value: " << card.getSuit() << " " << card.getValue() << std::endl;
#endif
        return card.getValue() == 13;  // 13 represents king
    }
    
    const Card& topCard = targetPile.back();
    // Check if colors are different and values are in sequence
    return (card.isRed() != topCard.isRed()) && 
           (card.getValue() == topCard.getValue() - 1);
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
    if (startIndex < 0 || startIndex >= sourcePile.size()) {
        return false;
    }

    if (endIndex == -1) {
        endIndex = sourcePile.size() - 1;
    }

    // Move cards
    for (int i = startIndex; i <= endIndex; i++) {
        targetPile.push_back(sourcePile[i]);
    }
    sourcePile.erase(sourcePile.begin() + startIndex, sourcePile.begin() + endIndex + 1);

    // Flip the new top card of the source pile if it exists
    if (!sourcePile.empty() && !sourcePile.back().isFaceUp()) {
        sourcePile.back().flip();
    }

    return true;
}

void Solitaire::handleMouseDown(Vector2 pos) {
    // Check stock pile first
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
    Rectangle stockRect = { stockX, stockY, static_cast<float>(CARD_WIDTH), static_cast<float>(CARD_HEIGHT) };
    if (CheckCollisionPointRec(pos, stockRect)) {
        if (stock.empty()) {
            // Move all cards from waste back to stock
            while (!waste.empty()) {
                Card card = waste.back();
                waste.pop_back();
                card.flip();  // Flip face down
                stock.push_back(card);
            }
        } else {
            // Deal a card from stock to waste
            Card card = stock.back();
            stock.pop_back();
            card.flip();  // Flip face up
            waste.push_back(card);
            lastDealTime = GetTime();
        }
        return;
    }

    // Find the card that was clicked
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR;
        
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

    // Check waste pile if no card was found in tableau
    if (draggedCards.empty() && !waste.empty()) {
        float wasteX = stockX + TABLEAU_SPACING;  // Use TABLEAU_SPACING instead of fixed value
        float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;  // 20 pixels from bottom
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
#ifdef DEBUG
        std::cout << "No valid target pile found at position (" << pos.x << ", " << pos.y << ")" << std::endl;
#endif
        // Return cards to original position
        if (draggedSourcePile == &waste) {
            float stockX = 50 * SCALE_FACTOR;
            float wasteX = stockX + TABLEAU_SPACING;
            float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
            draggedCards[0].setPosition(wasteX, wasteY);
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
#ifdef DEBUG
        std::cout << "Target pile is the same as source pile, returning cards" << std::endl;
#endif
        draggedCards.clear();
        draggedSourcePile = nullptr;
        return;
    }

    // Check if we can move to tableau
    if (canMoveToTableau(draggedCards[0], *targetPile)) {
#ifdef DEBUG
        std::cout << "Moving card to tableau pile" << std::endl;
#endif
        // If dragging from waste pile, only move the top card
        if (draggedSourcePile == &waste) {
            moveCards(*draggedSourcePile, *targetPile, draggedSourcePile->size() - 1);
        } else {
            moveCards(*draggedSourcePile, *targetPile, draggedStartIndex);
        }
    }
    // Check if we can move to foundation (only single cards can be moved to foundation)
    else if (draggedCards.size() == 1 && canMoveToFoundation(draggedCards[0], *targetPile)) {
#ifdef DEBUG
        std::cout << "Moving card to foundation pile" << std::endl;
#endif
        moveCards(*draggedSourcePile, *targetPile, draggedSourcePile->size() - 1);
    } else {
#ifdef DEBUG
        std::cout << "Invalid move, returning cards to original position" << std::endl;
#endif
        // Return cards to original position
        if (draggedSourcePile == &waste) {
            float stockX = 50 * SCALE_FACTOR;
            float wasteX = stockX + TABLEAU_SPACING;
            float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
            draggedCards[0].setPosition(wasteX, wasteY);
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

void Solitaire::update() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 pos = GetMousePosition();
        // Check if clicking on stock pile
        float stockX = 50 * SCALE_FACTOR;
        float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;
        Rectangle stockRect = { 
            static_cast<float>(stockX), 
            static_cast<float>(stockY), 
            static_cast<float>(CARD_WIDTH), 
            static_cast<float>(CARD_HEIGHT) 
        };
        
        if (CheckCollisionPointRec(pos, stockRect)) {
            // Deal cards from stock to waste
            if (!stock.empty()) {
                // Move top card from stock to waste
                Card card = stock.back();
                stock.pop_back();
                card.flip();  // Flip the card face up
                waste.push_back(card);
                lastDealTime = GetTime();  // Record when the card was dealt
            } else if (!waste.empty()) {
                // If stock is empty, move all cards from waste back to stock
                while (!waste.empty()) {
                    Card card = waste.back();
                    waste.pop_back();
                    card.flip();  // Flip the card face down
                    stock.push_back(card);
                }
            }
        } else {
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

    if (checkWin()) {
        gameWon = true;
    }
}

void Solitaire::draw() {
    // Draw background
    ClearBackground(GREEN);

    // Draw foundation piles (top left)
    for (int i = 0; i < 4; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 10 * SCALE_FACTOR;  // Removed MENU_HEIGHT offset
        if (!foundations[i].empty()) {
            foundations[i].back().setPosition(x, y);
            foundations[i].back().draw();
        } else {
            // Draw empty foundation slot
            DrawRectangle(x, y, CARD_WIDTH, CARD_HEIGHT, WHITE);
            DrawRectangleLines(x, y, CARD_WIDTH, CARD_HEIGHT, BLACK);
        }
    }

    // Draw tableau piles (lower position)
    for (int i = 0; i < 7; i++) {
        float x = 50 * SCALE_FACTOR + i * TABLEAU_SPACING;
        float y = 200 * SCALE_FACTOR;  // Removed MENU_HEIGHT offset
               
        for (size_t j = 0; j < tableau[i].size(); j++) {
            // Skip drawing cards that are being dragged
            if (draggedSourcePile == &tableau[i] && j >= draggedStartIndex) {
                continue;
            }
            tableau[i][j].setPosition(x, y + j * CARD_SPACING);
            tableau[i][j].draw();
            
            // Draw collision rectangle for each card
            DrawRectangleLines(x, y + j * CARD_SPACING, CARD_WIDTH, CARD_HEIGHT, BLUE);
        }
        // Draw empty tableau slot if pile is empty
        if (tableau[i].empty()) {
            DrawRectangle(x, y, CARD_WIDTH, CARD_HEIGHT, WHITE);
            DrawRectangleLines(x, y, CARD_WIDTH, CARD_HEIGHT, BLACK);
        }
    }

    // Draw stock pile (bottom left)
    float stockX = 50 * SCALE_FACTOR;
    float stockY = WINDOW_HEIGHT - CARD_HEIGHT - 20;  // 20 pixels from bottom
    if (!stock.empty()) {
        // Skip drawing the stock card if it's being dragged
        if (draggedSourcePile != &stock) {
            stock.back().setPosition(stockX, stockY);
            stock.back().draw();
        }
    }

    // Draw waste pile (next to stock, using TABLEAU_SPACING)
    float wasteX = stockX + TABLEAU_SPACING;  // Use TABLEAU_SPACING instead of fixed value
    float wasteY = WINDOW_HEIGHT - CARD_HEIGHT - 20;  // 20 pixels from bottom
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

    if (gameWon) {
        DrawText("You Win!", WINDOW_WIDTH/2 - 100, WINDOW_HEIGHT/2, 40, WHITE);
    }
} 