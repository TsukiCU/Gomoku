#include "display.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

bool GMKDisplay::open_display()
{
	for(int i=0;i<8;++i)
		params_[i]=0;
	if ((vga_gomoku_fd_= open(this->dev_name_, O_RDWR)) == -1) {
		fprintf(stderr, "could not open %s\n", this->dev_name_);
		return false;
	}
	return true;
}

bool GMKDisplay::update_piece_info(int x,int y, int piece, int current)
{
	uint16_t reg=0;
	reg|=piece;
	reg>>=1;
	reg|=current;
	reg>>=1;
	reg|=x;
	reg|=(y<<4);
	params_[1] = reg;
	return this->sync();
}

bool GMKDisplay::update_select(int x,int y)
{
	uint16_t reg=0;
	reg|=(x|(y<<4));
	params_[2] = reg;
	return this->sync();
}

bool GMKDisplay::clear_board()
{
	params_[0] = 0;
	return this->sync();
}

bool GMKDisplay::sync()
{
	if (ioctl(vga_gomoku_fd_, VGA_GOMOKU_WRITE, &arg_)){
		perror("ioctl(VGA_GOMOKU_WRITE) failed");
		return false;
	}
	return true;
}