#include "display.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

#include "../kmod/vga_gomoku.h"

bool GMKDisplay::update_register(unsigned int index,uint16_t val)
{
	if(index>8)
		return false;
	params_[index]=val;
	return this->sync();
}

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
	params_[1] = x|(y<<4)|(current<<8)|(piece<<9);
	printf("piece_info3:0x%04x\n",params_[1]);
	return this->sync();
}

bool GMKDisplay::update_select(int x,int y)
{
	uint16_t reg=0;
	reg|=((x<<4)|y);
	params_[2] = reg;
	return this->sync();
}

bool GMKDisplay::clear_board()
{
	params_[0] = 0xffff;
	bool ret = this->sync();
	params_[0] = 0;
	return ret;
}

bool GMKDisplay::sync()
{
	if (ioctl(vga_gomoku_fd_, VGA_GOMOKU_WRITE, params_)){
		perror("ioctl(VGA_GOMOKU_WRITE) failed");
		return false;
	}
	return true;
}
