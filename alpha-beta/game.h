#ifndef _MAIN_MENU_HH
#define _MAIN_MENU_HH

#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "network.h"
#include "../src/players.h"

class GameMenu {
private:
    Gomoku *game;
    int currentMode;

public:
    GameMenu(Gomoku *game) : game(game), currentMode(0) {}

    void displayMode();         // Display mode options.
    int  getChoice();           // Get user's choice.

    // Different modes
    void PvPMode();             // Player vs player. Integrated with pvp.cpp. Mode code : 1
    void PvEMode(int player);   // Player vs AI. Integrated with pve.cpp, evp.cpp. Mode code : 2, 3
    void networkMode();         // Players play over the network. Intergrated with network.cpp. Mode code : 4

    // Unused
    void display();

    void gameStart();           // Game start.
};

#endif