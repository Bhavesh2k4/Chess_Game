#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"
#include "model/ChessModel.h"

class Pawn : public Piece {
public:
    Pawn(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // PAWN_H