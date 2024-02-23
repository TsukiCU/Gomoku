#ifndef _PLAYERS_HH
#define _PLAYERS_HH
#include "gomoku.h"

class Gomoku;

class Player {
public:
    virtual int makeMoves(Gomoku& game) = 0;
    virtual ~Player() {}
};

class Player1 : public Player {
public:
    int x, y;  // (x, y) is the position of the current move.

    virtual ~Player1() override {}
    int makeMoves(Gomoku& game) override {
        if (!game.make_move(x, y)) {
            cout << "Player1 makes move at " << x << ", " << y << endl;
            return 0;
        }
        else {
            cout << "Invalid move at " << x << ", " << y << endl;
            return 1;
        }
    }
};

class Player2 : public Player {
public:
    int x, y;
    virtual ~Player2() override {}
    int makeMoves(Gomoku& game) {
        if (!game.make_move(x, y)) {
            cout << "Player2 makes move at " << x << ", " << y << endl;
            return 0;
        }
        else {
            cout << "Invalid move at " << x << ", " << y << endl;
            return 1;
        }
    }
};

#endif