#ifndef _GOMOKU_MCTS_H
#define _GOMOKU_MCTS_H

#include "../src/gomoku.h"
#include "mcts.h"
#include "state.h"

class Gomoku_state : public MCTS_state
{
public:
    Gomoku *game;
    bool isTerminal();      // The game has reached an end.
};

struct Gomoku_move : public MCTS_move
{
    Gomoku *game;
    bool operator==(const Gomoku_move& other) const override;
};

#endif
