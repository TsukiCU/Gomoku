#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"


int main() {
    int ai_turn = 1;
    Gomoku game;
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.

    cout << "\n\nGame started. " << endl;
    
    while (1) {
        // AI makes a move.
        if (ai_turn) {
            pair<int, int> bestMove= ai.findBestMove();
            ai.makeMove(bestMove);
            ai_turn = 0;
            game.record.push_back(bestMove);

            cout <<"AI made a move at " << bestMove.first + 1 << ", " << bestMove.second + 1 << "\n" << endl;

            game.displayBoard();

            if (game.state == 1) {
                cout << "You lose!" << endl;
                break;
            }
        }

        // User makes a move.
        int x, y;
        std::cin >> x >> y;

        if (p1.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game.record.push_back(make_pair(x-1, y-1));

        if (game.state == 1) {
            game.displayBoard();
            cout << "You win!" << endl;
            break;
        }
        
        ai_turn = 1;
        cout << "You made a move at " << x << ", " << y << endl;
    }

    return 0;
}