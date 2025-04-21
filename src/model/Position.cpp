#include "Position.h"

Position::Position(int r, int c) {
    this->row = r;
    this->col = c;
}

bool Position::isValid() const{
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

bool Position::operator==(const Position& other) const {
    return row == other.row && col == other.col;
}