#include <iostream>
#include "ChessView.h"

void ChessView::displayBoard(ChessModel* model) {
    std::cout << "\n  +----+----+----+----+----+----+----+----+\n";
    
    for (int row = 7; row >= 0; row--) {
        std::cout << (row + 1) << " |";
        
        for (int col = 0; col < 8; col++) {
            Piece* piece = model->getPiece(row, col);
            
            if (piece == nullptr) {
                std::cout << "    |";
            } else {
                char pieceType = piece->type;
                bool isWhite = piece->isWhite;
                std::string displayChar = " ";
                if (pieceType == 'K') {
                    displayChar = isWhite ? " K" : " k";
                } else if (pieceType == 'Q') {
                    displayChar = isWhite ? " Q" : " q";
                } else if (pieceType == 'R') {
                    displayChar = isWhite ? " R" : " r";
                } else if (pieceType == 'B') {
                    displayChar = isWhite ? " B" : " b";
                } else if (pieceType == 'N') {
                    displayChar = isWhite ? " N" : " n";
                } else if (pieceType == 'P') {
                    displayChar = isWhite ? " P" : " p";
                }

                std::cout << " " << displayChar << " |";  // Print the string
            }
        }
        
        std::cout << "\n  +----+----+----+----+----+----+----+----+\n";
    }
    
    std::cout << "     a    b    c    d    e    f    g    h\n\n";
    
    // Display whose turn it is
    std::cout << (model->isWhiteToMove() ? "White" : "Black") << " to move\n\n";
}

void ChessView::displayMenu() {
    std::cout << "=== Chess Board ===\n";
    std::cout << "1. Use default starting position (White moves first)\n";
    std::cout << "2. Enter FEN notation\n";
    std::cout << "Enter your choice (1-2): ";
}

std::string ChessView::getFEN() {
    std::string fen;
    std::cout << "Enter FEN notation: ";
    std::cin.ignore(); // Clear input buffer
    std::getline(std::cin, fen);
    return fen;
}

char ChessView::getFirstMoveChoice() {
    char choice;
    std::cout << "Who moves first? (w for White, b for Black): ";
    std::cin >> choice;
    return choice;
}

// Get move from user
Move ChessView::getMove() {
    std::string moveStr;
    std::cout << "Enter move (e.g., e2e4) or 'q' to quit: ";
    std::cin >> moveStr;
    
    if (moveStr == "q" || moveStr == "quit") {
        return Move(Position(-1, -1), Position(-1, -1));
    }
    
    if (moveStr.length() != 4) {
        std::cout << "Invalid move format. Please use format like 'e2e4'.\n";
        return Move(Position(-1, -1), Position(-1, -1));
    }
    
    int fromCol = moveStr[0] - 'a';
    int fromRow = moveStr[1] - '1';
    int toCol = moveStr[2] - 'a';
    int toRow = moveStr[3] - '1';
    
    return Move(Position(fromRow, fromCol), Position(toRow, toCol));
}

void ChessView::displayValidMoves(ChessModel* model, Position pos) {
    std::vector<Position> validMoves = model->getValidMoves(pos);
    std::cout << "Valid moves for piece at " << char(pos.col + 'a') << (pos.row + 1) << ": ";
    for (Position& move : validMoves) {
        std::cout << char(move.col + 'a') << (move.row + 1) << " ";
    }
    std::cout << std::endl;
}

void ChessView::displayError(const std::string& message) {
    std::cout << "Error: " << message << std::endl;
}