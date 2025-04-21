#ifndef MOVE_H
#define MOVE_H

#include "Position.h"

class Move {
public:
    Position from;
    Position to;
    
    Move(Position f, Position t);
};

#endif // MOVE_H