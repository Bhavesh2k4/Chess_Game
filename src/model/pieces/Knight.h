#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"
#include "model/ChessModel.h"

class Knight : public Piece {
public:
    Knight(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // KNIGHT_H