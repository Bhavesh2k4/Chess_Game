#include "WelcomeDialog.h"
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFont>
#include <QCloseEvent>
#include <QPalette>

WelcomeDialog::WelcomeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Chess Game"));
    setModal(true); 
    setFixedSize(300, 250);  

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20); 
    mainLayout->setSpacing(15);

    titleLabel = new QLabel(tr("Welcome to Chess!"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    newGameButton = new QPushButton(tr("Start New Game"), this);
    loadGameButton = new QPushButton(tr("Load Saved Game"), this);

    QFont buttonFont = newGameButton->font();
    buttonFont.setPointSize(12);
    newGameButton->setFont(buttonFont);
    loadGameButton->setFont(buttonFont);

    newGameButton->setMinimumSize(180, 40);
    loadGameButton->setMinimumSize(180, 40);

    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(newGameButton, 0, Qt::AlignCenter);
    mainLayout->addWidget(loadGameButton, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(newGameButton, &QPushButton::clicked, this, &WelcomeDialog::onNewGameClicked);
    connect(loadGameButton, &QPushButton::clicked, this, &WelcomeDialog::onLoadGameClicked);
}

WelcomeDialog::~WelcomeDialog() {}

void WelcomeDialog::onNewGameClicked()
{
    m_choice = UserChoice::NewGame;
    accept(); 
}

void WelcomeDialog::onLoadGameClicked()
{
    m_choice = UserChoice::LoadGame;
    accept(); 
}

WelcomeDialog::UserChoice WelcomeDialog::getChoice() const
{
    return m_choice;
}

void WelcomeDialog::closeEvent(QCloseEvent *event)
{
    if (result() != QDialog::Accepted) { 
        m_choice = UserChoice::Rejected;
    }
    QDialog::closeEvent(event);
}
