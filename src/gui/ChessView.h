#ifndef CHESS_VIEW_H
#define CHESS_VIEW_H

#include <string>
#include "model/ChessModel.h"
#include "model/Move.h"

class ChessView {
public:
    void displayBoard(ChessModel* model);
    void displayMenu();
    std::string getFEN();
    char getFirstMoveChoice();
    Move getMove();
    void displayValidMoves(ChessModel* model, Position pos);
    void displayError(const std::string& message);
};

#endif