﻿# ChessQt
 <img width="880" alt="image" src="https://github.com/user-attachments/assets/526019de-0336-44fc-9ec3-de5a2362d449" />


## Description

ChessQt is a C++ Qt application for playing chess. It provides a graphical user interface for interacting with the chess game, including features for loading games, saving games, and analyzing positions.

## Dependencies

- Qt6 (Core, Gui, Widgets, Sql)
- CMake (3.16 or higher)
- C++17

## Build Instructions

1.  Clone the repository:

    ```bash
    git clone <repository_url>
    ```
2.  Navigate to the project directory:

    ```bash
    cd OOAD_Project
    ```
3.  Create a build directory:

    ```bash
    mkdir build
    cd build
    ```
4.  Configure the project with CMake:

    ```bash
    cmake ..
    ```
5.  Build the project:

    ```bash
    make
    ```
6.  Run the application:

    ```bash
    ./ChessQt
    ```

## Project Structure

The project is structured into the following main components:

-   **src/main.cpp:** The entry point of the application.
-   **src/gui:** Contains the graphical user interface components, including:
    -   `BoardInteractionHandler`: Handles user interactions with the chessboard.
    -   `CapturedPiecesWidget`: Displays captured pieces.
    -   `ChessBoardWidget`: Implements the chessboard widget.
    -   `ChessView`: Manages the chess game view.
    -   `DrawingUtils`: Provides utility functions for drawing chess pieces and board elements.
    -   `GameLoadDialog`: Allows loading chess games from files.
    -   `MainWindow`: The main application window.
    -   `WelcomeDialog`: A welcome dialog displayed on startup.
-   **src/model:** Contains the data model for the chess game, including:
    -   `ChessModel`: Represents the chess game model.
    -   `Move`: Represents a chess move.
    -   `Position`: Represents a chess position.
    -   `DatabaseManager`: Manages the chess game database.
    -   `pieces`: Contains classes for different chess pieces (Bishop, King, Knight, Pawn, Piece, Queen, Rook).
-   **src/controller:** Contains the controller logic for the chess game:
    -   `ChessController`: Handles user input and updates the model and view.
-   **src/core:** Contains core utility functions:
    -   `FenUtils`: Provides utilities for working with Forsyth–Edwards Notation (FEN).
    -   `Utils`: Provides general utility functions.
