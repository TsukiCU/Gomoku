#include "gomoku.h"
#include "players.h"
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
    Player1 black;
    Player2 white;
    Gomoku game(&black, &white);

    for (int i=0; i<12; i++) {
        if (i%2 == 0) {
            // Black's turn
            black.x = records[i][0];
            black.y = records[i][1];
            black.makeMoves(game);

            if (game.state == 1) {
                cout << "Black wins!" << endl;
                break;
            }
        }

        else {
            // White's turn
            white.x = records[i][0];
            white.y = records[i][1];
            white.makeMoves(game);

            if (game.state == 1) {
                cout << "White wins!" << endl;
                break;
            }
        }

        cout << endl;
    }
}

