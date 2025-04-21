#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include "model/Position.h"

class ChessModel;
class QPaintEvent;
class QMouseEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class BoardInteractionHandler;

class ChessBoardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChessBoardWidget(ChessModel* model, QWidget *parent = nullptr);
    ~ChessBoardWidget() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void resetInteractionState(bool doUpdate = true);

    int squareSize() const;
    QRect squareRect(const Position& pos) const;
    Position positionFromPoint(const QPoint& point) const;

    Position getSelectedSquare() const;
    Position getDragIndicatorSource() const;

signals:
    void moveAttempted(const class Move& move);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    ChessModel* chessModel;

    BoardInteractionHandler* interactionHandler; 

   
};

#endif 
