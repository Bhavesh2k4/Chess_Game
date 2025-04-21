#ifndef KING_H
#define KING_H

#include "Piece.h"
#include "model/ChessModel.h"

class King : public Piece {
public:
    King(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // KING_H