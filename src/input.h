#ifndef INPUT_H
#define INPUT_H

#include <cstdint>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <thread>
#include "display.h"

#define INPUT_WAIT_INTERVAL_US 10000

enum InputEventType : unsigned char{
	// 0x0000xxxx - Xbox Event
	XBOX_UP = 0,
	XBOX_DOWN = 1,
	XBOX_LEFT = 2,
	XBOX_RIGHT = 3,
	XBOX_START = 4,
	XBOX_BACK = 5,
	XBOX_LSTICK = 6,
	XBOX_RSTICK = 7,
	XBOX_LB = 8,
	XBOX_LT = 9,
	XBOX_MENU = 10,
	XBOX_A = 12,
	XBOX_B = 13,
	XBOX_X = 14,
	XBOX_Y = 15,
	// 0x0001xxxx - Pad event
	PAD_MOUSE_LEFT = 16,
	PAD_MOUSE_RIGHT = 17,
	PAD_MOUSE_MID = 18,
	NONE = 0xFF
};

struct InputEvent{
	InputEventType type;
	uint16_t vga_x,vga_y;
};

class InputEventHandler{
public:
	virtual void handle_input_press(InputEvent event){
		printf("%d:pressed\n",event.type);
		command_received_ = true;
	}
	virtual void handle_input_release(InputEvent event){
		printf("%d:released\n",event.type);
	}

	uint16_t wait_for_command();

protected:
	bool command_received_ = false;
	uint16_t command_type_ = 0;
};

class BaseInputDevice{
public:
	BaseInputDevice(){};
	~BaseInputDevice(){close_device();}
	bool open_device();
	void close_device();
	
	void print_touchpad_message(struct XPPenMessage msg);

	virtual void create_handling_thread() = 0;
	void stop_handling_thread();

	void set_display(GMKDisplay *display) {display_=display;}
	GMKDisplay *get_display() {return display_;}
	void set_input_handler(InputEventHandler *handler){input_handler_=handler;}

protected:
	virtual void handle_message_func() = 0;

	GMKDisplay *display_=NULL;
	libusb_device_handle *handle_=NULL;
	InputEventHandler *input_handler_=NULL;

	std::string device_name_ = "";
	uint16_t vendor_id_ = 0;
	uint16_t product_id_ = 0;
	int interface_ = 0;
	unsigned char endpoint_ = 0;

	std::thread thread_;
	int thread_stopped_ = 0;
	const int usb_timeout_ = 2000;
};

class Console: public BaseInputDevice{
public:
	void create_handling_thread();
protected:
	void handle_message_func();
};

#endif