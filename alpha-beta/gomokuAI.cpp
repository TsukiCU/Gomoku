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
    int di = 0;      // Start direction index, begin with moving right
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
        s.find(shapeTable.OTHREE_2) != string::npos ||
        s.find(shapeTable.OTHREE_3) != string::npos ||
        s.find(shapeTable.OTHREE_4) != string::npos)
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

    if (game->state == 1) {
        game->state = 0;
    }

    game->switchPlayers();
    return 0;
}

pair<int, int> GomokuAI::decideThirdMove()
{
    // The first move is fixed as (7, 7).
    pair<int, int> secondMove = game->record[1];

    // Out of touch
    if (secondMove.first >= 9 || secondMove.first <= 5 || secondMove.second >= 9 || secondMove.second <= 5) {
        vector<pair<int, int>> response = {make_pair(6, 8), make_pair(8, 6), make_pair(8, 8), make_pair(6, 6)};
        srand(time(NULL));
        return response[rand() % 4];
    }

    // Direct4
    else if (secondMove.first + secondMove.second == 15)
        return make_pair(8, 8);

    else if (secondMove.first + secondMove.second == 13)
        return make_pair(6, 6);

    else {
        int x = secondMove.first - 7;
        int y = secondMove.second - 7;
        vector<pair<int, int>> response = {make_pair(secondMove.first - 2*x, secondMove.second),
                                           make_pair(secondMove.first, secondMove.second - 2*y)};
        srand(time(NULL));
        return response[rand() % 2];
    }
}

/*
 * XXX: May need to revise the logic.
 */
pair<int, int> GomokuAI::isKagestu(pair<int, int> bestMove)
{
    if (game->record.size() != 3) {
        cerr << "Incorrectly entered function isKagestu. " << endl;
        return bestMove;
    }

    // 花月 e.g. (7,7) -> (7,8) -> (8,7) 
    pair<int, int> firstMove = game->record[0];
    pair<int, int> secondMove = game->record[1];
    pair<int, int> thirdMove = game->record[2];

    if (firstMove != make_pair(7, 7))
        return bestMove;
    
    else if ((secondMove == make_pair(7, 8) && thirdMove == make_pair(6, 8))||
    (secondMove == make_pair(8, 7) && thirdMove == make_pair(8, 6)))   { bestMove = make_pair(6, 6);}
    
    else if ((secondMove == make_pair(6, 7) && thirdMove == make_pair(6, 8))||
    (secondMove == make_pair(7, 6) && thirdMove == make_pair(8, 6)))   { bestMove = make_pair(8, 8);}

    else if ((secondMove == make_pair(8, 7) && thirdMove == make_pair(8, 8))||
    (secondMove == make_pair(7, 6) && thirdMove == make_pair(6, 6)))   { bestMove = make_pair(6, 8);}

    else if ((secondMove == make_pair(7, 8) && thirdMove == make_pair(8, 8))||
    (secondMove == make_pair(6, 7) && thirdMove == make_pair(6, 6)))   { bestMove = make_pair(8, 6);}

    return bestMove;
}

pair<int, int> GomokuAI::isUgetsu(pair<int, int> bestMove)
{
    if (game->record.size() != 3) {
        cerr << "Incorrectly entered function isUgetsu. " << endl;
        return bestMove;
    }

    // 花月 e.g. (7,7) -> (7,6) -> (6,7)
    pair<int, int> firstMove = game->record[0];
    pair<int, int> secondMove = game->record[1];
    pair<int, int> thirdMove = game->record[2];

    if (firstMove != make_pair(7, 7))
        return bestMove;

    else if ((secondMove == make_pair(7, 6) && thirdMove == make_pair(6, 7))||
    (secondMove == make_pair(7, 8) && thirdMove == make_pair(6, 7)))   { bestMove = make_pair(5, 7);}
    
    else if ((secondMove == make_pair(7, 8) && thirdMove == make_pair(8, 7))||
    (secondMove == make_pair(7, 6) && thirdMove == make_pair(8, 7)))   { bestMove = make_pair(9, 7);}

    else if ((secondMove == make_pair(6, 7) && thirdMove == make_pair(7, 6))||
    (secondMove == make_pair(8, 7) && thirdMove == make_pair(7, 6)))   { bestMove = make_pair(7, 5);}

    else if ((secondMove == make_pair(6, 7) && thirdMove == make_pair(7, 8))||
    (secondMove == make_pair(8, 7) && thirdMove == make_pair(7, 8)))   { bestMove = make_pair(7, 9);}

    return bestMove;
}

