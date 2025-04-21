#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QPair>

class Move;
class ChessModel;

// Struct to hold basic game info for listing
struct GameInfo {
    qint64 gameId;
    QString startTime;
    QString endTime;
    QString result;
    QString whitePlayer;
    QString blackPlayer;
};

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(const QString& dbPath = "chess_games.db", QObject *parent = nullptr);
    ~DatabaseManager();

    bool initDatabase(); 

    qint64 startNewGame(const QString& whitePlayer = "White", const QString& blackPlayer = "Black");
    bool saveMove(qint64 gameId, int moveNumber, bool isWhiteMove, const Move& move, const QString& san, const QString& fenAfterMove);
    bool finishGame(qint64 gameId, const QString& result, const QString& finalFen);

    QList<GameInfo> getSavedGamesList();
    bool loadGameMoves(qint64 gameId, ChessModel* modelToLoadInto, QList<QString>& sanMovesList);

private:
    QSqlDatabase m_db;
    QString m_dbPath;

    bool openDatabase();
    bool createTables();
};

#endif 