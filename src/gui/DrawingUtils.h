#ifndef CHESS_DRAWING_UTILS_H
#define CHESS_DRAWING_UTILS_H

#include <QPainter>
#include <QRect>

class Piece;

namespace ChessDrawingUtils {

void drawPiece(QPainter& painter, const Piece* piece, const QRect& targetRect, int referenceSize);

} 

#endif