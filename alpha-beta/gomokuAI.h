#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "../src/gomoku.h"
#include <climits>
#include <unordered_map>


// Weights for each position. Closer to the center, higher the weight.
const int posWeights[15][15] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 6, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 6, 6, 6, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

/*
 * TODO: Maybe need to consider more. (Nah dont think so.)
 *
 * RENJU
 * OPEN_FOURS
 * HALF_OPEN_FOURS
 * OPEN_THREES
 * HALF_OPEN_THREES
 * OPEN_TWOS
 * HALF_OPEN_TWOS
 *
 *
 * 1 for stone in our color.
 * 0 for stone in opponent's color.
 * # for blank intersection.
 * I for invalid places (out of board).
 */


// http://gomokuworld.com/gomoku/1
struct shapesLookup {
    int RENJU_SCORE,
        OFOUR_SCORE,
        HFOUR_SCORE,
        OTHREE_SCORE,
        HTHREE_SCORE,
        OTWO_SCORE,
        HTWO_SCORE;

    string  RENJU,
        OFOUR,
        HFOUR_0, HFOUR_1, HFOUR_2, HFOUR_3, HFOUR_4,
        OTHREE_0, OTHREE_1, OTHREE_2,
        HTHREE_0, HTHREE_1, HTHREE_2, HTHREE_3, HTHREE_4,
        HTHREE_5, HTHREE_6, HTHREE_7, HTHREE_8, HTHREE_9,
        OTWOS_0, OTWOS_1, OTWOS_2,
        HTWOS_0, HTWOS_1, HTWOS_2, HTWOS_3, HTWOS_4, HTWOS_5, HTWOS_6;

    shapesLookup() :
        RENJU_SCORE(5000000),
        OFOUR_SCORE(1000000),
        HFOUR_SCORE(10000),
        OTHREE_SCORE(8000),
        HTHREE_SCORE(500),
        OTWO_SCORE(50),
        HTWO_SCORE(10),

        RENJU("11111"),
        OFOUR("#1111#"),
        HFOUR_0("01111#"),
        HFOUR_1("#11110"),
        HFOUR_2("111#1"),
        HFOUR_3("1#111"),
        HFOUR_4("11#11"),
        OTHREE_0("#111#"),
        OTHREE_1("1#11#"),
        OTHREE_2("11#1#"),
        HTHREE_0("##1110"),
        HTHREE_1("##11#1"),
        HTHREE_2("##1110"),
        HTHREE_3("0111##"),
        HTHREE_4("1##11"),
        HTHREE_5("11##1"),
        HTHREE_6("1#1#1"),
        HTHREE_7("0#111#0"),
        HTHREE_8("#1#110"),
        HTHREE_9("01#11#"),
        OTWOS_0("##11##"),
        OTWOS_1("#1#1#"),
        OTWOS_2("1##1"),
        HTWOS_0("###110"),
        HTWOS_1("1###1"),
        HTWOS_2("011###"),
        HTWOS_3("##1#10"),
        HTWOS_4("#1##10"),
        HTWOS_5("01##1#"),
        HTWOS_6("01#1##")
    {}
};


class GomokuAI {
public:
    Gomoku *game;                                               // AI holds reference to the game instance.
    shapesLookup shapeTable;                                    // AI knows all the valid shapes for evaluating.
    int  maxDepth;                                              // Max calculating depth per move.

    GomokuAI(Gomoku *game) : game(game), maxDepth(7) {}

    vector<pair<int, int>> getLegalMoves();                     // Get valid intersections on board.
    vector<pair<int, int>> getLegalMoves(int heuristic);        // Focus on the possible areas to reduce overhead.
    string posToStr(int x, int y);                              // Turn a (x, y) pair to str for filling record.
    int evaluate(int player);                                   // Evaluate the current board.
    int evaluate(int player, int heuristic);                    // Heuristicly evaluate for optimizing.
    int ratePos(int x, int y, int player);                      // Rate the value of one stone in a given position.
    int makeMove(pair<int, int> move);                          // AI makes a move at (x, y).
    int undoMove(pair<int, int> move);                          // Undo a move at (x, y). Used when searching.
    pair<int, int> findBestMove();                              // Find the best move, return a (x, y) pair.
    int MinMax(int depth, int alpha, int beta, bool isMax);     // Alpha Beta Prunning.
    int getScorefromTable(string s);                            // Look up shapesLookup table to get score.

    /*
     * (1, 0)   vertical line.
     * (0, 1)   horizontal line.
     * (1, 1)   diagonal line from lt to rb.
     * (1, -1)  diagonal line from rt to lb.
     */
    template<int x_dir, int y_dir>
    string getStrFromPos(int x, int y, int player)              // Get a string consisted of 9 char as (x, y) in the middle.
    {
        string ret  = "";
        int r_begin = x - x_dir*4, c_begin = y - y_dir*4;
        int r_end   = x + x_dir*4, c_end   = y + y_dir*4;
        int cur_r   = r_begin, cur_c = c_begin;
        // cout << "r_begin: " << r_begin <<" c_begin: "<<c_begin<<" r_end: "<< r_end <<" c_end: "<<c_end<< endl;

        while (cur_r != r_end + x_dir || cur_c != c_end + y_dir) {
            if (game->on_board(cur_r, cur_c)) {        
                if (game->board[cur_r][cur_c] == 0)
                    ret += '#';
                else if (game->board[cur_r][cur_c] == player)
                    ret += '1';
                else
                    ret += '0';
            }
            else
                ret += "I";    // Invalid place.
            cur_r += x_dir;
            cur_c += y_dir;
        }

        return ret;
    }

};

#endif