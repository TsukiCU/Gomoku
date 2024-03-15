#ifndef _GOMOKU_MCTS_H
#define _GOMOKU_MCTS_H

#include "../src/gomoku.h"
#include "mcts.h"

class Gomoku_state : public MCTS_state
{
public:
    Gomoku *game;
    bool isTerminal();      // The game has reached an end.
};

#endif
