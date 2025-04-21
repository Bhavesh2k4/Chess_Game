#include "gui/MainWindow.h"
#include "gui/WelcomeDialog.h"
#include "gui/ChessBoardWidget.h"
#include "gui/CapturedPiecesWidget.h"
#include "gui/GameLoadDialog.h"
#include "model/ChessModel.h"
#include "core/Utils.h"
#include "model/DatabaseManager.h"

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QMessageBox>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QDebug>
#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), fullMoveNumber(1), halfMoveClock(0), currentGameId(-1)
{
    dbManager = new DatabaseManager("chess_games.db", this);
    if (!dbManager->initDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to initialize the game database. Saved games will not be available.");
    } else {
         qDebug() << "Database initialized successfully.";
    }

    setupUi(); 
    setupConnections();
    updateStatus();
}

MainWindow::~MainWindow()
{
    delete chessModel;
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("Chess"));
    resize(890, 650);

    // Menu Bar
    QMenuBar *menuBar = this->menuBar();
    QMenu *gameMenu = menuBar->addMenu(tr("&Game"));
    QAction *newGameAction = gameMenu->addAction(tr("&New Game"));
    QAction *loadGameAction = gameMenu->addAction(tr("&Load Game"));
    gameMenu->addSeparator();
    QAction *quitAction = gameMenu->addAction(tr("&Quit"));

    // Central Widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main Horizontal Layout (Board | Side Panel)
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Model
    chessModel = new ChessModel();

    // Board Widget
    boardWidget = new ChessBoardWidget(chessModel, centralWidget);

    // Side Panel (Vertical Layout: Captured White, History, Captured Black, Status)
    QWidget *sidePanelWidget = new QWidget(centralWidget);
    QVBoxLayout *sidePanelLayout = new QVBoxLayout(sidePanelWidget);
    sidePanelLayout->setContentsMargins(5, 0, 10, 0); 

    blackCapturedWidget = new CapturedPiecesWidget(chessModel, false, sidePanelWidget);
    blackCapturedWidget->setToolTip("Pieces captured by White"); 
    blackCapturedWidget->setMinimumHeight(100);

    moveHistoryWidget = new QListWidget(sidePanelWidget);
    moveHistoryWidget->setAlternatingRowColors(true);
    moveHistoryWidget->setMaximumWidth(220);
    moveHistoryWidget->setMinimumWidth(120);

    whiteCapturedWidget = new CapturedPiecesWidget(chessModel, true, sidePanelWidget);
    whiteCapturedWidget->setToolTip("Pieces captured by Black"); 
    whiteCapturedWidget->setMinimumHeight(100);

    statusLabel = new QLabel("Status", sidePanelWidget);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true); 
    statusLabel->setStyleSheet("font-weight: bold;");
    statusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);


    sidePanelLayout->addWidget(whiteCapturedWidget);
    sidePanelLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));
    sidePanelLayout->addWidget(new QLabel("Move History:", sidePanelWidget));
    sidePanelLayout->addWidget(moveHistoryWidget, 1);
    sidePanelLayout->addWidget(blackCapturedWidget);
    sidePanelLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));
    sidePanelLayout->addWidget(statusLabel);
    sidePanelLayout->addStretch(0); 

    sidePanelWidget->setLayout(sidePanelLayout);


    // Use QSplitter for Resizeable Board/Side Panel
    QSplitter *splitter = new QSplitter(Qt::Horizontal, centralWidget);
    splitter->addWidget(boardWidget);
    splitter->addWidget(sidePanelWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    splitter->setHandleWidth(5);

    // Add splitter to main layout
    mainLayout->addWidget(splitter);
    centralWidget->setLayout(mainLayout);

    // Status Bar
    statusBar()->showMessage(tr("Ready"));

    // Connect Menu Actions
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);
    connect(loadGameAction, &QAction::triggered, this, &MainWindow::loadGame);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}


void MainWindow::setupConnections()
{
    if(boardWidget) {
        connect(boardWidget, &ChessBoardWidget::moveAttempted, this, &MainWindow::handleMoveAttempt, Qt::AutoConnection);
    } else {
        qWarning() << "setupConnections: boardWidget is null!";
    }
}

