#include "display.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

#include "../vga_kmod/vga_gomoku.h"

bool BBOX::in(uint16_t x, uint16_t y)
{
	//printf("left %u, x %u, right %u, up %u, y %u, bottom %u\n",left,x,right,up,y,bottom);
	return (left<=x && x<=right && up<=y && y<= bottom);
}
void BBOX::reset()
{
	left=0xffff;
}

BBOX BBOX::expand_by(BBOX &bbox)
{
	BBOX ret;
	if(left==0xffff){
		ret = bbox;
		return ret;
	}
	ret.left = bbox.left<left?bbox.left:left;
	ret.right = bbox.right>right?bbox.right:right;
	ret.up = bbox.up<up?bbox.up:up;
	ret.bottom = bbox.bottom>bottom?bbox.bottom:bottom;
	return ret;
}

uint16_t GMKDisplayMessageGroup::message_select_by_vga_xy(uint16_t vga_x, uint16_t vga_y)
{
	if(messages[0].visible && !messages[0].disabled && vga_x>=4 && vga_x<499 && vga_y >=7 && vga_y<472){
		uint16_t piece_x, piece_y;
		piece_x=(vga_x-4)/33;
		piece_y=(vga_y-7)/31;
		return (piece_x<<12)|(piece_y<<8);
	}
	if(!selected_area_.in(vga_x, vga_y))
		return 0xFFFF;
	for(auto msg:selectable){
		if(msg.bounding_box.in(vga_x, vga_y))
			return msg.index|0xFF00;
	}
	return 0xFFFF;
}

void GMKDisplayMessageGroup::update_selectable_cache()
{
	selectable.clear();
	selected_area_.reset();
	for(GMKDisplayMessageInfo &msg: messages){
		if(msg.selectable && msg.visible && !msg.disabled){
			selectable.push_back(msg);
			selected_area_ = selected_area_.expand_by(msg.bounding_box);
			printf("%s selectable\n",msg.content.c_str());
		}
	}
}

uint16_t GMKDisplayMessageGroup::first_selectable_message()
{
	update_selectable_cache();
	if(selectable.empty())
		return 0xffff;
	return selectable[0].index;
}

uint16_t GMKDisplayMessageGroup::next_board_by_direction(uint16_t board_message,int direction)
{
	uint16_t x = board_message>>12;
	uint16_t y = (board_message>>8)&0xf;
	if(direction==0){ // UP
		if(y>0)
			--y;
	} 
	else if(direction==1){ // DOWN
		if(y<14)
			++y;
	}
	else if(direction==2){ // LEFT
		if(x>=0)
			--x;
	}
	else if(direction==3){ // RIGHT
		if(x==14)
			return messages[0].right;
		++x;
	}
	return (x<<12)|(y<<8);
}

uint16_t GMKDisplayMessageGroup::next_message_by_direction(uint16_t current_message, int direction)
{
	uint16_t next = current_message;
	if(is_board_selected(current_message))
		return next_board_by_direction(current_message, direction);
	if(direction==0){ // UP
		// Iterate next message
		next = messages[current_message].up;
		while(next!=0xffff){ // 0xffff - no next message
			if(is_board_selected(next)){
				if(messages[0].selectable && messages[0].visible && !messages[0].disabled)
					// Return Piece board
					return next;
				return current_message;
			}
			if(messages[next].selectable && messages[next].visible && !messages[next].disabled)
				// Return Selectable message
				return next;
			next = messages[current_message].up;
		}
	}
	else if(direction==1){ // DOWN
		// Iterate next message
		next = messages[current_message].down;
		while(next!=0xffff){ // 0xffff - no next message
			if(is_board_selected(next)){
				if(messages[0].selectable && messages[0].visible && !messages[0].disabled)
					// Return Piece board
					return next;
				return current_message;
			}
			if(messages[next].selectable && messages[next].visible && !messages[next].disabled)
				// Return Selectable message
				return next;
			next = messages[current_message].down;
		}
	}
	else if(direction==2){ // LEFT
		// Iterate next message
		next = messages[current_message].left;
		while(next!=0xffff){ // 0xffff - no next message
			if(is_board_selected(next)){
				if(messages[0].selectable && messages[0].visible && !messages[0].disabled)
					// Return Piece board
					return next;
				return current_message;
			}
			if(messages[next].selectable && messages[next].visible && !messages[next].disabled)
				// Return Selectable message
				return next;
			next = messages[current_message].left;
		}
	}
	else if(direction==3){ // RIGHT
		// Iterate next message
		next = messages[current_message].right;
		while(next!=0xffff){ // 0xffff - no next message
			if(is_board_selected(next)){
				if(messages[0].selectable && messages[0].visible && !messages[0].disabled)
					// Return Piece board
					return next;
				return current_message;
			}
			if(messages[next].selectable && messages[next].visible && !messages[next].disabled)
				// Return Selectable message
				return next;
			next = messages[current_message].right;
		}	
	}
	// No selectable message, return current
	return current_message;
}

