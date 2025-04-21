#include "gui/ChessBoardWidget.h"
#include "model/pieces/Piece.h"
#include "model/Move.h"
#include "core/Utils.h"
#include "model/ChessModel.h"
#include "gui/Constants.h"
#include "gui/DrawingUtils.h"
#include "gui/BoardInteractionHandler.h" 

#include <QPainter>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDataStream>
#include <QIODevice>
#include <QDebug>
#include <QPen>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <QFontMetrics>
#include <vector>
#include <algorithm>

ChessBoardWidget::ChessBoardWidget(ChessModel* model, QWidget *parent)
    : QWidget(parent), chessModel(model)
{
    setAcceptDrops(true);
    interactionHandler = new BoardInteractionHandler(this, chessModel, this); // Create handler
}

ChessBoardWidget::~ChessBoardWidget() {
    delete interactionHandler; 
}

QSize ChessBoardWidget::minimumSizeHint() const { return QSize(320, 320); }
QSize ChessBoardWidget::sizeHint() const { return QSize(480, 480); }

int ChessBoardWidget::squareSize() const {
    int w = width();
    int h = height();
    return qMin(w, h) / 8;
}

QRect ChessBoardWidget::squareRect(const Position& pos) const {
    int sSize = squareSize();
    if (sSize == 0 || !pos.isValid()) return QRect();
    return QRect(pos.col * sSize, (7 - pos.row) * sSize, sSize, sSize);
}

Position ChessBoardWidget::positionFromPoint(const QPoint& point) const {
    int sSize = squareSize();
    if (sSize == 0 || !rect().contains(point)) return {-1, -1};

    int col = point.x() / sSize;
    int row = 7 - (point.y() / sSize);
    if (col < 0 || col > 7 || row < 0 || row > 7) return {-1, -1};

    return {row, col};
}

void ChessBoardWidget::resetInteractionState(bool doUpdate) {
    interactionHandler->resetState();
    if (doUpdate) update();
}

Position ChessBoardWidget::getSelectedSquare() const {
    return interactionHandler->getSelectedSquare();
}

Position ChessBoardWidget::getDragIndicatorSource() const {
    return interactionHandler->getDragIndicatorSource();
}

void ChessBoardWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int sSize = squareSize();
    if (sSize == 0 || !chessModel) return;

    // Draw chessboard squares
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            painter.setBrush((row + col) % 2 ? ChessConstants::DARK_SQUARE_COLOR : ChessConstants::LIGHT_SQUARE_COLOR);
            painter.drawRect(squareRect({row, col}));
        }
    }

    // Highlight selected square
    if (getSelectedSquare().isValid()) {
        painter.setBrush(ChessConstants::SELECTION_HIGHLIGHT_COLOR);
        painter.drawRect(squareRect(getSelectedSquare()));
    }

    // Draw pieces
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Position currentPos{row, col};

            if (getDragIndicatorSource().isValid() && currentPos == getDragIndicatorSource())
                continue;

            if (Piece* piece = chessModel->getPiece(row, col)) {
                ChessDrawingUtils::drawPiece(painter, piece, squareRect(currentPos), sSize);
            }
        }
    }

    // Show valid move indicators
    Position indicatorPos = getDragIndicatorSource().isValid() ? getDragIndicatorSource() : getSelectedSquare();
    if (indicatorPos.isValid()) {
        std::vector<Position> validMoves = chessModel->getValidMoves(indicatorPos);
        Piece* sourcePiece = chessModel->getPiece(indicatorPos.row, indicatorPos.col);
        if (sourcePiece && sourcePiece->isWhite == chessModel->isWhiteToMove()) { 
            painter.setPen(Qt::NoPen);
            painter.setBrush(ChessConstants::MOVE_INDICATOR_COLOR);
            int radius = sSize / 7;
            for (const auto& move : chessModel->getValidMoves(indicatorPos)) {
                painter.drawEllipse(squareRect(move).center(), radius, radius);
            }
        }
    }

    // Draw coordinates
    painter.setFont(QFont("Arial", qMax(8, sSize/7), QFont::Bold));
    QFontMetrics fm(painter.font());
    int padding = sSize / 15;

    // File letters (a-h)
    for (int col = 0; col < 8; ++col) {
        bool dark = (0 + col) % 2;
        painter.setPen(dark ? ChessConstants::LIGHT_SQUARE_COLOR : ChessConstants::DARK_SQUARE_COLOR);
        QString file = QString(QChar::fromLatin1('a' + col));
        QRect square = squareRect({0, col});
        painter.drawText(square.right() - fm.horizontalAdvance(file) - padding, 
                        square.bottom() - fm.descent() - padding, file);
    }

    // Rank numbers (1-8)
    for (int row = 0; row < 8; ++row) {
        bool dark = (row + 0) % 2;
        painter.setPen(dark ? ChessConstants::LIGHT_SQUARE_COLOR : ChessConstants::DARK_SQUARE_COLOR);
        QString rank = QString::number(row + 1);
        QRect square = squareRect({row, 0});
        painter.drawText(square.left() + padding, square.top() + fm.ascent() + padding, rank);
    }
}

void ChessBoardWidget::mousePressEvent(QMouseEvent* event) {
    interactionHandler->handleMousePress(event);
}

void ChessBoardWidget::mouseMoveEvent(QMouseEvent *event) {
    interactionHandler->handleMouseMove(event);
}

void ChessBoardWidget::dragEnterEvent(QDragEnterEvent* event) {
    interactionHandler->handleDragEnter(event);
}

void ChessBoardWidget::dragMoveEvent(QDragMoveEvent *event) {
    interactionHandler->handleDragMove(event);
}

void ChessBoardWidget::dropEvent(QDropEvent *event) {
    interactionHandler->handleDrop(event);
}
