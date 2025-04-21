#include "model/DatabaseManager.h"
#include "model/Move.h"
#include "model/ChessModel.h"
#include "core/Utils.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

DatabaseManager::DatabaseManager(const QString& dbName, QObject *parent)
    : QObject(parent)
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_dbPath = dataPath + "/" + dbName;
    qDebug() << "Database path:" << m_dbPath;
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
        QSqlDatabase::removeDatabase("chessConnection"); // Clean up connection
    }
}

bool DatabaseManager::openDatabase()
{
    // Check if the connection already exists and is open
    if (QSqlDatabase::contains("chessConnection")) {
        m_db = QSqlDatabase::database("chessConnection");
        if (m_db.isOpen()) {
            return true;
        }
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "chessConnection"); // Use a connection name
        m_db.setDatabaseName(m_dbPath);
    }


    if (!m_db.open()) {
        qWarning() << "Error: connection with database failed -" << m_db.lastError().text();
        return false;
    }
    qDebug() << "Database opened successfully.";
    return true;
}

bool DatabaseManager::createTables()
{
    if (!m_db.isOpen()) {
        qWarning() << "Database not open for creating tables.";
        return false;
    }

    QSqlQuery query(m_db);
    bool success = true;

    success &= query.exec(R"(
        CREATE TABLE IF NOT EXISTS Games (
            game_id INTEGER PRIMARY KEY AUTOINCREMENT,
            start_datetime TEXT NOT NULL,
            end_datetime TEXT,
            result TEXT,
            last_fen TEXT,
            white_player_name TEXT DEFAULT 'White',
            black_player_name TEXT DEFAULT 'Black'
        )
    )");
    if (!success) qWarning() << "Failed to create Games table:" << query.lastError();

    success &= query.exec(R"(
        CREATE TABLE IF NOT EXISTS Moves (
            move_id INTEGER PRIMARY KEY AUTOINCREMENT,
            game_id INTEGER NOT NULL,
            move_number INTEGER NOT NULL,
            is_white_move INTEGER NOT NULL,
            from_row INTEGER NOT NULL,
            from_col INTEGER NOT NULL,
            to_row INTEGER NOT NULL,
            to_col INTEGER NOT NULL,
            san TEXT NOT NULL,
            fen_after_move TEXT NOT NULL,
            FOREIGN KEY(game_id) REFERENCES Games(game_id) ON DELETE CASCADE
        )
    )");
    if (!success) qWarning() << "Failed to create Moves table:" << query.lastError();

    success &= query.exec("CREATE INDEX IF NOT EXISTS idx_moves_game_id ON Moves (game_id)");
     if (!success) qWarning() << "Failed to create index on Moves table:" << query.lastError();

    return success;
}

bool DatabaseManager::initDatabase()
{
    if (!openDatabase()) {
        return false;
    }
    if (!createTables()) {
        m_db.close();
        QSqlDatabase::removeDatabase("chessConnection");
        return false;
    }
    return true;
}

qint64 DatabaseManager::startNewGame(const QString& whitePlayer, const QString& blackPlayer)
{
    if (!m_db.isOpen()) return -1;

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Games (start_datetime, white_player_name, black_player_name, last_fen)
        VALUES (:start, :white, :black, :fen)
    )");
    query.bindValue(":start", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":white", whitePlayer);
    query.bindValue(":black", blackPlayer);
    query.bindValue(":fen", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"); // Initial FEN

    if (!query.exec()) {
        qWarning() << "Failed to start new game:" << query.lastError();
        return -1;
    }

    qint64 gameId = query.lastInsertId().toLongLong();
    qDebug() << "Started game with ID:" << gameId;
    return gameId;
}

bool DatabaseManager::saveMove(qint64 gameId, int moveNumber, bool isWhiteMove, const Move& move, const QString& san, const QString& fenAfterMove)
{
    if (!m_db.isOpen() || gameId < 0) {
        qWarning() << "Cannot save move: DB not open or invalid game ID" << gameId;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO Moves (game_id, move_number, is_white_move, from_row, from_col, to_row, to_col, san, fen_after_move)
        VALUES (:game_id, :move_num, :is_white, :f_row, :f_col, :t_row, :t_col, :san, :fen)
    )");
    query.bindValue(":game_id", gameId);
    query.bindValue(":move_num", moveNumber);
    query.bindValue(":is_white", isWhiteMove ? 1 : 0);
    query.bindValue(":f_row", move.from.row);
    query.bindValue(":f_col", move.from.col);
    query.bindValue(":t_row", move.to.row);
    query.bindValue(":t_col", move.to.col);
    query.bindValue(":san", san);
    query.bindValue(":fen", fenAfterMove);

    if (!query.exec()) {
        qWarning() << "Failed to save move for game" << gameId << ":" << query.lastError();
        return false;
    }

    // Also update the last_fen in the Games table
    QSqlQuery updateFenQuery(m_db);
    updateFenQuery.prepare("UPDATE Games SET last_fen = :fen WHERE game_id = :game_id");
    updateFenQuery.bindValue(":fen", fenAfterMove);
    updateFenQuery.bindValue(":game_id", gameId);
    if (!updateFenQuery.exec()) {
        qWarning() << "Failed to update last FEN for game" << gameId << ":" << updateFenQuery.lastError();
        // Not returning false here, as the move itself was saved.
    }


    return true;
}

