#ifndef BOARDINTERACTIONHANDLER_H
#define BOARDINTERACTIONHANDLER_H

#include "model/Position.h"
#include <QObject>
#include <QPoint>

class ChessBoardWidget;
class ChessModel;
class Piece;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

class BoardInteractionHandler : public QObject {
    Q_OBJECT

public:
    explicit BoardInteractionHandler(ChessBoardWidget* boardWidget, ChessModel* model, QObject *parent = nullptr);

    void handleMousePress(QMouseEvent* event);
    void handleMouseMove(QMouseEvent* event);
    void handleDragEnter(QDragEnterEvent* event);
    void handleDragMove(QDragMoveEvent* event);
    void handleDrop(QDropEvent* event);

    void resetState();

    Position getSelectedSquare() const;
    Position getDragIndicatorSource() const;

private:
    void startDrag();

    ChessBoardWidget* boardWidget;
    ChessModel* chessModel;

    Position selectedSquare;
    Position dragIndicatorSource;
    QPoint dragStartPosition;
    Piece* draggedPiece;
};

#endif
