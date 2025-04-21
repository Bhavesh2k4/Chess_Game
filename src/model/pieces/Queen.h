#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.h"
#include "model/ChessModel.h"

class Queen : public Piece {
public:
    Queen(bool white);
    std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) override;
};

#endif // QUEEN_H