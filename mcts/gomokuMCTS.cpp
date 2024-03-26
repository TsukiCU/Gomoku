#include "gomokuMCTS.h"

bool Gomoku_state::isTerminal()
{
    return game->state == 1 || game->is_draw();
}