#include "Piece.h"

Piece::Piece(char t, bool white) {
    this->type = t;
    this->isWhite = white;
    this->hasMoved = false;
}