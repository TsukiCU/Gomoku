#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "gomoku.h"
#include <climits>
#include <unordered_map>

/*
 * TODO: Maybe need to consider more. (Nah dont think so.)
 *
 * Possible Shapes on the board. '+' represents
 * black and 'o' represents white here.
 *
 * RENJU                +++++
 * OPEN_FOURS           ++++
 * HALF_OPEN_FOURS      o++++, o++ ++
 * OPEN_THREES          +++
 * HALF_OPEN_THREES     o+++, o+ ++
 * OPEN_TWOS            ++
 * HALF_OPEN_TWOS       o++, o+ +
 *
 */

struct shapesOnBoard {
    string  name;                                           // Name of the shape.
    int     score;                                          // Corresponding score.
    int     count;                                          // Times it appears on the board.
};

class GomokuAI {
private:
    Gomoku *game;                                           // AI holds reference to the game instance.

public:
    int  maxDepth;                                          // Max calculating depth per move.
    unordered_map<string, shapesOnBoard>    shape_map;      // Store the shapes on the board.
    unordered_map<string, int>              record;         // Avoid repetitive counting when evaluating.

    GomokuAI(Gomoku *game) : game(game) {
        initShapeMap();
    }

    vector<pair<int, int>> getLegalMoves();                 // Get valid intersections on board.
    vector<pair<int, int>> getLegalMoves(int heuristic);    // Focus on the possible areas to reduce overhead.
    void initShapeMap();                                    // Initialize shape maps.
    string posToStr(int x, int y);                          // Turn a (x, y) pair to str for filling record.
    int evaluate(Gomoku &game);                             // Evaluate the current board.
    int evaluate(Gomoku &game, int heuristic);              // Heuristicly evaluate for optimizing.
    int makeMove(pair<int, int> move);                      // AI makes a move at (x, y).
    int undoMove(pair<int, int> move);                      // Undo a move at (x, y). Used when searching.
    int MinMax(Gomoku &game, int depth, int alpha,          // Alpha Beta Prunning.
                int beta, bool isMax);
};

#endif