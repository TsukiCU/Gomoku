#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

int main() {
    Gomoku game(0);
    Player p1(&game, 1);
    Player p2(&game, 2);

    cout << "\n\nGame started. " << endl;
    game.displayBoard();

    while (1) {
        // Player 1 makes a move.
        int x, y;
        std::cin >> x >> y;

        if (p1.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game.record.push_back(make_pair(x-1, y-1));
        game.displayBoard();

        if (game.state == 1) {
            game.displayBoard();
            cout << "Black wins!" << endl;
            break;
        }
        
        cout << "Black made a move at " << x << ", " << y << endl;

        // Player 2 makes a move.
        std::cin >> x >> y;

        if (p2.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game.record.push_back(make_pair(x-1, y-1));

        if (game.state == 1) {
            game.displayBoard();
            cout << "White wins!" << endl;
            break;
        }
        game.displayBoard();
        
        cout << "White made a move at " << x << ", " << y << endl;
    }

    return 0;
}