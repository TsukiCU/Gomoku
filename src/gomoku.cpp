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

int Gomoku::make_move(pair<int, int> move)
{
    int x = move.first;
    int y = move.second;

    if (!valid_move(x, y))  return 1;
    board[x][y] = current_player;
    if (check_win(x, y)) {
        state = 1;
        return 0;
    }
    switchPlayers();
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

void Gomoku::switchPlayers()
{
    current_player = 3 - current_player;
}

void Gomoku::displayBoard()
{
    // [n] -    -    -    -    O    X    -    -    -    -    -    -    -    -    -
    // n is row index, '-' refers to there's no stone on this intercection.
    // 'X' refers to there is a black stone, 'O' refers to white.

    string filler = "   ";

    cout << '\n' << filler << "  ";
    for (int i = 0; i < board_size; i++) {
        if (i < 9)
            cout << i + 1 << filler;
        else
            cout << i + 1 << "  ";
    }
    cout << '\n';

    for (int i = 0; i < board_size; i++) {
        if (i < 9)
            cout << ' ' << i + 1 << filler;
        else
            cout << i + 1 << filler;

        for (int j = 0; j < board_size; j++) {
            switch (board[i][j]) {
                case 0: {cout << '-' << filler; break;}
                case 1: {cout << 'X' << filler; break;}
                case 2: {cout << 'O' << filler; break;}
            }
        }
        cout << "\n\n";
    }
    cout << endl;
}