#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "model/DatabaseManager.h" 

class ChessBoardWidget;
class ChessModel;
class QLabel;
class Move;
class QListWidget;
class CapturedPiecesWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void startNewGame();
    bool loadGame(); 

private slots:
    void handleMoveAttempt(const Move& move);

private:
    ChessBoardWidget *boardWidget = nullptr;
    ChessModel *chessModel = nullptr;
    QLabel *statusLabel = nullptr;
    QListWidget *moveHistoryWidget = nullptr;
    CapturedPiecesWidget *whiteCapturedWidget = nullptr;
    CapturedPiecesWidget *blackCapturedWidget = nullptr;

    int fullMoveNumber = 1;
    int halfMoveClock = 0;
    DatabaseManager *dbManager = nullptr;
    qint64 currentGameId = -1; 

    void setupUi();
    void setupConnections();
    void updateStatus();
    void showGameOverMessage(const QString& message);
    void updateMoveHistory(const QString& sanMove);
    void populateMoveHistory(const QList<QString>& sanMoves);
};

#endif
