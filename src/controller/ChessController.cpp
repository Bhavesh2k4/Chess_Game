#include <iostream>
#include <limits> 
#include "ChessController.h"
#include "core/Utils.h" // Include for SAN

ChessController::ChessController(ChessModel* m, ChessView* v) {
    this->model = m;
    this->view = v;
}

void ChessController::run() {
    int choice;
    view->displayMenu();
    std::cin >> choice;
    
    if (choice == 1) {
        model->setupStartingPosition();
    } else if (choice == 2) {
        std::string fen = view->getFEN();
        model->setupFromFEN(fen);
        
        // Check if user wants to override the FEN active color
        char moveChoice = view->getFirstMoveChoice();
        if (moveChoice == 'w' || moveChoice == 'W') {
            model->setWhiteToMove(true);
        } else if (moveChoice == 'b' || moveChoice == 'B') {
            model->setWhiteToMove(false);
        }
    } else {
        std::cout << "Invalid choice. Using default.\n";
        model->setupStartingPosition();
    }
    
    // Main game loop
    bool gameRunning = true;
    while (gameRunning) {
        view->displayBoard(model);

        if (model->isInCheck()) {
             std::cout << (model->isWhiteToMove() ? "White" : "Black") << " is in Check!\n";
        }
        
        // Get move from user
        Move move = view->getMove();
        
        // Check if user wants to quit
        if (move.from.row == -1) {
            gameRunning = false;
            continue;
        }
        
        // Show valid moves for the selected piece
        if (model->getPiece(move.from.row, move.from.col) != nullptr) {
            view->displayValidMoves(model, move.from);
        }
        
        // Make the move
        std::string sanAttempt = Utils::moveToSAN(move, *model); 
        bool moveSuccessful = model->makeMove(move);
        if (!moveSuccessful) {
            std::cout << "Move made: " << sanAttempt;
            // Add check/mate suffix based on state after the move
            if (model->getIsCheckmate()) {
                std::cout << "#"; // Checkmate
            } else if (model->isInCheck()) {
                 std::cout << "+"; // Check
            }
             std::cout << std::endl;

            if (model->isGameOver()) {
                view->displayBoard(model); 
                if (model->getIsCheckmate()) {
                    std::cout << "\nCHECKMATE! " << (model->isWhiteToMove() ? "Black" : "White") << " wins!\n";
                } else if (model->getIsStalemate()) {
                    std::cout << "\nSTALEMATE! Game is a draw.\n";
                }
                gameRunning = false;
            }
        } else {
            view->displayError("Invalid move. Please try again.");
        }
    }
}