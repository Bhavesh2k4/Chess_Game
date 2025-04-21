#ifndef FENUTILS_H
#define FENUTILS_H

#include <string>

class ChessModel;
class Piece;

class FenUtils {
public:
    static bool parseFen(const std::string& fen, ChessModel& model);

    static std::string generateFen(const ChessModel& model);

private:
    static Piece* createPieceFromChar(char typeChar);
};

#endif 
