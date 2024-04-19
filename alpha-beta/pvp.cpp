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

        // [8, 8] to make a move at 8, 8
        int x, y;
        std::cin >> x >> y;

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
        game.record.push_back(make_pair(x-1, y-1));

        if (game.state == 1) {
            game.displayBoard();
            cout << "Black wins!" << endl;
            break;
        }

        game.displayBoard();
        cout << "Black made a move at " << x << ", " << y << endl;

        // Player 2 makes a move.
        std::cin >> x >> y;

        if (x == -1 && y == -1) {
            if (!game.regret_move()) {
                game.displayBoard();
                cout << "regret a move, please continue" << endl;
            }
            
            continue;
        }

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