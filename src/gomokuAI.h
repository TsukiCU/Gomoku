#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "gomoku.h"
#include <climits>
#include <unordered_map>


enum states {

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
 */


// http://gomokuworld.com/gomoku/1
struct shapesLookup {
    const int RENJU_SCORE   = 100000;
    const int OFOUR_SCORE   = 50000;
    const int HFOUR_SCORE   = 5000;
    const int OTHREE_SCORE  = 3000;
    const int HTHREE_SCORE  = 500;
    const int OTWO_SCORE    = 50;
    const int HTWO_SCORE    = 10;

    const string RENJU      = "11111";
    const string OFOUR      = "#1111#";
    const string HFOUR_0    = "01111#";
    const string HFOUR_1    = "#11110";
    const string HFOUR_2    = "111#1";
    const string HFOUR_3    = "1#111";
    const string HFOUR_4    = "11#11";
    const string OTHREE_0   = "#111#";
    const string OTHREE_1   = "1#11#";
    const string OTHREE_2   = "11#1#";
    const string HTHREE_0   = "##1110";
    const string HTHREE_1   = "##11#1";
    const string HTHREE_2   = "##1110";
    const string HTHREE_3   = "0111##";
    const string HTHREE_4   = "1##11";
    const string HTHREE_5   = "11##1";
    const string HTHREE_6   = "1#1#1";
    const string HTHREE_7   = "0#111#0";
    const string HTHREE_8   = "#1#110";
    const string HTHREE_9   = "01#11#";
    const string OTWOS_0    = "##11##";
    const string OTWOS_1    = "#1#1#";
    const string OTWOS_2    = "1##1";
    const string HTWOS_0    = "###110";
    const string HTWOS_1    = "1###1";
    const string HTWOS_2    = "011###";
    const string HTWOS_3    = "##1#10";
    const string HTWOS_4    = "#1##10";
    const string HTWOS_5    = "01##1#";
    const string HTWOS_6    = "01#1##";
};


class GomokuAI {
public:
    Gomoku *game;                                               // AI holds reference to the game instance.
    shapesLookup shapeTable;                                    // AI knows all the valid shapes for evaluating.
    int  maxDepth;                                              // Max calculating depth per move.

    GomokuAI(Gomoku *game) : game(game) {}

    vector<pair<int, int>> getLegalMoves();                     // Get valid intersections on board.
    vector<pair<int, int>> getLegalMoves(int heuristic);        // Focus on the possible areas to reduce overhead.
    void initShapeMap();                                        // Initialize shape maps.
    string posToStr(int x, int y);                              // Turn a (x, y) pair to str for filling record.
    int evaluate(int player);                                   // Evaluate the current board.
    int evaluate(int player, int heuristic);                    // Heuristicly evaluate for optimizing.
    int ratePos(int x, int y, int player);                      // Rate the value of one stone in a given position.
    int makeMove(pair<int, int> move);                          // AI makes a move at (x, y).
    int undoMove(pair<int, int> move);                          // Undo a move at (x, y). Used when searching.
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
        string ret = "";
        int r_begin = x - x_dir*4, c_begin = y - y_dir*4;
        int r_end   = x + x_dir*4, c_end   = y + y_dir*4;
        int cur_r = r_begin, cur_c = c_begin;

        while (cur_r != r_end || cur_c != c_end) {
            if (game->valid_move(cur_r, cur_c)) {
                if (game->board[cur_r][cur_c] == 0)
                    ret += '#';
                else if (game->board[cur_r][cur_c] == player)
                    ret += '1';
                else
                    ret += '0';
                cur_r += x_dir;
                cur_c += y_dir;
            }
            else
                ret += "I";    // Invalid place.
        }

        return ret;
    }

};

#endif