uint16_t GMKDisplayMessageGroup::get_message_command(uint16_t index)
{
	if(is_board_selected(index)) // Selected a board position
		// Return make move command
		return 0;
	return index & 0x00FF;
}

void GMKDisplayMessageGroup::update_message_selectable(uint16_t index, bool selectable, bool update)
{
	messages[index].selectable = selectable;
	if(update)
		update_selectable_cache();
}

void GMKDisplayMessageGroup::generate_visibility()
{
	int group=0;
	group_visibility.push_back(0);
	for(auto msg:messages){
		if(msg.group!=group){
			group_visibility.push_back(msg.group<<10);
			group = msg.group;
		}
		group_visibility[group]|=(msg.visible<<msg.group_idx);
	}
}

void GMKDisplayMessageGroup::update_group_visibility(uint16_t group, bool visible)
{
	if(visible){
		group_visibility[group] |= 0x03ff;
		for(GMKDisplayMessageInfo &msg:messages){
			if(msg.group==group)
				msg.visible = true;
		}
	}
	else{
		group_visibility[group] &= ~(0x03ff);
		for(GMKDisplayMessageInfo &msg:messages){
			if(msg.group==group)
				msg.visible = false;
		}
	}
	update_selectable_cache();
}

void GMKDisplayMessageGroup::display_selectable()
{
	printf("Selectable\n");
	for(auto msg:messages){
		printf("%d:%s\n",msg.index,msg.content.c_str());
	}
	printf("\n");
}