void MainWindow::handleMoveAttempt(const Move& move) {
    if (!chessModel || !boardWidget || !dbManager || chessModel->isGameOver()) return;

    if (currentGameId < 0 && !chessModel->isGameOver()) { 
         qWarning() << "Attempted move but no active game ID is set. Move not saved.";
    }

    Piece* movingPiece = chessModel->getPiece(move.from.row, move.from.col);
    Piece* capturedPiece = chessModel->getPiece(move.to.row, move.to.col);
    bool isPawnMove = (movingPiece && movingPiece->type == 'P');
    bool isCapture = (capturedPiece != nullptr) || (movingPiece && movingPiece->type == 'P' && chessModel->getEnPassantTarget() && move.to == *(chessModel->getEnPassantTarget())); // Check EP

    std::string sanBase = Utils::moveToSAN(move, *chessModel);
    qDebug() << "Attempting move:" << QString::fromStdString(sanBase);

    bool success = chessModel->makeMove(move);

    if (success) {
        qDebug() << "Move successful:" << QString::fromStdString(sanBase);

        boardWidget->resetInteractionState(false); 
        std::string fenAfterMove = chessModel->getCurrentFEN();
        QString sanFull = QString::fromStdString(sanBase);
        if (chessModel->getIsCheckmate()) sanFull += '#';
        else if (chessModel->isInCheck()) sanFull += '+';

        bool isWhiteTurnJustEnded = !chessModel->isWhiteToMove(); 

        if (currentGameId >= 0) {
            if (!dbManager->saveMove(currentGameId, fullMoveNumber, isWhiteTurnJustEnded, move, sanFull, QString::fromStdString(fenAfterMove))) {
                 qWarning() << "Failed to save move to database for game" << currentGameId;
                 
            }
        } else {
             
             qDebug() << "Move made, but currentGameId is invalid. Move not saved to DB.";
        }

        updateMoveHistory(sanFull);
        
        if (isPawnMove || isCapture) {
            halfMoveClock = 0;
        } else {
            halfMoveClock++;
        }
        
        if (chessModel->isWhiteToMove()) { 
             fullMoveNumber++;
        }

        boardWidget->update(); 
        if (whiteCapturedWidget) whiteCapturedWidget->update();
        if (blackCapturedWidget) blackCapturedWidget->update();
        updateStatus();

        if (chessModel->isGameOver()) {
             QString endMessage;
             QString resultStr = ""; 
             if (chessModel->getIsCheckmate()) {
                 resultStr = chessModel->isWhiteToMove() ? "0-1" : "1-0"; // Loser's turn -> winner is opponent
                 endMessage = QString("Checkmate! %1 wins.")
                                  .arg(chessModel->isWhiteToMove() ? "Black" : "White");
             } else if (chessModel->getIsStalemate()) {
                 resultStr = "1/2-1/2";
                 endMessage = QString("Stalemate! Game is a draw.");
             }
             if (currentGameId >= 0 && !resultStr.isEmpty()) {
                 dbManager->finishGame(currentGameId, resultStr, QString::fromStdString(fenAfterMove));
             }

             showGameOverMessage(endMessage);
             boardWidget->setEnabled(false); 
        }

    } else {
        qDebug() << "Invalid move rejected by model.";
        statusBar()->showMessage(tr("Invalid move."), 2000);
    }
}

void MainWindow::updateMoveHistory(const QString& sanMove) {
    if (!moveHistoryWidget || !chessModel) return;

    QString historyEntry;
    bool isWhiteTurnJustEnded = !chessModel->isWhiteToMove();

    if (isWhiteTurnJustEnded) {
        historyEntry = QString("%1. %2").arg(fullMoveNumber).arg(sanMove);
        moveHistoryWidget->addItem(historyEntry);
    } else { 
        if (moveHistoryWidget->count() > 0) {
            QListWidgetItem* lastItem = moveHistoryWidget->item(moveHistoryWidget->count() - 1);
            if (lastItem) {
                historyEntry = lastItem->text() + "  " + sanMove;
                lastItem->setText(historyEntry);
            } else {
                 historyEntry = QString("%1. ... %2").arg(fullMoveNumber).arg(sanMove);
                 moveHistoryWidget->addItem(historyEntry);
            }
        } else {
             historyEntry = QString("%1. %2").arg(fullMoveNumber).arg(sanMove);
             moveHistoryWidget->addItem(historyEntry);
        }
    }

    moveHistoryWidget->scrollToBottom();
}

void MainWindow::populateMoveHistory(const QList<QString>& sanMoves) {
    if (!moveHistoryWidget) return;
    moveHistoryWidget->clear();
    int currentMoveNumberDisplay = 1; 
    QString currentLine;

    for (int i = 0; i < sanMoves.size(); ++i) {
        bool isWhiteMove = (i % 2 == 0);

        if (isWhiteMove) {
            currentLine = QString("%1. %2").arg(currentMoveNumberDisplay).arg(sanMoves[i]);
            if (i + 1 >= sanMoves.size()) {
                 moveHistoryWidget->addItem(currentLine);
            }
        } else {
            currentLine += "  " + sanMoves[i];
            moveHistoryWidget->addItem(currentLine);
            currentMoveNumberDisplay++; 
            currentLine = "";
        }
    }
     moveHistoryWidget->scrollToBottom();
}


