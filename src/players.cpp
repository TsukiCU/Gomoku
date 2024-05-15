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

int Player::regretMove()
{
	if(!game->regret_move())
		return 0;
	return 1;
}

int Player::resign()
{
	game->end_game(black);
	return 0;
}