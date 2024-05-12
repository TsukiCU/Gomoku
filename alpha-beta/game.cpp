#include "game.h"
#include "../src/gomoku.h"

int main()
{
    Gomoku game(1);
    GameMenu menu(&game);
    menu.gameStart();
    return 0;
}