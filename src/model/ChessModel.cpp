#include <cmath> 
#include <algorithm>
#include <vector>
#include <QDebug>

#include "model/ChessModel.h"
#include "core/FenUtils.h" // Include the new utility
#include "pieces/Pawn.h"
#include "pieces/Knight.h"
#include "pieces/Bishop.h"
#include "pieces/Rook.h"
#include "pieces/Queen.h"
#include "pieces/King.h"

ChessModel::ChessModel() {
    // Initialize the board with nullptr
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = nullptr;
        }
    }
    whiteToMove = true;
    isCheckmate = false;
    isStalemate = false;
    enPassantTarget = nullptr;
    for (int i = 0; i < 4; i++) {
        castlingRights[i] = true;
    }
}

ChessModel::~ChessModel() {
    clearBoard();
    clearCapturedPieces();
}

void ChessModel::clearCapturedPieces() {
    for (Piece* p : capturedByWhite) delete p;
    capturedByWhite.clear();
    for (Piece* p : capturedByBlack) delete p;
    capturedByBlack.clear();
}

void ChessModel::clearBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            delete board[i][j];
            board[i][j] = nullptr;
        }
    }
    moveHistory.clear();
    delete enPassantTarget;
    enPassantTarget = nullptr;
    isCheckmate = false;
    isStalemate = false;
    currentValidMoves.clear();
}

void ChessModel::setupStartingPosition() {
    setupFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void ChessModel::setupFromFEN(const std::string& fen) {
    if (!FenUtils::parseFen(fen, *this)) {
         qWarning("Failed to parse provided FEN string: %s", fen.c_str());
    }
}

std::string ChessModel::getCurrentFEN() const {
    return FenUtils::generateFen(*this);
}

Piece* ChessModel::getPiece(int row, int col) const {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        return board[row][col];
    }
    return nullptr;
}

bool ChessModel::isWhiteToMove() const {
    return whiteToMove;
}

void ChessModel::setWhiteToMove(bool white) {
    whiteToMove = white;
    updateCurrentValidMoves();
}

Position* ChessModel::getEnPassantTarget() const {
    return enPassantTarget;
}

bool ChessModel::getCastlingRight(int index) const {
    if (index >= 0 && index < 4) {
        return castlingRights[index];
    }
     return false;
}

const std::vector<Piece*>& ChessModel::getCapturedPieces(bool capturedByWhitePlayer) const {
    return capturedByWhitePlayer ? capturedByBlack : capturedByWhite;
}

// Get all valid moves for the piece at the given position
std::vector<Position> ChessModel::getValidMoves(Position pos) const {
    std::vector<Position> destinations;
    Piece* piece = getPiece(pos.row, pos.col);
    if (piece == nullptr || piece->isWhite != whiteToMove || !pos.isValid()) {
        return destinations;
    }
    for (const Move& move : currentValidMoves) {
        if (move.from == pos) {
            destinations.push_back(move.to);
        }
    }
    return destinations;
}