pair<int, int> GomokuAI::decideFourthMove()
{
    pair<int, int> bestMove = make_pair(-1, -1);

    // If the player keeps playing at the corners or edges, either
    // (s)he is stupid or does this on purpose.
    pair<int, int> firstMove = game->record[0];
    pair<int, int> thirdMove = game->record[2];
    if (firstMove != make_pair(7, 7) &&
        (thirdMove.first >= 9 || thirdMove.first <= 5 || thirdMove.second >= 9 || thirdMove.second <= 5))
    {
        vector<pair<int, int>> response = {make_pair(6, 8), make_pair(8, 6), make_pair(8, 8), make_pair(6, 6),
        make_pair(6, 7), make_pair(7, 6), make_pair(8, 7), make_pair(7, 8)};
        srand(time(NULL));

        while (true) {               
            pair<int, int> tmpMove = response[rand() % 8];
            if (game->valid_move(tmpMove.first, tmpMove.second)) {
                bestMove = tmpMove;
                break;
                // Impossible to reach isKagestu and isUgetsu so it's fine.
            }
        }
    }

    bestMove = isKagestu(bestMove);
    if (bestMove.first != -1)
        return bestMove;
    
    bestMove = isUgetsu(bestMove);
    if (bestMove.first != -1)
        return bestMove;

    return bestMove;
}

pair<int, int> GomokuAI::finishMove()
{
    pair<int, int> bestMove = make_pair(-1, -1);
    int size = game->record.size();
    if (size < 8)
        return bestMove;
    pair<int, int> scdlstMove = game->record[size-2];  // second to last

    // If AI has a Half four. Don't hesitate!
    int x = scdlstMove.first, y = scdlstMove.second, player = game->current_player;
    vector<string> s_list = {getStrFromPos<1, 0>(x, y, player), getStrFromPos<0, 1>(x, y, player),
    getStrFromPos<1, 1>(x, y, player), getStrFromPos<1, -1>(x, y, player)};

    for (auto s: s_list) {
        if (s.find(shapeTable.HFOUR_0) != string::npos ||
        s.find(shapeTable.HFOUR_1) != string::npos ||
        s.find(shapeTable.HFOUR_2) != string::npos ||
        s.find(shapeTable.HFOUR_3) != string::npos ||
        s.find(shapeTable.HFOUR_4) != string::npos ||
        s.find(shapeTable.OFOUR) != string::npos)
        // We are sure that one single move leads to success.
        {
            for (auto& move:getLegalMoves()) {
                int x = move.first, y = move.second;
                game->board[x][y] = game->current_player;  // temporarily set.
                if (game->check_win(x, y)) {
                    game->board[x][y] = 0;
                    return move;
                }
                game->board[x][y] = 0;
                // if (makeMove(move)) {
                //     cout << "Failed to make a move in finishMove()!" << endl;
                // }
                // if (game->state == 1) {
                //     undoMove(move); // This will also switch back the state.
                //     return move;
                // }
                // undoMove(move);
            }
        }
    }
    
    return bestMove;
}

pair<int, int> GomokuAI::findBestMove()
{
    /* HACK:FIXME: This is a temporary fix of this issue. Not a good idea. */
    int cur_player = game->current_player;
    int bestScore = INT_MIN;
    pair<int, int> bestMove = {-1, -1};

    // HACK:FIXME: Terrible idea. Fix this if I have time!!!!
    // If there is a [Half Four] in our (others') sructure. handle this immediately!!
    bestMove = finishMove();
    assert(cur_player == game->current_player);
    if (bestMove != make_pair(-1, -1)) {
        if (game->current_player != cur_player)
            game->switchPlayers(); /// FUUUUUUUCKKKKKKKK!
        //game->switchPlayers();  
        return bestMove;
    }

    // This is very important!!!
    assert(cur_player == game->current_player);

    /* Hard code for the first several moves. */
    // Hard code for the first move. The best move for the first move is always (7, 7)
    if (game->record.size() == 0)
        return make_pair(7, 7);

    // Hard code for the second move.
    if (game->record.size() == 1) {
        pair<int, int> firstMove = game->record.back();
        int x = firstMove.first, y = firstMove.second;
        if (x == 7 && y == 7) {
        // Several opening choices. Randomly choose one.
            vector<pair<int, int>> response = {make_pair(6, 8), make_pair(8, 6), make_pair(8, 8), make_pair(6, 6),
                                              make_pair(6, 7), make_pair(7, 6), make_pair(8, 7), make_pair(7, 8)};
            srand(time(NULL));
            return response[rand() % 8];
        }
        else
            return make_pair(7, 7);
    }

    // Hard code for the third move. This is deterministic so don't need to check validity.
    if (game->record.size() == 2) 
        return decideThirdMove();

    // Hard code for the fourth move.
    if (game->record.size() == 3) {
        pair<int, int> move = decideFourthMove();
        if (move.first != -1)
            return move;
    }

    for(auto& move:getLegalMoves()) {
        makeMove(move);
        int score = MiniMax(maxDepth, INT_MAX, INT_MIN, true);
        undoMove(move);
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    /* HACK:FIXME: As stated above. */
    if (game->current_player != cur_player)
        game->switchPlayers();

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