void MainWindow::updateStatus() {
    if (!chessModel || !statusLabel) return;

    QString statusText;
    if (chessModel->isGameOver()) {
        if (chessModel->getIsCheckmate()) {
            statusText = QString("CHECKMATE! %1 wins.")
                             .arg(chessModel->isWhiteToMove() ? "Black" : "White");
        } else if (chessModel->getIsStalemate()) {
            statusText = "STALEMATE! Draw.";
        } else {
            statusText = "Game Over.";
        }
    } else if (currentGameId < 0 && moveHistoryWidget && moveHistoryWidget->count() == 0) {
        statusText = "Select New Game or Load Game.";
    }
    else {
        statusText = QString("%1 to move.")
                         .arg(chessModel->isWhiteToMove() ? "White" : "Black");
        if (chessModel->isInCheck()) {
            statusText += " (Check!)";
        }
    }
    statusLabel->setText(statusText);
}

void MainWindow::showGameOverMessage(const QString& message) {
     QMessageBox::information(this, tr("Game Over"), message);
}

void MainWindow::startNewGame() {
     fullMoveNumber = 1;
     halfMoveClock = 0;
     currentGameId = -1;

     if (chessModel) {
         chessModel->setupStartingPosition();
     } else {
         qWarning() << "startNewGame: chessModel is null!";
         return; 
     }

     if (boardWidget) {
         boardWidget->resetInteractionState();
         boardWidget->setEnabled(true); 
         boardWidget->update();
     } else {
          qWarning() << "startNewGame: boardWidget is null!";
     }
     if (moveHistoryWidget) {
         moveHistoryWidget->clear();
     }
     if (whiteCapturedWidget) whiteCapturedWidget->update();
     if (blackCapturedWidget) blackCapturedWidget->update();

     if (dbManager) {
        currentGameId = dbManager->startNewGame(); // Get the ID for the new game
        if (currentGameId < 0) {
             qWarning() << "Failed to create new game entry in database.";
             QMessageBox::warning(this, "Database Error", "Could not save the start of the new game. Moves may not be saved.");
        } else {
            qDebug() << "Started new game with ID:" << currentGameId;
        }
     } else {
         QMessageBox::warning(this, "Database Error", "Database manager not available. Game will not be saved.");
     }

     updateStatus();
     qDebug() << "New game setup complete.";
     statusBar()->showMessage(tr("New Game Started."), 3000);
}

bool MainWindow::loadGame() {
    qDebug() << "Attempting to load game...";
    if (!dbManager) {
        QMessageBox::warning(this, "Load Game Error", "Database manager is not available.");
        return false;
    }

    if (dbManager->getSavedGamesList().isEmpty()) {
        QMessageBox::information(this, "Load Game", "No saved games found.");
        return false;
    }

    GameLoadDialog loadDialog(dbManager, this);
    if (loadDialog.exec() == QDialog::Accepted) {
        qint64 selectedGameId = loadDialog.getSelectedGameId();

        qDebug() << "User selected game ID:" << selectedGameId;

        QList<QString> sanMovesList;
        if (dbManager->loadGameMoves(selectedGameId, chessModel, sanMovesList)) {
            currentGameId = selectedGameId;
            fullMoveNumber = (sanMovesList.size() / 2) + 1;
            // Determine halfMoveClock based on last moves if possible, or reset
            // For simplicity, we'll reset it here. 
            halfMoveClock = 0;

            if (boardWidget) {
                boardWidget->resetInteractionState();
                boardWidget->setEnabled(!chessModel->isGameOver()); 
                boardWidget->update();
            } else {
                 qWarning() << "loadGame: boardWidget is null!";
            }

            if (whiteCapturedWidget) whiteCapturedWidget->update();
            if (blackCapturedWidget) blackCapturedWidget->update();

            populateMoveHistory(sanMovesList);
            updateStatus(); 

            statusBar()->showMessage(QString("Loaded game %1.").arg(selectedGameId), 3000);
            qDebug() << "Game" << selectedGameId << "loaded successfully.";
            return true;

        } else {
            QMessageBox::warning(this, "Load Game Error", QString("Failed to load game data for ID %1.").arg(selectedGameId));
            currentGameId = -1;
            return false;
        }
    } else {
        qDebug() << "Load game dialog cancelled by user.";
        return false; 
    }
}
