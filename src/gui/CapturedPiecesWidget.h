#ifndef CAPTUREDPIECESWIDGET_H
#define CAPTUREDPIECESWIDGET_H

#include <QWidget>
#include <vector>
#include "model/ChessModel.h"

class Piece;
class QPaintEvent;
class QPainter;

class CapturedPiecesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CapturedPiecesWidget(const ChessModel* model, bool showWhiteCaptures, QWidget *parent = nullptr);
    ~CapturedPiecesWidget();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const ChessModel* chessModel;
    bool showWhiteCaptures;
    void resizeEvent(QResizeEvent *event);

};

#endif