bool GMKDisplay::update_register(unsigned int index,uint16_t val, bool sync)
{
	if(index>8)
		return false;
	params_[index]=val;
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_p2_profile(int profile, bool sync)
{
	if(profile==0){	// AI Profile
		printf("AI profile\n");
		update_group_visibility(3,(uint16_t)0b101,false);
		params_[0]&=~(1<<4);
	}else if(profile==1){ // P2 Profile
		printf("P2 profile\n");
		update_group_visibility(3,(uint16_t)0b011,false);
		params_[0]|=(1<<4);
	}
	else if(profile==2){  // No player
		printf("No Player\n");
		update_group_visibility(3,(uint16_t)0b001,false);
		params_[0]|=(1<<4);
	}
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_message_visibility(uint16_t index, bool visible, bool sync)
{
	GMKDisplayMessageInfo &msg = msg_group_->messages[index];
	msg.visible = visible;
	uint16_t group_idx = msg.group_idx;
	uint16_t &param = msg_group_->group_visibility[msg.group];
	
	if(visible)
		param |= (1<<group_idx);
	else
		param &= ~(1<<group_idx);

	params_[3] = param;
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_group_visibility(uint16_t group, uint16_t val, bool sync)
{
	bool visible[10];
	for(size_t i=0;i<10;++i)
		visible[i]=(val>>i)&1;
	val = (val & 0x03FF) | (group << 10);
	msg_group_->group_visibility[group] = val;
	for(GMKDisplayMessageInfo &msg:msg_group_->messages){
		if(msg.group==group)
			msg.visible=visible[msg.group_idx];
	}

	params_[3] = val;
	printf("%s by val:%04x\n",__func__,params_[3]);
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_group_visibility(uint16_t group, bool visible, bool sync)
{
	msg_group_->update_group_visibility(group, visible);

	params_[3] = msg_group_->group_visibility[group];
	printf("Visibility\n");
	for(size_t i=0;i<msg_group_->group_visibility.size();++i){
		printf("0x%04x ",msg_group_->group_visibility[i]);
	}
	printf("\n\n");
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_touchpad_cursor(uint16_t vga_x, uint16_t vga_y,bool visible, bool sync)
{
	//printf("update touchpad cursor: x %u, y %u, visible %d\n",x,y,visible);
	params_[0] = ((params_[0] & 0xFFFD) | (visible<<1));
	//printf("param0 0x%04x\n",params_[0]);
	params_[5] = vga_x;
	params_[6] = vga_y;

	// If cursor visible, update select
	if(visible){
		uint16_t index = msg_group_->message_select_by_vga_xy(vga_x, vga_y);
		update_select(index,false);
	}

	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::show_menu()
{
	play_sound(2);
	// Dialog set to 1
	params_[0]|=1;
	update_group_visibility(0, false);
	update_group_visibility(1, true);
	update_group_visibility(2, false);
	update_group_visibility(3, false);
	update_group_visibility(4, false);
	return update_group_visibility(5, false);
}
bool GMKDisplay::show_board(bool clear)
{
	bool ret=true;
	// Dialog set to 0
	params_[0]&=(~1);
	update_group_visibility(0, true);
	update_group_visibility(1, false);
	update_group_visibility(2, true);
	update_group_visibility(3, true);
	update_group_visibility(4, false);
	update_group_visibility(5, false);
	// Select board center
	ret = update_select(7, 7,false);
	if(clear)
		return clear_board();
	return ret;
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

bool GMKDisplay::update_piece_info(int x,int y, int piece, int current, bool sync)
{
	if(hint_.first!=15&&hint_.second!=15){
		// Hide hint
		params_[1] = hint_.second|(hint_.first<<4);
		hint_.first=15;
		hint_.second=15;
		this->sync();
		return update_piece_info(x, y, piece,sync);
	}
	if(piece==3) // Highlighted mark
		params_[1] = y|(x<<4)|(1<<8);
	else
		params_[1] = y|(x<<4)|(piece<<9);
	if(current)
		params_[2] = (params_[2]&0xff00)|(y|(x<<4));
	printf("piece_info3:0x%04x\n",params_[1]);
	printf("piece_info_params_[2]:%04x\n",params_[2]);
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::update_select(int x,int y, bool sync)
{
	// Update board selection
	params_[2] = ((x<<4)|y)<<8|(params_[2]&0x00ff);
	printf("select_params_[2]:%04x\n",params_[2]);
	// Unselect message
	params_[4] = 0;
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::clear_board()
{
	// Reset board command
	params_[0] = 0xffff;
	// Piece Info register
	params_[1] = 0;
	// Selected register
	params_[2] = 0xFFFF;
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

bool GMKDisplay::update_select(uint16_t message_index, bool sync)
{
	if(message_index&0xFF00)
		// Update board selection
		params_[2] = (message_index&0xff00)|(params_[2]&0x00ff);
	else
		params_[2] = (0xff00)|(params_[2]&0x00ff);
	// Update message selection
	params_[4] = message_index&0x00ff;
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::set_player_piece(bool top_black,bool sync)
{
	
	params_[0] = ((params_[0] & ~(top_black<<2)) | (top_black<<2));
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::set_turn_mark(bool top_turn,bool sync)
{
	params_[0] = ((params_[0] & ~(top_turn<<3)) | (top_turn<<3));
	if(sync)
		return this->sync();
	return true;
}

bool GMKDisplay::switch_turn_mark(bool sync)
{
	params_[0] ^= (1 << 3);
	if(sync)
		return this->sync();
	return true;	
}

// 0 - YOU WIN
// 1 - YOU LOSE
// 2 - P1 WIN
// 3 - P2 WIN
void GMKDisplay::show_game_result(int result, bool show)
{
	if(!show){
		for(GMKDisplayMessageInfo &msg:msg_group_->messages){
			msg.disabled=false;
		}
		msg_group_->update_selectable_cache();
		update_register(0, get_register(0)&~(1<<5));
		return;
	}
	update_group_visibility(4,(uint16_t)(1<<result));
	update_group_visibility(5,(uint16_t)(1));
	for(GMKDisplayMessageInfo &msg:msg_group_->messages){
		msg.disabled=true;
	}
	msg_group_->messages[20].disabled=false;
	GMKDisplayMessageInfo &msg = msg_group_->messages[20];
	printf("%s:selectable %d, visible %d, disabled %d\n",msg.content.c_str(),msg.selectable,msg.visible,msg.disabled);
	msg_group_->update_selectable_cache();
	update_register(0, get_register(0)|(1<<5));
	if(result==1)
		play_sound(1);
	else
	 	play_sound(0);
}

void GMKDisplay::show_confirm_message(bool show)
{
	if(!show){
		for(GMKDisplayMessageInfo &msg:msg_group_->messages){
			msg.disabled=false;
		}
		msg_group_->update_selectable_cache();
		update_register(0, get_register(0)&~(1<<5));
		return ;
	}
	update_group_visibility(4,(uint16_t)(1<<4));
	update_group_visibility(5,(uint16_t)(0b110));
	for(GMKDisplayMessageInfo &msg:msg_group_->messages){
		msg.disabled=true;
	}
	msg_group_->messages[21].disabled=false;
	msg_group_->messages[22].disabled=false;
	GMKDisplayMessageInfo &msg = msg_group_->messages[20];
	printf("%s:selectable %d, visible %d, disabled %d\n",msg.content.c_str(),msg.selectable,msg.visible,msg.disabled);
	msg_group_->update_selectable_cache();
	update_register(0, get_register(0)|(1<<5));
}

void GMKDisplay::show_scanning_message(bool show)
{
	if(!show){
		for(GMKDisplayMessageInfo &msg:msg_group_->messages){
			msg.disabled=false;
		}
		msg_group_->update_selectable_cache();
		update_register(0, get_register(0)&~(1<<5));
		return;
	}
	update_group_visibility(4,(uint16_t)(1<<5));
	update_group_visibility(5,(uint16_t)(0b000));
	for(GMKDisplayMessageInfo &msg:msg_group_->messages){
		msg.disabled=true;
	}
	// EXIT
	msg_group_->messages[20].disabled=false;
	msg_group_->update_selectable_cache();
	// Enable message box
	update_register(0, get_register(0)|(1<<5));
}

void GMKDisplay::show_hint(int x_index,int y_index)
{
	hint_.first = x_index;
	hint_.second = y_index;
	params_[1] = y_index|(x_index<<4)|(1<<8);
	this->sync();
}

void GMKDisplay::play_sound(int index)
{
	params_[0] &= 0x00FF;
	this->sync();
	params_[0] |= (1<<(index+8));
	this->sync();
	usleep(50000);
	params_[0] &= 0x00FF;
	this->sync();
}