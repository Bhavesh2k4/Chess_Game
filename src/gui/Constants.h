#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QColor>
#include <QChar>
#include <QMap>

namespace ChessConstants {
    inline constexpr QColor WHITE_PIECE_COLOR = QColor(248, 248, 248);
    inline constexpr QColor BLACK_PIECE_COLOR = QColor(86, 83, 82);
    inline constexpr QColor PIECE_BORDER_COLOR = QColor(70, 70, 70);
    inline constexpr qreal  PIECE_BORDER_THICKNESS = 2.5;
    inline constexpr qreal  CAPTURED_PIECE_SIZE_FACTOR = 0.6;

    inline const QColor LIGHT_SQUARE_COLOR = QColor(238, 238, 210);
    inline const QColor DARK_SQUARE_COLOR = QColor(118, 150, 86);
    inline const QColor SELECTION_HIGHLIGHT_COLOR = QColor(255, 255, 0, 100);
    inline const QColor MOVE_INDICATOR_COLOR = QColor(0, 0, 0, 70);

    inline const QMap<char, QChar> PIECE_UNICODE_MAP = {
        { 'K', QChar(0x265A) }, { 'Q', QChar(0x265B) }, { 'R', QChar(0x265C) },
        { 'B', QChar(0x265D) }, { 'N', QChar(0x265E) }, { 'P', QChar(0x265F) }
    };
}

#endif