// Legal Move Generation
bool ChessModel::isMoveLegal(const Move& move) {
    Piece* movingPiece = getPiece(move.from.row, move.from.col);
    if (!movingPiece || movingPiece->isWhite != whiteToMove) return false;

    Piece* targetPiece = getPiece(move.to.row, move.to.col);
    bool pieceHadMoved = movingPiece->hasMoved;
    Piece* capturedEnPassantPawn = nullptr;
    Position capturedEnPassantPos = {-1, -1};
    Piece* castlingRook = nullptr;
    Position rookFromPos = {-1, -1}, rookToPos = {-1, -1};
    bool rookHadMoved = false;

    // Simulate EP capture
    if (movingPiece->type == 'P' && enPassantTarget && move.to == *enPassantTarget) {
        capturedEnPassantPos.row = movingPiece->isWhite ? move.to.row - 1 : move.to.row + 1;
        capturedEnPassantPos.col = move.to.col;
        if (capturedEnPassantPos.isValid()) {
             capturedEnPassantPawn = board[capturedEnPassantPos.row][capturedEnPassantPos.col];
             board[capturedEnPassantPos.row][capturedEnPassantPos.col] = nullptr;
        }
    }

    // Simulate Castling
    if (movingPiece->type == 'K' && abs(move.from.col - move.to.col) == 2) {
        if (isInCheck()) return false;

        rookFromPos.row = move.from.row;
        rookToPos.row = move.from.row;
        int step = (move.to.col > move.from.col) ? 1 : -1;
        rookFromPos.col = (step == 1) ? 7 : 0;
        rookToPos.col = move.from.col + step;

        // Check square king passes over
        if (isSquareAttacked({move.from.row, rookToPos.col}, !movingPiece->isWhite)) {
             if (capturedEnPassantPawn) board[capturedEnPassantPos.row][capturedEnPassantPos.col] = capturedEnPassantPawn;
             return false;
        }
        // Check destination square (for king)
        if (isSquareAttacked(move.to, !movingPiece->isWhite)) {
            if (capturedEnPassantPawn) board[capturedEnPassantPos.row][capturedEnPassantPos.col] = capturedEnPassantPawn;
             return false;
        }
        // Additional check for Queen-side castling (b1/b8 square)
         if (step == -1 && isSquareAttacked({move.from.row, move.from.col + 2*step}, !movingPiece->isWhite) ) {
            if (capturedEnPassantPawn) board[capturedEnPassantPos.row][capturedEnPassantPos.col] = capturedEnPassantPawn;
            return false;
         }

        // Simulate rook move for the final check
        castlingRook = board[rookFromPos.row][rookFromPos.col];
        if (castlingRook) {
            rookHadMoved = castlingRook->hasMoved;
            board[move.from.row][move.to.col - step] = castlingRook;
            board[rookFromPos.row][rookFromPos.col] = nullptr;
        } else { 
             if (capturedEnPassantPawn) board[capturedEnPassantPos.row][capturedEnPassantPos.col] = capturedEnPassantPawn;
            return false;
        }
    }

    // Simulate main piece move
    board[move.to.row][move.to.col] = movingPiece;
    board[move.from.row][move.from.col] = nullptr;

    // Check if king is attacked AFTER the move
    Position kingPos = findKing(movingPiece->isWhite);
    bool leavesKingInCheck = false;
    if (!kingPos.isValid()) { leavesKingInCheck = true; } // Should not happen
    else { leavesKingInCheck = isSquareAttacked(kingPos, !movingPiece->isWhite); }

    // Undo simulation
    board[move.from.row][move.from.col] = movingPiece;
    board[move.to.row][move.to.col] = targetPiece;
    if (movingPiece) movingPiece->hasMoved = pieceHadMoved;
    if (capturedEnPassantPawn) board[capturedEnPassantPos.row][capturedEnPassantPos.col] = capturedEnPassantPawn;
    if (castlingRook) {
         board[rookFromPos.row][rookFromPos.col] = castlingRook;
         board[move.from.row][move.to.col - ((move.to.col > move.from.col) ? 1 : -1)] = nullptr;
         if (castlingRook) castlingRook->hasMoved = rookHadMoved;
    }

    return !leavesKingInCheck;
}


void ChessModel::updateCurrentValidMoves() {
    currentValidMoves.clear();
    isCheckmate = false;
    isStalemate = false;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Piece* p = board[r][c];
            if (p && p->isWhite == whiteToMove) {
                Position fromPos(r, c);
                std::vector<Position> pseudoMoves = p->getPossibleMoves(fromPos, this);
                for (const Position& toPos : pseudoMoves) {
                    Move potentialMove(fromPos, toPos);
                    if (isMoveLegal(potentialMove)) {
                        currentValidMoves.push_back(potentialMove);
                    }
                }
            }
        }
    }
}

void ChessModel::updateGameStatus() {
    bool opponentInCheck = isInCheck();
    bool opponentHasMoves = !currentValidMoves.empty();
    isCheckmate = opponentInCheck && !opponentHasMoves;
    isStalemate = !opponentInCheck && !opponentHasMoves;
}


void ChessModel::updateCastlingRights(const Move& move, Piece* movedPiece, Piece* capturedPiece) {
     if (!movedPiece) return;
    if (movedPiece->type == 'K') {
        if (movedPiece->isWhite) { castlingRights[0] = false; castlingRights[1] = false; }
        else { castlingRights[2] = false; castlingRights[3] = false; }
    } else if (movedPiece->type == 'R') {
         if (movedPiece->isWhite) {
            if (move.from.row == 0 && move.from.col == 0) castlingRights[1] = false;
            if (move.from.row == 0 && move.from.col == 7) castlingRights[0] = false;
        } else {
            if (move.from.row == 7 && move.from.col == 0) castlingRights[3] = false;
            if (move.from.row == 7 && move.from.col == 7) castlingRights[2] = false;
        }
    }
     if (capturedPiece && capturedPiece->type == 'R') {
         const Position& to = move.to; 
         if (to.row == 0 && to.col == 0) castlingRights[1] = false;
         if (to.row == 0 && to.col == 7) castlingRights[0] = false;
         if (to.row == 7 && to.col == 0) castlingRights[3] = false;
         if (to.row == 7 && to.col == 7) castlingRights[2] = false;
     }
}

void ChessModel::updateEnPassantTarget(const Move& move, Piece* movedPiece) {
    delete enPassantTarget;
    enPassantTarget = nullptr;
    if (movedPiece && movedPiece->type == 'P' && abs(move.from.row - move.to.row) == 2) {
        int epRow = movedPiece->isWhite ? move.from.row + 1 : move.from.row - 1;
        enPassantTarget = new Position(epRow, move.from.col);
    }
}

