#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "model/Position.h"
#include "model/Move.h"

class ChessModel;

class Utils {
public:
    static std::string positionToString(const Position& pos);
    static std::string moveToSAN(const Move& move, const ChessModel& model);

private:
    static std::string getDisambiguation(const Move& move, const ChessModel& model);
};

#endif
