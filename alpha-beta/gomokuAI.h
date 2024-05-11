#ifndef __GOMOKUAI_HH
#define __GOMOKUAI_HH

#include "../src/gomoku.h"
#include <climits>
#include <map>
#include <cassert>
#include <algorithm>


// Weights for each position. Closer to the center, higher the weight.
const int posWeights[15][15] =
{
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0},
    {0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 6, 6, 6, 6, 6, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 6, 7, 7, 7, 6, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 6, 7, 8, 7, 6, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 6, 7, 7, 7, 6, 4, 3, 2, 1, 0},
    {0, 1, 2, 3, 4, 6, 6, 6, 6, 6, 4, 3, 2, 1, 0},
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
 * XXX: Didn't really take I into consideration which will probably cause bugs
 * when the game reaches the edge but it should be very unlikely to happen.
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
        OTHREE_0, OTHREE_1, OTHREE_2, OTHREE_3, OTHREE_4,
        HTHREE_0, HTHREE_1, HTHREE_2, HTHREE_3, HTHREE_4,
        HTHREE_5, HTHREE_6, HTHREE_7, HTHREE_8, HTHREE_9,
        OTWOS_0, OTWOS_1, OTWOS_2,
        HTWOS_0, HTWOS_1, HTWOS_2, HTWOS_3, HTWOS_4, HTWOS_5, HTWOS_6;

    shapesLookup() :
    // In the case in <branch debug> debug.cpp, the threshold is 33809.
    // 33809 ❌ 33810 ✅
        RENJU_SCORE(5000000),
        OFOUR_SCORE(100000),
        HFOUR_SCORE(50000),
        OTHREE_SCORE(10000),
        HTHREE_SCORE(700),
        OTWO_SCORE(80),
        HTWO_SCORE(20),

        RENJU("11111"),
        OFOUR("#1111#"),

        HFOUR_0("01111#"),
        HFOUR_1("#11110"),
        HFOUR_2("111#1"),
        HFOUR_3("1#111"),
        HFOUR_4("11#11"),

        OTHREE_0("##111##"),
        OTHREE_1("0#111##"),
        OTHREE_2("##111#0"),
        OTHREE_3("#1#11#"),
        OTHREE_4("#11#1#"),

        HTHREE_0("##1110"),
        HTHREE_1("#11#10"),
        HTHREE_2("011#1#"),
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

struct VectorComparer {
    bool operator()(const vector<pair<int, int>> v1, const vector<pair<int, int>> v2) const {
    // v1 < v2 return true. otherwise return false.
        if (v1.size() != v2.size()) {
            return v1.size() < v2.size();
        }
        auto sorted_v1 = v1;
        auto sorted_v2 = v2;

        sort(sorted_v1.begin(), sorted_v1.end());
        sort(sorted_v2.begin(), sorted_v2.end());

        return sorted_v1 < sorted_v2;
    }
};

class GomokuAI {
public:
    Gomoku *game;                                               // AI holds reference to the game instance.
    shapesLookup shapeTable;                                    // AI knows all the valid shapes for evaluating.
    int  maxDepth;                                              // Max calculating depth per move.
    int  strategy;                                              // The aggresive degree of AI. (1->3)

    GomokuAI(Gomoku *game, int strategy):
    game(game), maxDepth(5), strategy(strategy) {}

    vector<pair<int, int>> getLegalMoves();                     // Get valid intersections on board.
    vector<pair<int, int>> getLegalMoves(bool heuristic);       // Focus on the possible areas to reduce overhead.
    string posToStr(int x, int y);                              // Turn a (x, y) pair to str for filling record.
    int evaluate(int player);                                   // Evaluate the current board.
    int evaluate(int player, int heuristic);                    // Heuristicly evaluate for optimizing.
    int ratePos(int x, int y, int player);                      // Rate the value of one stone in a given position.
    int makeMove(pair<int, int> move);                          // AI makes a move at (x, y).
    int undoMove(pair<int, int> move);                          // Undo a move at (x, y). Used when searching.
    pair<int, int> findBestMove();                              // Find the best move, return a (x, y) pair.
    int MiniMax(int depth, int alpha, int beta, bool isMax);    // Alpha Beta Prunning.    
    int getScorefromTable(string s);                            // Look up shapesLookup table to get score.

    // Beginnings.
    pair<int, int> decideThirdMove();   // AI plays black and it's the third move.

    // The fourth move. Apply the strongest known defense.
    // https://zhuanlan.zhihu.com/p/549399379
    // [花月, Kagetsu], [雨月, Ugetsu]
    pair<int, int> decideFourthMove();
    pair<int, int> isKagestu(pair<int, int> bestMove);
    pair<int, int> isUgetsu(pair<int, int> bestMove);
    pair<int, int> finishMove();

    // Another way of doing all this.
    map<vector<pair<int, int>>, pair<int, int>, VectorComparer> OpeningMap {
        // ai as white
        {{{6, 7}, {7, 7}, {7, 6}}, {5, 6}},
        {{{6, 7}, {7, 7}, {7, 8}}, {5, 8}},
        {{{8, 7}, {7, 7}, {7, 6}}, {9, 6}},
        {{{8, 7}, {7, 7}, {7, 8}}, {9, 8}},
        {{{6, 6}, {7, 7}, {6, 8}, {6, 7}, {5, 7}}, {5, 6}},

        // ai as black
        {{{7, 7}, {6, 6}, {6, 8}, {8, 6}}, {9, 7}},
        {{{7, 7}, {6, 6}, {8, 6}, {8, 8}}, {6, 8}},
        {{{7, 7}, {6, 8}, {8, 8}, {6, 6}}, {7, 5}}
    };

    /* vertical line.
     * (1, 0)  
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