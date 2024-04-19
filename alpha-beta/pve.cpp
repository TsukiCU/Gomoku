#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

int main() {
    Gomoku game(1);
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.

    cout << "\n\nGame started. " << endl;
    
    game.displayBoard();
    while (1) {
        int x, y;
        std::cin >> x >> y;
        
        // User makes a move. Assume (-1, -1) means want to regret.
        if (x == -1 && y == -1) {
            if (!game.regret_move()) {
                game.displayBoard();
                cout << "regret a move, please continue" << endl;
            }
            
            continue;
        }

        else if (p1.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game.record.push_back(make_pair(x - 1, y - 1));

        if (game.state == 1) {
            game.displayBoard();
            cout << "You win!" << endl;
            break;
        }

        // AI makes a move.
        pair<int, int> bestMove= ai.findBestMove();
        ai.makeMove(bestMove);
        game.record.push_back(bestMove);

        cout << "You made a move at " << x << ", " << y << ", " << "AI made a move at " 
        << bestMove.first + 1 << ", " << bestMove.second + 1 << "\n" << endl;

        game.displayBoard();

        if (game.state == 1) {
            cout << "You lose!" << endl;
            break;
        }
    }

    return 0;
}