#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"
#include "model/ChessModel.h"

class Rook : public Piece {
public:
    Rook(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // ROOK_H