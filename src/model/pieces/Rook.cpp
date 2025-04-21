#include "Rook.h"

Rook::Rook(bool white) : Piece('R', white) {}

std::vector<Position> Rook::getPossibleMoves(Position pos, const ChessModel* model) {
    std::vector<Position> moves;
    
    // Function to add a move if valid
    auto addIfValid = [&moves](int row, int col) {
        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            moves.push_back(Position(row, col));
        }
    };
    
    // Function to add moves in a direction until blocked
    auto addMovesInDirection = [model, addIfValid, pos, this](int rowDir, int colDir) {
        int row = pos.row + rowDir;
        int col = pos.col + colDir;
        while (row >= 0 && row < 8 && col >= 0 && col < 8) {
            if (model->getPiece(row, col) == nullptr) {
                addIfValid(row, col);
                row += rowDir;
                col += colDir;
            } else if (model->getPiece(row, col)->isWhite != this->isWhite) {
                addIfValid(row, col);
                break;
            } else {
                break;
            }
        }
    };
    
    // Rook moves horizontally and vertically
    addMovesInDirection(1, 0);
    addMovesInDirection(-1, 0);
    addMovesInDirection(0, 1);
    addMovesInDirection(0, -1);
    
    return moves;
}