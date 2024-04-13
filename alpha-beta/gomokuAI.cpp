#include "gomokuAI.h"

string GomokuAI::posToStr(int x, int y)
{
    return to_string(x) + to_string(y);
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

vector<pair<int, int>> GomokuAI::getLegalMoves(bool heuristic)
{
    /* So traverse the board in a spiral way from inside out.
     * 
     * This is basically https://leetcode.cn/problems/spiral-matrix/ in reversing direction.
     */

    vector<pair<int, int>> legalMoves;
    int row = game->board_size, col = game->board_size;
    int x = int((row-1)/2), y = int((col-1)/2);
    int steps = 1;   // Initial step size
    int di = 0;     // Start direction index, begin with moving right
    std::vector<std::pair<int, int>> directions = {
    {0, 1},  // Right
    {1, 0},  // Down
    {0, -1}, // Left
    {-1, 0}  // Up
    };
    int moves = 1;  // Number of handled places.

    if(!game->board[x][y])
        legalMoves.push_back(make_pair(x, y));

    while (moves < row*col) {
        for (int i=0; i<2; i++) {
            for (int j=0; j<steps; j++) {
                int nx = x + directions[di].first;
                int ny = y + directions[di].second;
                if (game->on_board(nx, ny)) {
                    if (!game->board[nx][ny])
                        legalMoves.push_back(make_pair(nx, ny));
                    moves ++;
                    x = nx;
                    y = ny;
                }
            }
            if (moves >= row*col) {
                return legalMoves;
            }
            di = (di+1) % 4;
        }
        steps += 1;
    }

    return legalMoves;
}

int GomokuAI::getScorefromTable(string s)
{
    int score = 0;

    // RENJU
    if (s.find(shapeTable.RENJU) != string::npos)
        score += shapeTable.RENJU_SCORE;

    // OPEN_FOURS
    if (s.find(shapeTable.OFOUR) != string::npos)
        score += shapeTable.OFOUR_SCORE;

    // HALF_OPEN_FOURS
    if (s.find(shapeTable.HFOUR_0) != string::npos ||
        s.find(shapeTable.HFOUR_1) != string::npos ||
        s.find(shapeTable.HFOUR_2) != string::npos ||
        s.find(shapeTable.HFOUR_3) != string::npos ||
        s.find(shapeTable.HFOUR_4) != string::npos)
        {score += shapeTable.HFOUR_SCORE;}

    // OPEN_THREES
    if (s.find(shapeTable.OTHREE_0) != string::npos ||
        s.find(shapeTable.OTHREE_1) != string::npos ||
        s.find(shapeTable.OTHREE_2) != string::npos)
        {score += shapeTable.OTHREE_SCORE;}

    // HALF_OPEN_THREES
    if (s.find(shapeTable.HTHREE_0) != string::npos||
        s.find(shapeTable.HTHREE_1) != string::npos||
        s.find(shapeTable.HTHREE_2) != string::npos||
        s.find(shapeTable.HTHREE_3) != string::npos||
        s.find(shapeTable.HTHREE_4) != string::npos||
        s.find(shapeTable.HTHREE_5) != string::npos||
        s.find(shapeTable.HTHREE_6) != string::npos||
        s.find(shapeTable.HTHREE_7) != string::npos||
        s.find(shapeTable.HTHREE_8) != string::npos||
        s.find(shapeTable.HTHREE_9) != string::npos)
        {score += shapeTable.HTHREE_SCORE;}


    // OPEN_TWOS
    if (s.find(shapeTable.OTWOS_0) != string::npos ||
        s.find(shapeTable.OTWOS_1) != string::npos ||
        s.find(shapeTable.OTWOS_2) != string::npos)
        {score += shapeTable.OTWO_SCORE;}

    // HALF_OPEN_TWOS
    if (s.find(shapeTable.HTWOS_0) != string::npos ||
        s.find(shapeTable.HTWOS_1) != string::npos ||
        s.find(shapeTable.HTWOS_2) != string::npos ||
        s.find(shapeTable.HTWOS_3) != string::npos ||
        s.find(shapeTable.HTWOS_4) != string::npos ||
        s.find(shapeTable.HTWOS_5) != string::npos ||
        s.find(shapeTable.HTWOS_6) != string::npos)
        {score += shapeTable.HTWO_SCORE;}

    return score;
}

int GomokuAI::ratePos(int x, int y, int player)
{
    // int dirs[4][4] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    /* WTF? I can't even do this using loops? What kind of language is this? */

    int score = 0;
    int weight = posWeights[x][y];
    string s;

    s = getStrFromPos<1, 0>(x, y, player);
    score += getScorefromTable(s);
    s = getStrFromPos<0, 1>(x, y, player);
    score += getScorefromTable(s);
    s = getStrFromPos<1, 1>(x, y, player);
    score += getScorefromTable(s);
    s = getStrFromPos<1, -1>(x, y, player);
    score += getScorefromTable(s);

    return weight * score;
}

int GomokuAI::evaluate(int player)
{
    /* TODO: May need rethinking */
    int my_score = 0, op_score = 0;
    int state = 0;

    for (int row = 0; row < game->board_size; row++)
        for (int col = 0; col < game->board_size; col++) {
            state = game->board[row][col];
            if (!state)
                continue;
            else if (state == player)
                my_score += ratePos(row, col, player);
            else
                op_score += ratePos(row, col, 3-player);
        }

    switch(strategy) {
        case 1: {
            return  my_score - 3*op_score;
            break;
        }

        case 2: {
            return  my_score - op_score;
            break;
        }

        case 3: {
            return  3*my_score - op_score;
            break;
        }

        default: {
            // should not get here.
            cout << "Warning: Set a strategy for AI." << endl;
            return my_score - op_score;
        }
    }
    
}

int GomokuAI::evaluate(int player, int heuristic)
{
    /* TODO: Heuristic evaluation. */
    return 0;
}

int GomokuAI::makeMove(pair<int, int> move)
{
    if (!game->make_move(move)) {
        // cout << "AI makes move at " << move.first << ", " << move.second << endl;
        return 0;
    }
    else {
        // cout << "Invalid move at " << move.first << ", " << move.second << endl;
        return 1;
    }
}

int GomokuAI::undoMove(pair<int, int> move)
{
    int x = move.first, y = move.second;
    game->board[x][y] = 0;

    if (game->state == 1)
        game->state = 0;

    return 0;
}

pair<int, int> GomokuAI::findBestMove()
{
    pair<int, int> bestMove = {-1, -1};
    int bestScore = INT_MIN;

    for(auto& move:getLegalMoves()) {
        makeMove(move);
        int score = MiniMax(maxDepth, INT_MAX, INT_MIN, true);
        undoMove(move);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

int GomokuAI::MiniMax(int depth, int alpha, int beta, bool isMax)
{
    if (!depth || game->state == 1) {
        return evaluate(game->current_player);
    }

    if (isMax) {
        int maxEval = INT_MIN;
        for (auto& move:getLegalMoves()) {
            makeMove(move);
            int eval = MiniMax(depth-1, alpha, beta, false);   // Now minimizing.
            undoMove(move);
            maxEval = max(maxEval, eval);
            alpha = max(alpha, maxEval);   // Update alpha.
            if (beta <= alpha)
                break;      // Beta prunning.
        }
        return maxEval;
    }
    else {
        int minEval = INT_MAX;
        for (auto& move:getLegalMoves()) {
            makeMove(move);
            int eval = MiniMax(depth-1, alpha, beta, true);    // Now maximizing.
            undoMove(move);
            minEval = min(minEval, eval);
            beta = min(beta, minEval);     // Update beta
            if (beta <= alpha)
                break;      // Alpha prunning.
        }
        return minEval;
    }
}