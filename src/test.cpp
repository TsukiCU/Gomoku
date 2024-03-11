#include "gomoku.h"
#include "gomokuAI.h"
#include "players.h"

int main() {
    Gomoku game;
    Player p1(&game, 1);
    GomokuAI ai(&game);

    cout << "\n\nGame started. " << endl;
    game.displayBoard();
    while (1) {
        int x, y;
        std::cin >> x >> y;
        
        // User makes a move.
        if (p1.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }

        if (game.state == 1) {
            game.displayBoard();
            cout << "You win!" << endl;
            break;
        }
        
        // AI makes a move.
        pair<int, int> bestMove= ai.findBestMove();
        ai.makeMove(bestMove);

        cout << "You made a move at " << x << ", " << y << ", " << "AI made a move at " 
        << bestMove.first + 1 << ", " << bestMove.second + 1 << "\n" << endl;

        game.displayBoard();

        if (game.state == 1) {
            cout << "You lose!" << endl;
            break;
        }
    }

    return 0; // This line will never be executed
}
