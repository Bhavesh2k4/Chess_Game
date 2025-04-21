#ifndef GAMELOADDIALOG_H
#define GAMELOADDIALOG_H

#include <QDialog>
#include <QList>
#include "model/DatabaseManager.h" 

class QTableWidget;
class DatabaseManager;

class GameLoadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameLoadDialog(DatabaseManager *dbManager, QWidget *parent = nullptr);
    qint64 getSelectedGameId() const;

private slots:
    void accept() override;

private:
    void setupUi();
    void populateTable();

    DatabaseManager *dbManager;
    QTableWidget *tableWidget;
    QList<GameInfo> savedGames;
    qint64 selectedGameId;
};

#endif 
