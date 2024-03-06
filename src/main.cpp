#include "gomoku.h"
#include "players.h"
#include "gomokuAI.h"
#include <iostream>

using namespace std;

// testing records. Black should win at the ninth round.
// {5, 5} -> {5, 9}
vector<vector<int>> records = {
{5, 5}, {6, 5}, {5, 6}, {6, 6},
{5, 7}, {6, 7}, {5, 8}, {6, 8},
{5, 9}, {1, 1}, {2, 2}, {3, 3}
};

int main()
{
    Gomoku game;
    Player black(&game, 1);
    Player white(&game, 2);

    for (int i=0; i<12; i++) {
        if (i%2 == 0) {
            // Black's turn
            black.makeMove(make_pair(records[i][0], records[i][1]));

            if (game.state == 1) {
                cout << "Black wins!" << endl;
                break;
            }
        }

        else {
            // White's turn
            white.makeMove(make_pair(records[i][0], records[i][1]));

            if (game.state == 1) {
                cout << "White wins!" << endl;
                break;
            }
        }

        cout << endl;
    }
}

