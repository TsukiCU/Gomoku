#ifndef _GOMOKU_HH
#define _GOMOKU_HH

#include <vector>
#include <iostream>

class Player;
using namespace std;

class Gomoku {
public:
    int state;                              // 0 for ongoing, 1 for we have a winner already.
    int board_size;                         // The standard Gomoku board is 15x15.
    int current_player;                     // 1 for black's turn, 2 for white's turn.
    int winner;                             // It's useful to know who wins in MCTS. Same as current_player.
    int WIN_LENGTH;                   		// Ending condition: Form an unbroken line of five stones.
    int mode;                               // 0 for pvp, 1 for pve. This is useful when we regret a move. 
    int regretTimes;                        // Times user has regreted. Used only when online gaming and playing with AI.
    vector<vector<int>> board;              // The board.
    vector<pair<int, int>> record;          // Game record.

    Gomoku(int mode) : state(0), board_size(15), current_player(1),
    WIN_LENGTH(5), mode(mode), regretTimes(0), board(board_size, vector<int>(board_size, 0)) {}

    bool on_board(int x, int y);            // Check if (x, y) is a valid position.
    bool valid_move(int x, int y);          // Check if the move at (x, y) is valid.
    int make_move(pair<int, int> move);     // Make a move at (x, y).    vector<pair<int, int>> getLegalMoves();
    int regret_move();                      // Regret a move. If it's pvp, dial back one move. If it's pve, dial back 2 moves.

    bool check_win(int x, int y);           // Check if the winning condition is satisfied.
    bool is_draw();                         // Check if players draw.
    void switchPlayers();                   // Switch Black and white.
    void displayBoard();                    // Display board in terminal.

    void resetGame();                       // Clear board, reset everything.
    void recordGame();                      // Record game in a record/

    template<int x_step, int y_step>
    bool check(int x, int y) {
        int cur_x = x + x_step*(1-WIN_LENGTH);
        int cur_y = y + y_step*(1-WIN_LENGTH);
        int cumulative_stone = 0;

        for (int i=0; i<2*WIN_LENGTH-1; i++) {
            if (!on_board(cur_x, cur_y)) {
                cur_x += x_step;
                cur_y += y_step;
                continue;
            }

            if (this->board[cur_x][cur_y] != current_player)
                cumulative_stone = 0;
            else cumulative_stone++;

            /* TODO: Rigorously, six (or more) in a row is forbidden. */
            if (cumulative_stone == WIN_LENGTH)
                return true;
            cur_x += x_step;
            cur_y += y_step;
        }

        return false;
    }
};

#endif