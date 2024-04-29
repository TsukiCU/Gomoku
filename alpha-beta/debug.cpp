#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

vector<vector<int>> EndList = {
    {7,7},
    {7,6},
    {6,7},
    {5,7},
    {7,8},
    {5,6},
    {8,9},
    {6,6},
    {9,10}
    };

void applyEndGame(Gomoku &game)
{
    unsigned long i;
    for (i = 0; i < EndList.size(); i++)
    {
        pair<int, int> move = make_pair(EndList[i][0], EndList[i][1]);
        game.make_move(move);
        game.record.push_back(move);
    }
}

int main() {
    Gomoku game(1);
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.

    applyEndGame(game);
    pair<int, int> move = ai.findBestMove();
    game.displayBoard();

    cout << "next move is " << move.first + 1 << " , " << move.second + 1 << endl;

    return 0;
}