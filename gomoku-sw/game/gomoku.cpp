#include <iostream>
#include <fstream>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "gomoku.h"

bool Gomoku::on_board(int x, int y)
{
    return x >= 0 && x < board_size && y >= 0 && y < board_size;
}

bool Gomoku::valid_move(int x, int y)
{
	/* Game not ongoing. */
	if(state==1)
		return false;
    return on_board(x, y) && board[x][y] == 0;
}

void Gomoku::end_game(bool black_win)
{
	winner = black_win?1:2;
	state = 1;
	return;
}

int Gomoku::make_move(pair<int, int> move,bool fake)
{
    int x = move.first;
    int y = move.second;

    if (!valid_move(x, y))  return 1;
    board[x][y] = current_player;
	if(display && !fake){
		// Piece value is different from current_player
		display->update_piece_info(x, y, current_player);
		display->play_sound(3);
	}
	if(record_game && !fake)
		record.push_back(make_pair(x, y));
    if (check_win(x, y)) {
		end_game(current_player==1);
        return 0;
    }

    switchPlayers(fake);
    return 0;
}

/*
 * [WIN_LENGTH] of stones in a row.
 * Can be horizontally, vertically, or diagonally.
 */
bool Gomoku::check_win(int x, int y)
{
    // check horizontally
    if (check<1, 0>(x, y))
        return true;

    // check vertically
    if (check<0, 1>(x, y))
        return true;

    // check diagonally
    if (check<1, 1>(x, y) || check<-1, 1>(x, y))
        return true;

    return false;
}

bool Gomoku::is_draw()
{
    if (state == 1) {
        // Shouldn't reach here.
        cout << "The winning condition is satisfied already." << endl;
        return false;
    }

    for (int i=0; i<board_size; i++)
        for (int j=0; j<board_size; j++)
            if (board[i][j] == 0) return false;

    return true;
}

void Gomoku::switchPlayers(bool fake)
{
    current_player = 3 - current_player;
	if(display && !fake)
		display->switch_turn_mark();
}

void Gomoku::displayBoard()
{
    // [n] -    -    -    -    O    X    -    -    -    -    -    -    -    -    -
    // n is row index, '-' refers to there's no stone on this intercection.
    // 'X' refers to there is a black stone, 'O' refers to white.
    // '@' refers to the current move made by any player.

    string filler = "   ";
    pair<int, int> lastMove = make_pair(-1, -1);
    if (record.size() > 0)
        lastMove = record.back();

    cout << '\n' << filler << "  ";
    for (int i = 0; i < board_size; i++) {
        if (i < 9)
            cout << i + 1 << filler;
        else
            cout << i + 1 << "  ";
    }
    cout << '\n';

    for (int i = 0; i < board_size; i++) {
        if (i < 9)
            cout << ' ' << i + 1 << filler;
        else
            cout << i + 1 << filler;

        for (int j = 0; j < board_size; j++) {
            if (i == lastMove.first && j == lastMove.second)
                cout << '@' << filler;
            else
            switch (board[i][j]) {
                case 0: {cout << '-' << filler; break;}
                case 1: {cout << 'X' << filler; break;}
                case 2: {cout << 'O' << filler; break;}
            }
        }
        cout << "\n\n";
    }
    // cout << endl;
	// if(vga_gomoku_fd!=-1)
	// 	return;
	// const char *filename = VGA_DRIVER_FILENAME;
	// if ((vga_gomoku_fd = open(filename, O_RDWR)) == -1) {
	// 	fprintf(stderr, "could not open %s\n", filename);
	// 	exit(-1);
	// }
}

void Gomoku::resetGame()
{
    this->board = vector<vector<int>>(board_size, vector<int>(board_size, 0));
    this->record.erase(record.begin(), record.end());
    this->state = 0;
    this->current_player = 1;
    this->winner = 0;
    this->regretTimes = 0;
	this->winArray.clear();
	if(display){
		display->show_confirm_message(false);
		display->clear_board();
	}
}

int Gomoku::regret_move()
{
    // PvP
    if (mode == 0) {
        if (record.size() == 0) {
            cerr << "Cannot regret a move. " << endl;
            return 1;
        }
        auto rit = record.rbegin();
        pair<int, int> lastMove = *rit;
        board[lastMove.first][lastMove.second] = 0;

        // clear the record.
        if(record_game){
       		record.erase(record.end());
		}
		
		if(display){
			display->update_piece_info(lastMove.first, lastMove.second, 0);
			if(record.size()){ // Set last mark
				const pair<int, int> &lastMove = *(record.rbegin());
				display->update_piece_info(lastMove.first,lastMove.second,board[lastMove.first][lastMove.second]);
			}
			else { // No more piece on board, hide last mark
				printf("No piece Params[2]_:%04x\n",display->get_register(2)|0x00ff );
				display->update_register(2, display->get_register(2)|0x00ff );
			}
		}

		switchPlayers();
    }

    // PvE
    else {
        if (record.size() < 2) {
            cerr << "Cannot regret a move. " << endl;
            return 1;
        }
        auto rit = record.rbegin();
        pair<int, int> lastMove = *rit;
        pair<int, int> secondLastMove = *(rit + 1);
        board[lastMove.first][lastMove.second] = 0;
        board[secondLastMove.first][secondLastMove.second] = 0;

        // clear the record.
		if(record_game)
        	record.erase(record.end()-2, record.end());
        regretTimes++;
		
		if(display){
			display->update_piece_info(lastMove.first, lastMove.second, 0);
			display->update_piece_info(secondLastMove.first, secondLastMove.second, 0);
			if(record.size()){ // Set last mark
				const pair<int, int> &lastMove = *(record.rbegin());
				display->update_piece_info(lastMove.first,lastMove.second,board[lastMove.first][lastMove.second]);
			}
			else { // No more piece on board, hide last mark
				printf("Params[2]_:%04x\n",display->get_register(2)|0x00ff );
				display->update_register(2, display->get_register(2)|0x00ff );
			}
		}

        if (regretTimes >= 3) {
            cout << "Think before you make a move!" << endl;
        }

        /* TODO: We might disable regret function if player regrets too often. */
    }

    return 0;
}

void Gomoku::recordGame()
{
    string filename = "record.txt";
    ofstream file(filename, ios::out | ios::app);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Record game into the record.txt (append mode)
    file << "========================== New Game ==========================";
    file << "\n\n";

    for (auto move:record) {
        int first = move.first;
        int second = move.second;

        file << first;
        file << ",";
        file << second;
        file << "  ";
    }

    file.close();

    // Should it fail
    if (file.fail()) {
        cerr << "Error while closing record.tx." << endl;
    } else {
        cout << "Written to record.txt." << endl;
    }
}