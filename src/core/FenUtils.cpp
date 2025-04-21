#include "core/FenUtils.h"
#include "model/ChessModel.h"
#include "model/Position.h"
#include "model/pieces/Piece.h"
#include "model/pieces/Pawn.h"
#include "model/pieces/Knight.h"
#include "model/pieces/Bishop.h"
#include "model/pieces/Rook.h"
#include "model/pieces/Queen.h"
#include "model/pieces/King.h"

#include <sstream>
#include <cctype>
#include <QDebug>

Piece* FenUtils::createPieceFromChar(char typeChar) {
    bool isWhite = std::isupper(typeChar);
    char upperType = std::toupper(typeChar);
    switch(upperType) {
        case 'P': return new Pawn(isWhite);
        case 'N': return new Knight(isWhite);
        case 'B': return new Bishop(isWhite);
        case 'R': return new Rook(isWhite);
        case 'Q': return new Queen(isWhite);
        case 'K': return new King(isWhite);
        default:
            qWarning("Warning: Invalid piece type '%c' in FEN parsing.", typeChar);
            return nullptr;
    }
}

bool FenUtils::parseFen(const std::string& fen, ChessModel& model) {
    model.clearBoard();
    model.clearCapturedPieces(); 
    model.whiteToMove = true;
    for (int i = 0; i < 4; ++i) model.castlingRights[i] = false;
    delete model.enPassantTarget;
    model.enPassantTarget = nullptr;

    std::istringstream fenStream(fen);
    std::string segment;

    if (!std::getline(fenStream, segment, ' ')) return false;
    int row = 7;
    int col = 0;
    for (char c : segment) {
        if (c == '/') {
            if (col != 8) qWarning("FEN Parsing Warning: Row %d did not sum to 8 columns.", row + 1);
            row--;
            col = 0;
            if (row < 0) return false;
        } else if (std::isdigit(c)) {
            int emptySquares = c - '0';
            if (col + emptySquares > 8) return false;
            col += emptySquares;
        } else if (std::isalpha(c)) {
            if (col >= 8) return false;
            Piece* piece = createPieceFromChar(c);
            if (piece) {
                model.board[row][col] = piece;
            } else {
                qWarning("FEN Parsing Error: Invalid piece character '%c'", c);
            }
            col++;
        } else {
             qWarning("FEN Parsing Error: Invalid character '%c' in piece placement segment.", c);
             return false;
        }
    }
    if (row != 0 || col != 8) {
        qWarning("FEN Parsing Warning: Did not end at row 0, col 8. Final pos: (%d,%d)", row, col);
    }

    if (!std::getline(fenStream, segment, ' ')) return false;
    if (segment == "w") {
        model.whiteToMove = true;
    } else if (segment == "b") {
        model.whiteToMove = false;
    } else {
        qWarning("FEN Parsing Error: Invalid active color '%s'", segment.c_str());
        return false;
    }

    if (!std::getline(fenStream, segment, ' ')) return false;
    if (segment != "-") {
        for (char c : segment) {
            switch (c) {
                case 'K': model.castlingRights[0] = true; break;
                case 'Q': model.castlingRights[1] = true; break;
                case 'k': model.castlingRights[2] = true; break;
                case 'q': model.castlingRights[3] = true; break;
                default: qWarning("FEN Parsing Warning: Invalid castling character '%c'", c); break;
            }
        }
    }

    if (!std::getline(fenStream, segment, ' ')) return false;
    if (segment != "-") {
        if (segment.length() == 2 && segment[0] >= 'a' && segment[0] <= 'h' && segment[1] >= '1' && segment[1] <= '8') {
            int epCol = segment[0] - 'a';
            int epRow = segment[1] - '1';

            if (!((model.whiteToMove && epRow == 5) || (!model.whiteToMove && epRow == 2))) {
                qWarning("FEN Parsing Warning: En passant target square %s is inconsistent with side to move.", segment.c_str());
            }

            int pawnStartRow = model.whiteToMove ? epRow - 1 : epRow + 1;
            int pawnEndRow = model.whiteToMove ? epRow + 1 : epRow - 1;
            if (pawnStartRow >= 0 && pawnStartRow < 8 && pawnEndRow >=0 && pawnEndRow < 8) {
                Piece* adjacentPawn = model.getPiece(pawnStartRow, epCol);
                if (!adjacentPawn || adjacentPawn->type != 'P' || adjacentPawn->isWhite == model.whiteToMove) {
                    qWarning("FEN Parsing Warning: No opponent pawn could have created the en passant target %s", segment.c_str());
                }
                if (model.getPiece(pawnEndRow, epCol) != nullptr) {
                    qWarning("FEN Parsing Warning: Square behind en passant target %s is occupied.", segment.c_str());
                }
            }

            delete model.enPassantTarget;
            model.enPassantTarget = new Position(epRow, epCol);
        } else {
            qWarning("FEN Parsing Error: Invalid en passant target square format '%s'", segment.c_str());
        }
    }

    if (std::getline(fenStream, segment, ' ')) {
        try {
            std::stoi(segment);
        } catch (...) {
            qWarning("FEN Parsing Warning: Invalid halfmove clock value '%s'", segment.c_str());
        }
    } else {
        qWarning("FEN Parsing Warning: Missing halfmove clock.");
    }

    if (std::getline(fenStream, segment, ' ')) {
        try {
            std::stoi(segment);
        } catch (...) {
            qWarning("FEN Parsing Warning: Invalid fullmove number value '%s'", segment.c_str());
        }
    } else {
        qWarning("FEN Parsing Warning: Missing fullmove number.");
    }

    model.updateCurrentValidMoves();
    return true;
}

std::string FenUtils::generateFen(const ChessModel& model) {
    std::ostringstream fen;

    for (int row = 7; row >= 0; --row) {
        int emptyCount = 0;
        for (int col = 0; col < 8; ++col) {
            Piece* p = model.board[row][col];
            if (p == nullptr) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << (p->isWhite ? p->type : (char)std::tolower(p->type));
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (row > 0) {
            fen << '/';
        }
    }

    fen << ' ' << (model.whiteToMove ? 'w' : 'b');

    fen << ' ';
    std::string castleStr = "";
    if (model.castlingRights[0]) castleStr += 'K';
    if (model.castlingRights[1]) castleStr += 'Q';
    if (model.castlingRights[2]) castleStr += 'k';
    if (model.castlingRights[3]) castleStr += 'q';
    fen << (castleStr.empty() ? "-" : castleStr);

    fen << ' ';
    if (model.enPassantTarget != nullptr && model.enPassantTarget->isValid()) {
        int epRow = model.enPassantTarget->row;
        int epCol = model.enPassantTarget->col;
        bool validEp = false;

        if (!model.whiteToMove && epRow == 5) {
            Piece* p1 = model.getPiece(4, epCol);
            if (p1 && p1->type == 'P' && !p1->isWhite) validEp = true;
        } else if (model.whiteToMove && epRow == 2) {
            Piece* p1 = model.getPiece(3, epCol);
            if (p1 && p1->type == 'P' && p1->isWhite) validEp = true;
        }

        if (validEp) {
            fen << (char)('a' + epCol) << (char)('1' + epRow);
        } else {
            qWarning("FEN Generation Warning: En passant target square (%d,%d) is invalid for current state. Writing '-' instead.", epRow, epCol);
            fen << '-';
        }

    } else {
        fen << '-';
    }

    fen << " 0";
    fen << " 1";

    return fen.str();
}
