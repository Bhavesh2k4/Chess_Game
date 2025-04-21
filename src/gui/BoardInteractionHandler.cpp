#include "gui/BoardInteractionHandler.h"
#include "gui/ChessBoardWidget.h"
#include "model/ChessModel.h"
#include "model/pieces/Piece.h"
#include "model/Move.h"
#include "core/Utils.h"
#include "gui/DrawingUtils.h"

#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDataStream>
#include <QIODevice>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include <vector>
#include <algorithm>

BoardInteractionHandler::BoardInteractionHandler(ChessBoardWidget* boardWidget, ChessModel* model, QObject *parent)
    : QObject(parent),
      boardWidget(boardWidget),
      chessModel(model),
      selectedSquare{-1, -1},
      dragIndicatorSource{-1, -1},
      draggedPiece(nullptr)
{
    if (!boardWidget) qWarning("BoardInteractionHandler created with null boardWidget!");
    if (!chessModel) qWarning("BoardInteractionHandler created with null chessModel!");
}

Position BoardInteractionHandler::getSelectedSquare() const {
    return selectedSquare;
}

Position BoardInteractionHandler::getDragIndicatorSource() const {
    return dragIndicatorSource;
}

void BoardInteractionHandler::resetState() {
    selectedSquare = {-1, -1};
    dragIndicatorSource = {-1, -1};
    draggedPiece = nullptr;
}

void BoardInteractionHandler::handleMousePress(QMouseEvent* event) {
    if (!boardWidget || !chessModel) return;

    if (event->button() != Qt::LeftButton || chessModel->isGameOver()) {
        if (selectedSquare.isValid() || dragIndicatorSource.isValid()) {
            resetState();
            boardWidget->update();
        }
        return;
    }

    const Position clickedPos = boardWidget->positionFromPoint(event->pos());
    if (!clickedPos.isValid()) {
        if (selectedSquare.isValid() || dragIndicatorSource.isValid()) {
            resetState();
            boardWidget->update();
        }
        return;
    }

    Piece* clickedPiece = chessModel->getPiece(clickedPos.row, clickedPos.col);
    draggedPiece = nullptr;

    if (selectedSquare.isValid()) {
        Position startPos = selectedSquare;
        Piece* selectedPiece = chessModel->getPiece(startPos.row, startPos.col);

        if (!selectedPiece) {
            qWarning() << "Selected square valid but no piece found at" << startPos.row << "," << startPos.col;
            resetState();
            boardWidget->update();
            return;
        }

        if (clickedPos == startPos) {
            resetState();
            boardWidget->update();
            return;
        }

        auto validMoves = chessModel->getValidMoves(startPos);
        bool isValidMove = std::find(validMoves.begin(), validMoves.end(), clickedPos) != validMoves.end();

        if (isValidMove) {
            Move move(startPos, clickedPos);
            qDebug() << "Click Move Attempted:" << QString::fromStdString(Utils::moveToSAN(move, *chessModel));
            emit boardWidget->moveAttempted(move);
        } else if (clickedPiece && clickedPiece->isWhite == selectedPiece->isWhite) {
            selectedSquare = clickedPos;
            dragIndicatorSource = {-1, -1};
            dragStartPosition = event->pos();
            draggedPiece = clickedPiece;
            qDebug() << "Selection switched to:" << QString::fromStdString(Utils::positionToString(clickedPos));
            boardWidget->update();
        } else {
            qDebug() << "Selection cancelled (invalid second click).";
            resetState();
            boardWidget->update();
        }
        return;
    }

    if (clickedPiece && clickedPiece->isWhite == chessModel->isWhiteToMove()) {
        selectedSquare = clickedPos;
        dragIndicatorSource = {-1, -1};
        dragStartPosition = event->pos();
        draggedPiece = clickedPiece;
        qDebug() << "Piece Selected (Click):" << QString::fromStdString(Utils::positionToString(clickedPos));
        boardWidget->update();
    } else if (selectedSquare.isValid() || dragIndicatorSource.isValid()) {
        resetState();
        boardWidget->update();
    }
}

void BoardInteractionHandler::handleMouseMove(QMouseEvent *event) {
    if (!boardWidget || !chessModel) return;

    if (!(event->buttons() & Qt::LeftButton) || !draggedPiece || chessModel->isGameOver())
        return;

    if ((event->pos() - dragStartPosition).manhattanLength() >= QApplication::startDragDistance()) {
        startDrag();
    }
}

