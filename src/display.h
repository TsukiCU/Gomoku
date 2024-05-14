#ifndef _DISPLAY_HH
#define _DISPLAY_HH

#include <cstdint>
#include <stdint.h>
#include <string>
#include <sys/types.h>
#include <vector>
#define VGA_DRIVER_FILENAME "/dev/vga_ball"

struct BBOX{
	uint16_t left=0xffff,up=0xffff,right=0xffff,bottom=0xffff;
	BBOX expand_by(BBOX &bbox);
	BBOX(){};
	BBOX(uint16_t _left,uint16_t _up,uint16_t _right, uint16_t _bottom):
		left(_left), up(_up), right(_right), bottom(_bottom){}
	bool in(uint16_t x, uint16_t y);
	void reset();
};

struct GMKDisplayMessageInfo{
	std::string content="";
	uint16_t group=0;
	uint16_t group_idx=0;
	uint16_t index=0;
	uint16_t up=0xffff, down=0xffff, left=0xffff, right=0xffff;
	bool selectable=false;
	bool visible=false;
	BBOX bounding_box=BBOX(0xFFFF,0xFFFF,0xFFFF,0xFFFF);
};


class GMKDisplayMessageGroup{
public:
	uint16_t get_message_command(uint16_t index);
	uint16_t message_select_by_vga_xy(uint16_t vga_x,uint16_t vga_y);
	uint16_t next_message_by_direction(uint16_t current_message, int direction);
	uint16_t first_selectable_message();

	void update_message_selectable(uint16_t index, bool selectable, bool update_cache=false);
	void update_selectable_cache();
	void generate_visibility();
	void update_group_visibility(uint16_t group, bool visible);

	inline static bool is_board_selected(uint16_t message_index){return !(message_index&0x00ff);}

	std::vector<GMKDisplayMessageInfo> messages;
	std::vector<GMKDisplayMessageInfo> selectable;
	std::vector<uint16_t> group_visibility;

protected:
	uint16_t next_board_by_direction(uint16_t board_message,int direction);

	BBOX selected_area_;
};


class GMKDisplay{
	public:
		GMKDisplay()
			{};
		// piece 0:No piece, 1:white piece, 2:black piece
		bool update_piece_info(int x,int y,int piece, int current=1, bool sync=true);
		bool update_select(int board_x,int board_y, bool sync=true);
		bool update_select(uint16_t message_index, bool sync=true);
		bool update_register(unsigned int index,uint16_t val, bool sync=true);
		uint16_t get_register(uint16_t index){return params_[index];}
		bool update_touchpad_cursor(uint16_t vga_x, uint16_t vga_y, bool visible=true, bool sync=true);
		bool show_menu();
		bool show_board(bool clear);
		bool clear_board();
		bool sync();
		bool open_display();

		void set_message_group(GMKDisplayMessageGroup *group){msg_group_ = group;}
		bool update_message_visibility(uint16_t index, bool visible, bool sync=true);
		bool update_group_visibility(uint16_t group, bool visible, bool sync=true);

	protected:

		uint16_t params_[8];
		int vga_gomoku_fd_=-1;
		const char *dev_name_=VGA_DRIVER_FILENAME;

		GMKDisplayMessageGroup *msg_group_;
};

#endif
