#include "Utils.h"
#include "model/ChessModel.h"
#include "model/pieces/Piece.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

std::string Utils::positionToString(const Position& pos) {
    if (!pos.isValid()) return "??";
    return std::string(1, 'a' + pos.col) + std::string(1, '1' + pos.row);
}

// Returns SAN disambiguation string when multiple pieces can reach the same target
std::string Utils::getDisambiguation(const Move& move, const ChessModel& model) {
    Piece* movingPiece = model.getPiece(move.from.row, move.from.col);
    if (!movingPiece || movingPiece->type == 'P' || movingPiece->type == 'K') {
        return "";
    }

    char pieceType = movingPiece->type;
    bool pieceColor = movingPiece->isWhite;
    std::string disambiguation = "";
    bool fileNeeded = false;
    bool rankNeeded = false;
    bool collisionFound = false;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (r == move.from.row && c == move.from.col) continue;

            Piece* otherPiece = model.getPiece(r, c);
            if (otherPiece && otherPiece->type == pieceType && otherPiece->isWhite == pieceColor) {
                std::vector<Position> pseudoMoves = otherPiece->getPossibleMoves({r, c}, const_cast<ChessModel*>(&model));
                bool canReachTarget = false;
                for (const auto& pseudoTo : pseudoMoves) {
                    if (pseudoTo == move.to) {
                        canReachTarget = true;
                        break;
                    }
                }

                if (canReachTarget) {
                    collisionFound = true;
                    if (c != move.from.col) {
                        fileNeeded = true;
                    } else {
                        rankNeeded = true;
                    }
                }
            }
            if (fileNeeded && rankNeeded) goto end_disambiguation_loop;
        }
    }

end_disambiguation_loop:

    if (fileNeeded) {
        disambiguation += ('a' + move.from.col);
    }
    if (rankNeeded) {
        disambiguation += ('1' + move.from.row);
    } else if (collisionFound && !fileNeeded && !rankNeeded) {
        disambiguation += ('a' + move.from.col);
    }

    return disambiguation;
}

// Converts a Move into Standard Algebraic Notation (SAN) without check/mate suffix
std::string Utils::moveToSAN(const Move& move, const ChessModel& model) {
    Piece* piece = model.getPiece(move.from.row, move.from.col);
    Piece* captured = model.getPiece(move.to.row, move.to.col);
    Position* epTarget = model.getEnPassantTarget();

    if (!piece) return "InvalidMove(NoPiece)";

    if (piece->type == 'K' && abs(move.from.col - move.to.col) == 2) {
        return (move.to.col > move.from.col) ? "O-O" : "O-O-O";
    }

    std::string san = "";
    bool isPawn = (piece->type == 'P');
    bool isEpCapture = isPawn && epTarget && move.to == *epTarget;
    bool isCapture = (captured != nullptr) || isEpCapture;

    if (!isPawn) {
        san += piece->type;
        san += getDisambiguation(move, model);
    } else if (isCapture) {
        san += ('a' + move.from.col);
    }

    if (isCapture) {
        san += 'x';
    }

    san += positionToString(move.to);

    if (isPawn && (move.to.row == 0 || move.to.row == 7)) {
        san += "=Q";
    }

    return san;
}
