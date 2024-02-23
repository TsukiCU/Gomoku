#ifndef _GOMOKU_HH
#define _GOMOKU_HH

#include <vector>
#include <iostream>

class Player;
using namespace std;

class Gomoku {
private:
    Player *player1;
    Player *player2;

public:
    int state;  // 0 for ongoing, 1 for we have a winner already.
    int board_size;  // The standard Gomoku board is 15x15.
    int current_player;  // 1 for black's turn, 2 for white's turn.
    const int WIN_LENGTH;  // Ending condition: Form an unbroken line of five stones.
    std::vector<std::vector<int>> board;  // The board.

    Gomoku(Player* p1, Player* p2) : player1(p1), player2(p2), state(0), board_size(15),
    current_player(1), WIN_LENGTH(5), board(board_size, std::vector<int>(board_size, 0)) {}

    bool on_board(int x, int y);    // Check if (x, y) is a valid position.
    bool valid_move(int x, int y);  // Check if the move at (x, y) is valid.
    int  make_move(int x, int y);   // Make a move at (x, y).
    bool check_win(int x, int y);   // Check if the winning condition is satisfied.
    void reset_game();              // Reset

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

            if (this->board[cur_x][cur_y] != current_player) cumulative_stone = 0;
            else cumulative_stone++;

            /* TODO: Rigorously, six (or more) in a row is forbidden. */
            if (cumulative_stone == WIN_LENGTH) return true;
            cur_x += x_step;
            cur_y += y_step;
        }

        return false;
    }
};

#endif