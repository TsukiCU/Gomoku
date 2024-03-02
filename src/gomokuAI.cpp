#include "gomokuAI.h"

vector<pair<int, int>> GomokuAI::getLegalMoves()
{
    vector<pair<int, int>> legalMoves;
    for (int row=0; row<game->board_size; row++)
        for (int col=0; col<game->board_size; col++)
            if (!game->board[row][col])
                legalMoves.push_back(make_pair(row, col));

    return legalMoves;
}

vector<pair<int, int>> GomokuAI::getLegalMoves(int heuristic)
{
    /* TODO: Heuristic searching.
     * Probably no need to store every possible move on the board.
     */
    vector<pair<int, int>> legalMoves;
    return legalMoves;
}

int GomokuAI::evaluate(Gomoku &game)
{
    return 0;
}

int GomokuAI::evaluate(Gomoku &game, int heuristic)
{
    /* TODO: Heuristic evaluation. */
    return 0;
}

int GomokuAI::makeMove(int x, int y)
{
    if (!game->make_move(x, y)) {
        cout << "AI makes move at " << x << ", " << y << endl;
        return 0;
    }
    else {
        cout << "Invalid move at " << x << ", " << y << endl;
        return 1;
    }
}

int GomokuAI::MinMax(Gomoku &game, int depth, int alpha, int beta)
{
    return 0;
}