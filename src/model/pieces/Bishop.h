#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"
#include "model/ChessModel.h"

class Bishop : public Piece {
public:
    Bishop(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // BISHOP_H