#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

void applyEndgame(Gomoku *game)
{
    game->board[7][7] = 1;
    game->board[8][8] = 2;
    // game->board[8][9] = 1;
    // game->board[7][8] = 2;
    // game->board[9][7] = 1;
    // game->board[8][8] = 1;
    // game->board[7][9] = 1;
}

int main() {
    Gomoku game;
    Player p1(&game, 1);
    GomokuAI ai(&game, 3);

    cout << "\n\nGame started. " << endl;

    //applyEndgame(&game);
    
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

    return 0;
}