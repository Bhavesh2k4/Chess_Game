#ifndef POSITION_H
#define POSITION_H

class Position {
public:
    int row;
    int col;
    
    Position(int r, int c);
    bool isValid() const;
    bool operator==(const Position& other) const;
};

#endif // POSITION_H