bool ChessModel::makeMove(const Move& move) {
    if (isGameOver()) {
        qDebug() << "Game is over.";
        return false;
    }

    bool moveFound = false;
    for(const Move& validMove : currentValidMoves) {
        if (validMove.from == move.from && validMove.to == move.to) {
            moveFound = true;
            break;
        }
    }
    if (!moveFound) {
        qDebug() << "Attempted move" << move.from.row << "," << move.from.col << "->"
                 << move.to.row << "," << move.to.col << "is not in the list of valid moves.";
        return false;
    }

    Piece* piece = board[move.from.row][move.from.col];
    Piece* capturedPiece = board[move.to.row][move.to.col];
    Piece* actualCaptured = capturedPiece;

    // Handle En Passant
    if (piece->type == 'P' && enPassantTarget && move.to == *enPassantTarget) {
        int capturedRow = piece->isWhite ? move.to.row - 1 : move.to.row + 1;
        actualCaptured = board[capturedRow][move.to.col];
        board[capturedRow][move.to.col] = nullptr;
        qDebug() << "En passant capture performed.";
    }

    // Handle Castling
    if (piece->type == 'K' && abs(move.from.col - move.to.col) == 2) {
        int rookFromCol = (move.to.col > move.from.col) ? 7 : 0;
        int rookToCol   = (move.to.col > move.from.col) ? 5 : 3;
        Piece* rook = board[move.from.row][rookFromCol];
        if (rook) {
            board[move.from.row][rookToCol] = rook;
            board[move.from.row][rookFromCol] = nullptr;
            rook->hasMoved = true;
            qDebug() << "Castling rook moved.";
        } else {
            qWarning() << "CRITICAL ERROR: Castling move valid but rook missing.";
            return false;
        }
    }

    updateCastlingRights(move, piece, actualCaptured);

    // Move the piece
    board[move.to.row][move.to.col] = piece;
    board[move.from.row][move.from.col] = nullptr;

    // Handle Pawn Promotion
    bool isPromotion = false;
    if (piece && piece->type == 'P' && (move.to.row == 0 || move.to.row == 7)) {
        Piece* pawnToPromote = piece;
        bool promoteToWhite = pawnToPromote->isWhite;
        // Create promotion piece directly here (Queen by default)
        // TODO: Allow choosing promotion piece (e.g., via UI signal/slot)
        Piece* promotedPiece = new Queen(promoteToWhite);
        if (promotedPiece) {
            board[move.to.row][move.to.col] = promotedPiece;
            promotedPiece->hasMoved = true;
            piece = promotedPiece;
            qDebug() << "Pawn promoted to Queen!";
        } else {
            qWarning() << "Failed to create promotion piece!";
            board[move.from.row][move.from.col] = pawnToPromote;
            board[move.to.row][move.to.col] = capturedPiece;
            return false;
        }
        delete pawnToPromote;
        isPromotion = true;
    }

    // Update hasMoved flag
    if (piece && !isPromotion) {
        piece->hasMoved = true;
    }

    updateEnPassantTarget(move, piece);

    // Track captured pieces
    if (actualCaptured != nullptr) {
        if (actualCaptured->isWhite) capturedByBlack.push_back(actualCaptured);
        else capturedByWhite.push_back(actualCaptured);
        qDebug() << "Piece captured:" << actualCaptured->type << "at" << move.to.row << "," << move.to.col;
    }

    whiteToMove = !whiteToMove;
    moveHistory.push_back(move);
    updateCurrentValidMoves();
    updateGameStatus();

    return true;
}

Position ChessModel::findKing(bool white) const {
    // Find the king
    Position kingPos(-1, -1);
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = getPiece(row, col);
            if (piece != nullptr && piece->type == 'K' && piece->isWhite == white) {
                return Position(row, col);
                break;
            }
        }
    }
    return {-1, -1};
}

bool ChessModel::isSquareAttacked(Position pos, bool byWhite) const {
    if (!pos.isValid()) return false;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            Piece* piece = getPiece(row, col);
            if (piece && piece->isWhite == byWhite) {
                if (piece->type == 'P') {
                    int dir = piece->isWhite ? 1 : -1;
                    if (pos.row == row + dir && (pos.col == col + 1 || pos.col == col - 1)) {
                         return true;
                    }
                } else if (piece->type == 'K') {
                     int dr = abs(row - pos.row); int dc = abs(col - pos.col);
                     if (dr <= 1 && dc <= 1) {
                         return true;
                     }
                 } else {
                    std::vector<Position> moves = piece->getPossibleMoves(Position(row, col), this);
                    for (Position& move : moves) {
                        if (move.row == pos.row && move.col == pos.col) {
                            return true;
                        }
                    }
                 }
            }
        }
    }
    return false;
}

bool ChessModel::isInCheck() const {
     Position kingPos = findKing(whiteToMove);
     if (!kingPos.isValid()) return false;
     return isSquareAttacked(kingPos, !whiteToMove);
}
