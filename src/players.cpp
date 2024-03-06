#include <iostream>
#include "players.h"

using namespace std;

int Player::makeMove(pair<int, int> move)
{
    if (!game->make_move(move)) {
        // cout << "Player " << pid << " makes move at " << move.first << ", " << move.second << endl;
        return 0;
    }
    else {
        // cout << "Player " << pid << " makes move at " << move.first << ", " << move.second << endl;
        return 1;
    }
}