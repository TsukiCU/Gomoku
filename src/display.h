#ifndef _DISPLAY_HH
#define _DISPLAY_HH

#include <stdint.h>

class GMKDisplay{
	public:
		GMKDisplay(const char *dev_name):dev_name_(dev_name)
			{};
		// piece 0:No piece, 1:black piece, 2:white piece
		bool update_piece_info(int x,int y,int piece, int current=1);
		bool update_select(int x,int y);
		bool update_register(unsigned int index,uint16_t val);
		bool clear_board();
		bool sync();
		bool open_display();
	protected:
		uint16_t params_[8];
		int vga_gomoku_fd_=-1;
		const char *dev_name_;
};

#endif
