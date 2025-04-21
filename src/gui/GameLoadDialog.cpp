#include "gui/GameLoadDialog.h"
#include "model/DatabaseManager.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

GameLoadDialog::GameLoadDialog(DatabaseManager *dbManager, QWidget *parent) :
    QDialog(parent),
    dbManager(dbManager),
    tableWidget(nullptr),
    selectedGameId(-1)
{
    if (!dbManager) {
        qWarning() << "GameLoadDialog created with null DatabaseManager!";
    }
    setupUi();
    populateTable();
}

qint64 GameLoadDialog::getSelectedGameId() const {
    return selectedGameId;
}

void GameLoadDialog::setupUi() {
    setWindowTitle("Load Saved Game");
    setMinimumSize(500, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);

    tableWidget = new QTableWidget(0, 5, this);
    tableWidget->setHorizontalHeaderLabels({"ID", "Start Time", "Players", "Result", "End Time"});
    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableWidget->verticalHeader()->setVisible(false);
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->setSortingEnabled(true); 

    layout->addWidget(tableWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GameLoadDialog::accept); 
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void GameLoadDialog::populateTable() {
    if (!dbManager || !tableWidget) return;

    savedGames = dbManager->getSavedGamesList();
    tableWidget->setRowCount(savedGames.count()); 

    tableWidget->setSortingEnabled(false);

    for (int i = 0; i < savedGames.count(); ++i) {
        const GameInfo& info = savedGames[i];

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(info.gameId));
        idItem->setData(Qt::UserRole, QVariant::fromValue(info.gameId));
        tableWidget->setItem(i, 0, idItem);

        tableWidget->setItem(i, 1, new QTableWidgetItem(QDateTime::fromString(info.startTime, Qt::ISODate).toString("yyyy-MM-dd hh:mm")));
        tableWidget->setItem(i, 2, new QTableWidgetItem(QString("%1 vs %2").arg(info.whitePlayer).arg(info.blackPlayer)));
        tableWidget->setItem(i, 3, new QTableWidgetItem(info.result.isEmpty() ? "In Progress" : info.result));
        tableWidget->setItem(i, 4, new QTableWidgetItem(info.endTime.isEmpty() ? "-" : QDateTime::fromString(info.endTime, Qt::ISODate).toString("yyyy-MM-dd hh:mm")));
    }

    tableWidget->resizeColumnsToContents();
    tableWidget->sortByColumn(1, Qt::DescendingOrder);
    tableWidget->setSortingEnabled(true);
}

void GameLoadDialog::accept() {
    if (!tableWidget) {
        QDialog::reject();
        return;
    }

    QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
    if (!selectedItems.isEmpty() && selectedItems.first()->row() >= 0) {
        QTableWidgetItem* idItem = tableWidget->item(selectedItems.first()->row(), 0);
        if (idItem) {
            selectedGameId = idItem->data(Qt::UserRole).toLongLong();
            qDebug() << "GameLoadDialog accepted, selected game ID:" << selectedGameId;
            QDialog::accept();
        } else {
             QMessageBox::warning(this, "Selection Error", "Could not retrieve game ID from selection.");
             selectedGameId = -1;
        }
    } else {
        QMessageBox::warning(this, "No Selection", "Please select a game to load.");
        selectedGameId = -1;
    }
}
