#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "gomoku.h"
#include <climits>
#include <queue>

class GomokuAI {
private:
    Gomoku *game;                                               // AI holds reference to the game instance.

public:
    bool maiximizingPlayer;                                     // Is it maximizing or minimizing?
    int  maxDepth;                                              // Max calculating depths.
    GomokuAI(Gomoku *game) : game(game) {}

    vector<pair<int, int>> getLegalMoves();                     // Get valid intersections on board.
    vector<pair<int, int>> getLegalMoves(int heuristic);        // Focus on the possible areas to reduce overhead.
    int evaluate(Gomoku &game);                                 // Evaluation of the current board.
    int evaluate(Gomoku &game, int heuristic);                  // Heuristic evaluation for optimizing.
    int makeMove(int x, int y);                                 // AI makes a move at (x, y).
    int undoMove(int x, int y);                                 // Undo a move at (x, y). Used when searching.
    int MinMax(Gomoku &game, int depth, int alpha, int beta);   // Alpha Beta Prunning.
};

#endif