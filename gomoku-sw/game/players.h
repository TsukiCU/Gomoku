#ifndef _PLAYERS_HH
#define _PLAYERS_HH
#include "gomoku.h"

struct PlayerInfo{
	int pid;                        // Player ID :)
	char name[120];
};

class Player {
    public:
    Gomoku *game;                   // Player holds reference to the game.
	PlayerInfo info{0,"Player"};	// Player information
	bool black;						// Player piece color

    Player(Gomoku *game, int id) : game(game) {info.pid=id;}     // the current player is pid == 1? Black : White.

    int makeMove(pair<int, int>);   // Player makes a move at (x, y).
	int regretMove();				// Player regrets move
	int resign();					// Player resigns
};


#endif