#include "gomoku.h"
#include "gomokuAI.h"
#include "players.h"

int main() {
    Gomoku game;
    Player p1(&game, 1);
    GomokuAI ai(&game);

    cout << "\n\nGame started. " << endl;
    while (1) {
        int x, y;
        std::cin >> x >> y;
        
        // User makes a move.
        p1.makeMove(make_pair(x, y));
        cout << "You made a move at " << x << ", " << y << endl;
        
        // AI makes a move.
        pair<int, int> bestMove= ai.findBestMove();
        ai.makeMove(bestMove);
        cout << "AI made a move at " << bestMove.first << ", " << bestMove.second << "\n" << endl;

        if (game.state == 1) {
            cout << "Good Game!" << endl;
            break;
        }
    }

    return 0; // This line will never be executed
}
