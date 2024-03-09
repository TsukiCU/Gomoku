#ifndef _PLAYERS_HH
#define _PLAYERS_HH
#include "gomoku.h"

class Player {
    public:
    Gomoku *game;                   // Player holds reference to the game.
    int pid;                        // Player ID :)

    Player(Gomoku *game, int id) : game(game), pid(id) {}     // the current player is pid == 1? Black : White.

    int makeMove(pair<int, int>);   // Player makes a move at (x, y).
};


#endif