bool DatabaseManager::finishGame(qint64 gameId, const QString& result, const QString& finalFen)
{
     if (!m_db.isOpen() || gameId < 0) return false;

     QSqlQuery query(m_db);
     query.prepare(R"(
         UPDATE Games
         SET end_datetime = :end_time, result = :result, last_fen = :fen
         WHERE game_id = :game_id
     )");
     query.bindValue(":end_time", QDateTime::currentDateTime().toString(Qt::ISODate));
     query.bindValue(":result", result);
     query.bindValue(":fen", finalFen);
     query.bindValue(":game_id", gameId);

     if (!query.exec()) {
         qWarning() << "Failed to finish game:" << query.lastError();
         return false;
     }
     qDebug() << "Finished game with ID:" << gameId << "Result:" << result;
     return true;
}


QList<GameInfo> DatabaseManager::getSavedGamesList()
{
    QList<GameInfo> games;
    if (!m_db.isOpen()) return games;

    QSqlQuery query("SELECT game_id, start_datetime, end_datetime, result, white_player_name, black_player_name FROM Games ORDER BY start_datetime DESC", m_db);
    if (!query.exec()) {
        qWarning() << "Failed to get saved games list:" << query.lastError();
        return games;
    }

    while (query.next()) {
        GameInfo info;
        info.gameId = query.value(0).toLongLong();
        info.startTime = query.value(1).toString();
        info.endTime = query.value(2).toString();
        info.result = query.value(3).toString();
        info.whitePlayer = query.value(4).toString();
        info.blackPlayer = query.value(5).toString();
        games.append(info);
    }
    return games;
}

bool DatabaseManager::loadGameMoves(qint64 gameId, ChessModel* modelToLoadInto, QList<QString>& sanMovesList)
{
    if (!m_db.isOpen() || gameId < 0 || !modelToLoadInto) return false;

    sanMovesList.clear();
    QString finalFen;
    bool gameFound = false;

    QSqlQuery gameQuery(m_db);
    gameQuery.prepare("SELECT last_fen FROM Games WHERE game_id = :game_id");
    gameQuery.bindValue(":game_id", gameId);

    if (!gameQuery.exec()) {
         qWarning() << "Failed to query game" << gameId << ":" << gameQuery.lastError();
         return false;
    }

    if (gameQuery.next()) {
        finalFen = gameQuery.value(0).toString();
        gameFound = true;
        qDebug() << "Found game" << gameId << "with last FEN:" << finalFen;
    } else {
         qWarning() << "Game with ID" << gameId << "not found in Games table.";
         return false;
    }

    // If FEN is somehow empty (shouldn't happen with current save logic, but defensively check)
    if (finalFen.isEmpty()) {
         qWarning() << "Final FEN is empty for game" << gameId << ". Using starting position as fallback.";
         finalFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    }

    // Setup the model from the final FEN
    try {
         modelToLoadInto->setupFromFEN(finalFen.toStdString());
    } catch (const std::exception& e) {
        qWarning() << "Failed to setup model from FEN for game" << gameId << ":" << e.what();
        return false;
    } catch (...) {
         qWarning() << "Unknown exception setting up model from FEN for game" << gameId;
         return false;
    }

    // Get the list of SAN moves for the history display
    QSqlQuery movesQuery(m_db);
    movesQuery.prepare("SELECT san FROM Moves WHERE game_id = :game_id ORDER BY move_number ASC, is_white_move ASC");
    movesQuery.bindValue(":game_id", gameId);
    if (!movesQuery.exec()) {
        qWarning() << "Failed to load SAN moves for game" << gameId << ":" << movesQuery.lastError();
        // Continue anyway, show the board state
    } else {
        while (movesQuery.next()) {
            sanMovesList.append(movesQuery.value(0).toString());
        }
    }

    qDebug() << "Loaded game" << gameId << "with final FEN:" << finalFen << "and" << sanMovesList.count() << "moves.";
    return true;
}