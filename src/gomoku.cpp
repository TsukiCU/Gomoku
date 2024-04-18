#include <iostream>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include "gomoku.h"
#include "../kmod/vga_gomoku.h"

bool Gomoku::on_board(int x, int y)
{
    return x >= 0 && x < board_size && y >= 0 && y < board_size;
}

bool Gomoku::valid_move(int x, int y)
{
    return on_board(x, y) && board[x][y] == 0;
}

int Gomoku::make_move(pair<int, int> move)
{
    int x = move.first;
    int y = move.second;

    if (!valid_move(x, y))  return 1;
    board[x][y] = current_player;
    if (check_win(x, y)) {
        winner = current_player;
        state = 1;
        return 0;
    }

	// vga_gomoku_arg_t arg;
	// arg.param[0] = 1;
	// arg.param[1] = (unsigned char)x;
	// arg.param[2] = (unsigned char)y;
	// arg.param[3] = (unsigned char)current_player;
	// if (ioctl(vga_gomoku_fd, VGA_GOMOKU_WRITE, &arg)){
	// 	perror("ioctl(VGA_GOMOKU_WRITE) failed");
	// 	return 0;
	// }
    switchPlayers();
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

void Gomoku::switchPlayers()
{
    current_player = 3 - current_player;
}

void Gomoku::displayBoard()
{
    // [n] -    -    -    -    O    X    -    -    -    -    -    -    -    -    -
    // n is row index, '-' refers to there's no stone on this intercection.
    // 'X' refers to there is a black stone, 'O' refers to white.

    string filler = "   ";

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

void Gomoku::clearBoard()
{
	board = vector<vector<int>>(board_size, std::vector<int>(board_size, 0));
}
