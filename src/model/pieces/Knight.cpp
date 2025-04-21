#include "Knight.h"

Knight::Knight(bool white) : Piece('N', white) {}

std::vector<Position> Knight::getPossibleMoves(Position pos, const ChessModel* model) {
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
    
    // Knight moves
    const int knightMoves[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    for (int i = 0; i < 8; i++) {
        int row = pos.row + knightMoves[i][0];
        int col = pos.col + knightMoves[i][1];
        if (row >= 0 && row < 8 && col >= 0 && col < 8 && canMoveTo(row, col)) {
            addIfValid(row, col);
        }
    }
    
    return moves;
}