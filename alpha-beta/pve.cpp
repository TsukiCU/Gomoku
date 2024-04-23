#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "../src/players.h"

#include "../src/xboxcont.h"
#include <libusb-1.0/libusb.h>
#define SLEEP 1


libusb_device **devs; //
libusb_context *ctx = NULL; //
libusb_device_handle *handle = NULL;


void applyEndgame(Gomoku *game)
{
    game->board[7][7] = 1;
    game->board[8][8] = 2;
    game->board[8][9] = 1;
    game->board[7][8] = 2;
    game->board[9][7] = 1;
    game->board[8][7] = 2;
}

// void applyEndgame(Gomoku *game)
// {
//     game->board[2][2] = 1;
//     game->board[2][3] = 2;
//     game->board[3][2] = 1;
//     game->board[3][3] = 2;
// }

int main() {
    Gomoku game(1);
    Player p1(&game, 1);
    GomokuAI ai(&game, 1);  // Use strategy 1 for best performance.


    //controller
    
    int result = find_xbox_controller();
    //finished opening

    cout << "\n\nGame started. " << endl;

    //applyEndgame(&game);
    
    game.displayBoard();
    while (1) {
        int x, y;
        //std::cin >> x >> y;

        getCommandXb(&handle,x,y);
        
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

    close_controller(&devs, &ctx);


    return 0;
}