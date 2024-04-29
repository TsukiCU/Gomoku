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

vector<vector<int>> toyList = {
    {2,2},
    {3,3},
    {2,1},
    {2,0},
    {2,3},
    {3,2},
    {4,0},
    //{1,4}
    };

void applyEndGame(Gomoku &game)
{
    unsigned long i;
    
    for (i = 0; i < EndList.size(); i++)
    //for (i = 0; i < toyList.size(); i++)
    {
        pair<int, int> move = make_pair(EndList[i][0], EndList[i][1]);
        //pair<int, int> move = make_pair(toyList[i][0], toyList[i][1]);
        game.make_move(move); 
        game.record.push_back(move);
    }
}

int main() {
    Gomoku game(1);
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.

    applyEndGame(game);
    // cout<<game.record.size()<<endl;
    // cout<<ai.getLegalMoves().size()<<endl;

    //cout << "evaluation is " << ai.evaluate(2) << endl;
    //assert(game.current_player == 2);
    pair<int, int> move = ai.findBestMove();
    //game.displayBoard();

    cout << "next move is " << move.first + 1 << " , " << move.second + 1 << endl;

    return 0;
}