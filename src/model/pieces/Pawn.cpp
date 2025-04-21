#include "Pawn.h"

Pawn::Pawn(bool white) : Piece('P', white) {}

std::vector<Position> Pawn::getPossibleMoves(Position pos, const ChessModel* model) {
    std::vector<Position> moves;

    // Function to check if position has an enemy piece (for pawn captures)
    auto hasEnemy = [model, this](int row, int col) -> bool {
        if (row < 0 || row >= 8 || col < 0 || col >= 8) return false; 
        Piece* piece = model->getPiece(row, col);
        return piece != nullptr && piece->isWhite != this->isWhite;
    };

    // Function to add a move if valid
    auto addIfValid = [&moves](int row, int col) {
        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            moves.push_back(Position(row, col));
        }
    };

    // Function to check if a square is empty
    auto isEmpty = [model](int row, int col) -> bool {
        if (row < 0 || row >= 8 || col < 0 || col >= 8) return false; 
        return model->getPiece(row, col) == nullptr;
    };

    // Pawns move differently based on color
    int dir = isWhite ? 1 : -1;
    int startRow = isWhite ? 1 : 6;

    // Forward one square
    int nextRow = pos.row + dir;
    if (isEmpty(nextRow, pos.col)) {
        addIfValid(nextRow, pos.col);

        // Forward two squares from starting position
        if (pos.row == startRow && isEmpty(pos.row + 2 * dir, pos.col)) {
            addIfValid(pos.row + 2 * dir, pos.col);
        }
    }

    // Capture diagonally
    if (hasEnemy(nextRow, pos.col - 1)) {
        addIfValid(nextRow, pos.col - 1);
    }
    if (hasEnemy(nextRow, pos.col + 1)) {
        addIfValid(nextRow, pos.col + 1);
    }

    // En passant
    Position* enPassantTarget = model->getEnPassantTarget();
    if (enPassantTarget != nullptr) {
        int expectedPawnRow = enPassantTarget->row - dir;
        if (pos.row == expectedPawnRow) {

            if (abs(pos.col - enPassantTarget->col) == 1) {
                addIfValid(enPassantTarget->row, enPassantTarget->col);
            }
        }
    }

    return moves;
}