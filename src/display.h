#ifndef _DISPLAY_HH
#define _DISPLAY_HH

#include <cstdint>
#include <stdint.h>

class GMKDisplay{
	public:
		GMKDisplay(const char *dev_name):dev_name_(dev_name)
			{};
		// piece 0:No piece, 1:white piece, 2:black piece
		bool update_piece_info(int x,int y,int piece, int current=1);
		bool update_select(int x,int y);
		bool update_register(unsigned int index,uint16_t val);
		bool select_message(int index);
		bool update_message_visibility(int index, bool visible);
		bool update_message_visibility(uint16_t val);
		bool update_touchpad_cursor(uint16_t x, uint16_t y, bool visible=true);
		bool show_menu();
		bool show_board(bool clear);
		bool clear_board();
		bool sync();
		bool open_display();
	protected:
		uint16_t params_[8];
		int vga_gomoku_fd_=-1;
		const char *dev_name_;
};

#endif
