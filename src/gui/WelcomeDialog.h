#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>

class QPushButton;
class QLabel;
class QVBoxLayout;

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    enum class UserChoice {
        Rejected,
        NewGame,
        LoadGame
    };

    explicit WelcomeDialog(QWidget *parent = nullptr);
    ~WelcomeDialog();

    UserChoice getChoice() const;

private slots:
    void onNewGameClicked();
    void onLoadGameClicked();

protected:
    void closeEvent(QCloseEvent *event) override; 

private:
    QLabel *titleLabel;
    QPushButton *newGameButton;
    QPushButton *loadGameButton;
    QVBoxLayout *mainLayout;

    UserChoice m_choice = UserChoice::Rejected;
};

#endif 