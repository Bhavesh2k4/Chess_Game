#include <QApplication>
#include "gui/MainWindow.h"
#include "gui/WelcomeDialog.h"
#include "model/ChessModel.h"
#include "gui/ChessView.h"
#include "controller/ChessController.h"
#include <iostream>
#include <string>
#include <vector>
int main(int argc, char *argv[])
{
    bool consoleMode = false;
    std::vector<std::string> args(argv + 1, argv + argc); // Get command line arguments

    // Check for a simple "--console" flag
    for (const std::string& arg : args) {
        if (arg == "--console" || arg == "-c") {
            consoleMode = true;
            break;
        }
    }

    if (consoleMode) {
        std::cout << "Running in Console Mode...\n";
        ChessModel model;
        ChessView view;
        ChessController controller(&model, &view);
        controller.run();
        return 0;
    } else {
        // --- GUI Mode ---
        // Create the Qt Application instance
        QApplication app(argc, argv);

        while (true) {
            // Show Welcome Dialog
            WelcomeDialog welcomeDialog;
            if (welcomeDialog.exec() != QDialog::Accepted) {
                return 0;
            }

            WelcomeDialog::UserChoice choice = welcomeDialog.getChoice();
            MainWindow *mainWindow = new MainWindow();

            if (choice == WelcomeDialog::UserChoice::NewGame) {
                mainWindow->startNewGame();
                mainWindow->show();
                return app.exec();
            } else if (choice == WelcomeDialog::UserChoice::LoadGame) {
                if (mainWindow->loadGame()) {
                    mainWindow->show();
                    return app.exec();
                } else {
                    delete mainWindow; 
                    continue;
                }
            } else {
                delete mainWindow;
                return 1;
            }
        }
    }
}
