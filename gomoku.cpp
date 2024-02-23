#include <iostream>
#include <vector>
#include "gomoku.h"

bool Gomoku::on_board(int x, int y)
{
    return x >= 0 && x < board_size && y >= 0 && y < board_size;
}

bool Gomoku::valid_move(int x, int y)
{
    return on_board(x, y) && board[x][y] == 0;
}

int Gomoku::make_move(int x, int y)
{
    if (!valid_move(x, y))  return 1;
    board[x][y] = current_player;
    if (check_win(x, y)) {
        state = 1;
        return 0;
    }
    current_player = 3 - current_player; // switch players.
    return 0;
}

/*
 * [WIN_LENGTH] of stones in a row.
 * Can be horizontally, vertically, or diagonally.
 */
bool Gomoku::check_win(int x, int y)
{
    // check horizontally
    if (check<1, 0>(x, y))
        return true;

    // check vertically
    if (check<0, 1>(x, y))
        return true;

    // check diagonally
    if (check<1, 1>(x, y) || check<-1, 1>(x, y))
        return true;

    return false;
}

void Gomoku::reset_game()
{
    board.assign(board_size, std::vector<int>(board_size, 0));
    current_player = 1;
}