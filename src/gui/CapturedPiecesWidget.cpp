#include "CapturedPiecesWidget.h"
#include "DrawingUtils.h"
#include <QPainter>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>
#include <vector>

CapturedPiecesWidget::CapturedPiecesWidget(const ChessModel* model, bool showWhiteCaptures, QWidget *parent) 
    : QWidget(parent), chessModel(model), showWhiteCaptures(showWhiteCaptures)
{}

CapturedPiecesWidget::~CapturedPiecesWidget() {}

QSize CapturedPiecesWidget::minimumSizeHint() const {
    const int pieceSize = 20;
    const int rows = 3;
    const int cols = 5;
    const int spacing = 4;
    return QSize(cols * pieceSize + (cols - 1) * spacing, rows * pieceSize + (rows - 1) * spacing);
}

QSize CapturedPiecesWidget::sizeHint() const {
    const int pieceSize = 25;
    const int rows = 3;
    const int cols = 5;
    const int spacing = 5;
    return QSize(cols * pieceSize + (cols - 1) * spacing, rows * pieceSize + (rows - 1) * spacing);
}

void CapturedPiecesWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (!chessModel) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const std::vector<Piece*>& capturedPieces = chessModel->getCapturedPieces(showWhiteCaptures);

    const int maxCols = 5;
    const int spacing = 5;
    const int pieceSize = qMin(35, qMax(20, height() / 3 - spacing));
    const int maxRows = 3;

    int startX = (width() - (maxCols * pieceSize + (maxCols - 1) * spacing)) / 2;
    int startY = (height() - (maxRows * pieceSize + (maxRows - 1) * spacing)) / 2;

    for (size_t i = 0; i < capturedPieces.size() && i < maxCols * maxRows; ++i) {
        int row = i / maxCols;
        int col = i % maxCols;

        int x = startX + col * (pieceSize + spacing);
        int y = startY + row * (pieceSize + spacing);

        QRect pieceRect(x, y, pieceSize, pieceSize);
        ChessDrawingUtils::drawPiece(painter, capturedPieces[i], pieceRect, pieceSize);
    }
}

void CapturedPiecesWidget::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    update(); // Redraw on resize
}
