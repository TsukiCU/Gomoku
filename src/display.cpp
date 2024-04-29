#include "display.h"
#include <cstdint>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

#include "../kmod/vga_gomoku.h"
#include "tcp.h"

bool GMKDisplay::update_register(unsigned int index,uint16_t val)
{
	if(index>8)
		return false;
	params_[index]=val;
	return this->sync();
}

bool GMKDisplay::update_message_visibility(int index, bool visible)
{
	if(index)
		params_[3] = (params_[3] & (~(1<<index)));
	else
		params_[3] = (params_[3] | (1<<index));
	return this->sync();
}
bool GMKDisplay::update_message_visibility(uint16_t val)
{
	params_[3] = val;
	return this->sync();
}
bool GMKDisplay::update_touchpad_cursor(uint16_t x, uint16_t y,bool visible)
{
	printf("update touchpad cursor: x %u, y %u, visible %d\n",x,y,visible);
	params_[0] = ((params_[0] & 0xFD) | (visible<<1));
	printf("param0 0x%02x\n",params_[0]);
	params_[4] = x;
	params_[5] = y;
	return this->sync();
}

bool GMKDisplay::show_menu()
{
	params_[0]=1;
	params_[4]=(1<<2);
	return update_message_visibility(0b111110);
}
bool GMKDisplay::show_board(bool clear)
{
	bool ret;
	params_[0]=0;
	params_[4]=(1<<6);
	ret = update_message_visibility(0b111000000);
	if(clear)
		return clear_board();
	return ret;
}

bool GMKDisplay::select_message(int index)
{
	params_[4]=(1<<index);
	return this->sync();
}

bool GMKDisplay::open_display()
{
	for(int i=0;i<8;++i)
		params_[i]=0;
	params_[2]=0x77ff;
	if ((vga_gomoku_fd_= open(this->dev_name_, O_RDWR)) == -1) {
		fprintf(stderr, "could not open %s\n", this->dev_name_);
		return false;
	}
	return true;
}

bool GMKDisplay::update_piece_info(int x,int y, int piece, int current)
{
	params_[1] = y|(x<<4)|(piece<<9);
	if(current)
		params_[2] = (params_[2]&0xff00)|(y|(x<<4));
	printf("piece_info3:0x%04x\n",params_[1]);
	printf("piece_info_params_[2]:%04x\n",params_[2]);
	return this->sync();
}

bool GMKDisplay::update_select(int x,int y)
{
	params_[2] = ((x<<4)|y)<<8|(params_[2]&0x00ff);
	printf("select_params_[2]:%04x\n",params_[2]);
	return this->sync();
}

bool GMKDisplay::clear_board()
{
	params_[0] = 0xffff;
	bool ret = this->sync();
	params_[0] = 0x0000;
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
