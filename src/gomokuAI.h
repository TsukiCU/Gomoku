#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "gomoku.h"

class GomokuAI {
private:
    Gomoku *game;

public:
    GomokuAI(Gomoku *game) : game(game) {}

    int evaluate(vector<vector<int>> &board);                   // Evaluation of the current board.
    int evaluate(vector<vector<int>> &board, int heuristic);    // TODO: heuristic evaluation method.
};

#endif