#include "King.h"

King::King(bool white) : Piece('K', white) {}

std::vector<Position> King::getPossibleMoves(Position pos, const ChessModel* model) {
    std::vector<Position> moves;
    
    // Function to check if position is empty or has an enemy piece
    auto canMoveTo = [model, this](int row, int col) -> bool {
        Piece* piece = model->getPiece(row, col);
        return piece == nullptr || piece->isWhite != this->isWhite;
    };
    
    // Function to add a move if valid
    auto addIfValid = [&moves](int row, int col) {
        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            moves.push_back(Position(row, col));
        }
    };
    
    // Regular king moves (one square in any direction)
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            int row = pos.row + i;
            int col = pos.col + j;
            if (row >= 0 && row < 8 && col >= 0 && col < 8 && canMoveTo(row, col)) {
                addIfValid(row, col);
            }
        }
    }
    
    // Castling
    if (!hasMoved) {
        // Kingside castling
        if (model->getCastlingRight(isWhite ? 0 : 2)) {
            if (model->getPiece(pos.row, pos.col + 1) == nullptr &&
                model->getPiece(pos.row, pos.col + 2) == nullptr) {
                // Check if the king is not in check and doesn't pass through check
                bool canCastle = true;
                // TODO: Implement proper check for castling through check
                if (canCastle) {
                    addIfValid(pos.row, pos.col + 2);
                }
            }
        }
        
        // Queenside castling
        if (model->getCastlingRight(isWhite ? 1 : 3)) {
            if (model->getPiece(pos.row, pos.col - 1) == nullptr &&
                model->getPiece(pos.row, pos.col - 2) == nullptr &&
                model->getPiece(pos.row, pos.col - 3) == nullptr) {
                // Check if the king is not in check and doesn't pass through check
                bool canCastle = true;
                // TODO: Implement proper check for castling through check
                if (canCastle) {
                    addIfValid(pos.row, pos.col - 2);
                }
            }
        }
    }
    
    return moves;
}