#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include "model/Position.h"

// Forward declaration
class ChessModel;

class Piece {
public:
    char type;
    bool isWhite;
    bool hasMoved;
    
    Piece(char t, bool white);
    virtual ~Piece() = default;
    
    // Virtual function to be overridden by each piece type
    virtual std::vector<Position> getPossibleMoves(Position pos, const ChessModel* model) = 0;
};

#endif // PIECE_H