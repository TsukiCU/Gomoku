#include <fstream>

#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

void applyEndgame(Gomoku *game)
{
    game->board[7][7] = 1;
    game->board[8][8] = 2;
    game->board[8][9] = 1;
    game->board[7][8] = 2;
    // game->board[9][7] = 1;
    // game->board[8][8] = 1;
    // game->board[7][9] = 1;
}

void recordGame(vector<pair<int, int>> record)
{
    ofstream fd("record.txt");

    for (auto move:record) {
        int x = move.first, y = move.second;
        fd << x << "  " << y << endl;
    }

    fd.close();
}

int main()
{
    Gomoku game;
    GomokuAI black(&game, 1);
    GomokuAI white(&game, 3);
    vector<pair<int, int>> record = {};
    pair<int, int> move;

    applyEndgame(&game);
    // game.displayBoard();

    while (1) {
        move = black.findBestMove();
        black.makeMove(move);
        record.push_back(move);

        if (game.state == 1) {
            cout << "Black wins." << endl;
            break;
        }

        move = white.findBestMove();
        white.makeMove(move);
        record.push_back(move);

        if (game.state == 1) {
            cout << "White wins." << endl;
            break;
        }
    }

    recordGame(record);
    game.displayBoard();
}