void BoardInteractionHandler::startDrag() {
    if (!boardWidget || !chessModel || !draggedPiece) {
        qWarning("startDrag called with invalid state");
        resetState();
        boardWidget->update();
        return;
    }

    Position startPos = boardWidget->positionFromPoint(dragStartPosition);
    if (!startPos.isValid()) {
        qWarning() << "Invalid drag start position.";
        resetState();
        boardWidget->update();
        return;
    }

    Piece* pieceAtStart = chessModel->getPiece(startPos.row, startPos.col);
    if (pieceAtStart != draggedPiece) {
        qWarning() << "Mismatch between draggedPiece and piece at start.";
        if (pieceAtStart && pieceAtStart->isWhite == chessModel->isWhiteToMove()) {
            draggedPiece = pieceAtStart;
        } else {
            resetState();
            boardWidget->update();
            return;
        }
    }

    if (draggedPiece->isWhite != chessModel->isWhiteToMove()) {
        qWarning() << "Tried to drag opponent's piece.";
        resetState();
        boardWidget->update();
        return;
    }

    selectedSquare = {-1, -1};
    dragIndicatorSource = startPos;
    Piece* pieceToDrag = draggedPiece;
    draggedPiece = nullptr;

    qDebug() << "Drag started from" << QString::fromStdString(Utils::positionToString(startPos));

    QMimeData *mimeData = new QMimeData;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << startPos.row << startPos.col;
    mimeData->setData("application/x-chess-move-start", data);

    QDrag *drag = new QDrag(boardWidget);
    drag->setMimeData(mimeData);

    int sSize = boardWidget->squareSize();
    if (sSize <= 0) {
        qWarning() << "Invalid square size.";
        dragIndicatorSource = {-1,-1};
        boardWidget->update();
        return;
    }

    QPixmap pixmap(sSize, sSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    ChessDrawingUtils::drawPiece(painter, pieceToDrag, pixmap.rect(), sSize);

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(sSize / 2, sSize / 2));

    boardWidget->update();
    drag->exec(Qt::MoveAction);

    qDebug() << "Drag finished.";
    if (dragIndicatorSource == startPos)
        dragIndicatorSource = {-1, -1};

    selectedSquare = {-1, -1};
    boardWidget->update();
}

void BoardInteractionHandler::handleDragEnter(QDragEnterEvent* event) {
    if (!boardWidget || !chessModel) { event->ignore(); return; }

    if (!chessModel->isGameOver() && event->mimeData()->hasFormat("application/x-chess-move-start"))
        event->acceptProposedAction();
    else
        event->ignore();
}

void BoardInteractionHandler::handleDragMove(QDragMoveEvent *event) {
    if (!boardWidget || !chessModel) { event->ignore(); return; }

    if (!chessModel->isGameOver() && event->mimeData()->hasFormat("application/x-chess-move-start"))
        event->acceptProposedAction();
    else
        event->ignore();
}

void BoardInteractionHandler::handleDrop(QDropEvent *event) {
    if (!boardWidget || !chessModel) { event->ignore(); return; }

    if (chessModel->isGameOver()) {
        event->ignore();
        resetState();
        boardWidget->update();
        return;
    }

    Position startPos = {-1, -1};
    if (event->mimeData()->hasFormat("application/x-chess-move-start")) {
        QByteArray data = event->mimeData()->data("application/x-chess-move-start");
        QDataStream stream(&data, QIODevice::ReadOnly);
        int row, col;
        if ((stream >> row >> col).status() == QDataStream::Ok) {
            startPos = {row, col};
        }
    }

    if (!startPos.isValid()) {
        qWarning() << "Invalid or missing drag start position.";
        event->ignore();
        resetState();
        boardWidget->update();
        return;
    }

    Position endPos = boardWidget->positionFromPoint(event->position().toPoint());
    if (!endPos.isValid()) {
        qWarning() << "Invalid drop position.";
        event->ignore();
        resetState();
        boardWidget->update();
        return;
    }

    Piece* piece = chessModel->getPiece(startPos.row, startPos.col);
    bool isValidDrop = false;
    if (piece && piece->isWhite == chessModel->isWhiteToMove()) {
        auto validMoves = chessModel->getValidMoves(startPos);
        isValidDrop = std::any_of(validMoves.begin(), validMoves.end(),
                                  [&](const Position& p) { return p == endPos; });
    }

    if (isValidDrop) {
        Move move(startPos, endPos);
        qDebug() << "Drag Move Attempted:" << QString::fromStdString(Utils::moveToSAN(move, *chessModel));
        emit boardWidget->moveAttempted(move);
        event->acceptProposedAction();
    } else {
        qWarning() << "Invalid drop move from" << QString::fromStdString(Utils::positionToString(startPos))
                   << "to" << QString::fromStdString(Utils::positionToString(endPos));
        event->ignore();
    }

    resetState();
    boardWidget->update();
}
