#ifndef CHESS_MODEL_H
#define CHESS_MODEL_H

#include <string>
#include <vector>
#include <optional>
#include "model/pieces/Piece.h"
#include "Position.h"
#include "Move.h"
#include "core/FenUtils.h"

class Piece;

class ChessModel {
friend class FenUtils; 
private:
    Piece* board[8][8];
    bool whiteToMove;
    Position* enPassantTarget;
    bool castlingRights[4]; // 0: white kingside, 1: white queenside, 2: black kingside, 3: black queenside

    // Game State
    bool isCheckmate = false;
    bool isStalemate = false;
    std::vector<Move> currentValidMoves;
    std::vector<Piece*> capturedByWhite;
    std::vector<Piece*> capturedByBlack;
    std::vector<Move> moveHistory;      

    // Private Helper Methods
    bool isMoveLegal(const Move& move);
    void updateCurrentValidMoves();
    void updateGameStatus();
    void updateCastlingRights(const Move& move, Piece* movedPiece, Piece* capturedPiece);    
    void updateEnPassantTarget(const Move& move, Piece* movedPiece);
    Position findKing(bool white) const;
    bool isSquareAttacked(Position square, bool byWhite) const;
    
    void clearBoard();
    void clearCapturedPieces();


public:
    ChessModel();
    ~ChessModel();

    void setupStartingPosition();
    void setupFromFEN(const std::string& fen);
    std::string getCurrentFEN() const;
    Piece* getPiece(int row, int col) const;
    bool isWhiteToMove() const;
    void setWhiteToMove(bool white);
    Position* getEnPassantTarget() const;
    bool getCastlingRight(int index) const;    
    const std::vector<Piece*>& getCapturedPieces(bool capturedByWhitePlayer) const; 
    std::vector<Position> getValidMoves(Position pos) const;    
    bool makeMove(const Move& move);
    bool isInCheck() const;
    bool getIsCheckmate() const { return isCheckmate; }
    bool getIsStalemate() const { return isStalemate; }
    bool isGameOver() const { return isCheckmate || isStalemate; }
    const std::vector<Move>& getMoveHistory() const { return moveHistory; } 
};

#endif // CHESS_MODEL_H
