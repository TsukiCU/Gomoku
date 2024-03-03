#include "gomokuAI.h"

string GomokuAI::posToStr(int x, int y)
{
    return to_string(x) + to_string(y);
}

void GomokuAI::initShapeMap()
{
    shape_map["RENJU"]              = shapesOnBoard{"RENJU",            10000, 0};
    shape_map["OPEN_FOURS"]         = shapesOnBoard{"OPEN_FOURS",       9000,  0};
    shape_map["HALF_OPEN_FOURS"]    = shapesOnBoard{"HALF_OPEN_FOURS",  5000,  0};
    shape_map["OPEN_THREES"]        = shapesOnBoard{"OPEN_THREES",      3000,  0};
    shape_map["HALF_OPEN_THREES"]   = shapesOnBoard{"HALF_OPEN_THREES", 1500,  0};
    shape_map["OPEN_TWOS"]          = shapesOnBoard{"OPEN_TWOS",        1000,  0};
    shape_map["HALF_OPEN_TWOS"]     = shapesOnBoard{"HALF_OPEN_TWOS",   500,   0};
}

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

int GomokuAI::makeMove(pair<int, int> move)
{
    if (!game->make_move(move)) {
        cout << "AI makes move at " << move.first << ", " << move.second << endl;
        return 0;
    }
    else {
        cout << "Invalid move at " << move.first << ", " << move.second << endl;
        return 1;
    }
}

int GomokuAI::undoMove(pair<int, int> move)
{
    int x = move.first, y = move.second;
    game->board[x][y] = 0;

    return 0;
}

int GomokuAI::MinMax(Gomoku &game, int depth, int alpha, int beta, bool isMax)
{
    if (!depth || game.state == 1)
        return evaluate(game);

    if (isMax) {
        int maxEval = INT_MIN;
        for (auto& move:getLegalMoves()) {
            makeMove(move);
            int eval = MinMax(game, depth-1, alpha, beta, false);   // Now minimizing.
            undoMove(move);
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);   // Update alpha.
            if (beta <= alpha)
                break;      // Beta prunning.
        }
        return maxEval;
    }
    else {
        int minEval = INT_MAX;
        for (auto& move:getLegalMoves()) {
            makeMove(move);
            int eval = MinMax(game, depth-1, alpha, beta, true);    // Now maximizing.
            undoMove(move);
            minEval = min(minEval, eval);
            beta = min(beta, eval);
            if (beta <= alpha)
                break;      // Alpha prunning.
        }
        return minEval;
    }
}