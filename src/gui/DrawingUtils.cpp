#include "DrawingUtils.h"
#include "model/pieces/Piece.h"        
#include "Constants.h" 

#include <QFont>
#include <QPainterPath>
#include <QPainterPathStroker>

namespace ChessDrawingUtils {

    void drawPiece(QPainter& painter, const Piece* piece, const QRect& targetRect, int referenceSize) {
        if (!piece || targetRect.isNull() || referenceSize <= 0 ) return;

        QChar pieceChar = ChessConstants::PIECE_UNICODE_MAP.value(piece->type, '?');
        QFont pieceFont("Arial Unicode MS", referenceSize * 0.6);
        painter.setFont(pieceFont);

        // Calculate text positioning
        QPainterPath textPath;
        textPath.addText(0, 0, painter.font(), pieceChar);
        QRectF textBounds = textPath.boundingRect();
        qreal dx = targetRect.left() + (targetRect.width() - textBounds.width())/2 - textBounds.left();
        qreal dy = targetRect.top() + (targetRect.height() - textBounds.height())/2 - textBounds.top();
        textPath.translate(dx, dy);

        // Create border effect
        QPainterPathStroker stroker;
        stroker.setWidth(ChessConstants::PIECE_BORDER_THICKNESS);
        stroker.setCapStyle(Qt::RoundCap);
        stroker.setJoinStyle(Qt::RoundJoin);
        QPainterPath borderPath = stroker.createStroke(textPath);

        painter.save();

        painter.setPen(Qt::NoPen);
        painter.setBrush(ChessConstants::PIECE_BORDER_COLOR);
        painter.drawPath(borderPath);
        painter.setBrush(piece->isWhite ? ChessConstants::WHITE_PIECE_COLOR : ChessConstants::BLACK_PIECE_COLOR);
        painter.drawPath(textPath);

        painter.restore();
    }

}