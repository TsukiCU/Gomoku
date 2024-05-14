// xboxcont.h
#ifndef XBOXCONT_H
#define XBOXCONT_H

#include <cstdint>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "display.h"
#include "input.h"

#define CONTROLLER_VENDOR_ID 0x045e
#define CONTROLLER_PRODUCT_ID 0x028e
#define CONTROLLER_ENDPOINT 0x81

struct XboxMessage{
	uint16_t header;
	uint16_t buttons;
	unsigned char lt_level;
	unsigned char rt_level;
	int16_t lstick_dir_right;
	int16_t lstick_dir_up;
	int16_t rstick_dir_right;
	int16_t rstick_dir_up;
	unsigned char unknown[9];
};

class XboxController: public BaseInputDevice{
public:
	XboxController()
	{
		device_name_ = "Xbox controller";
		vendor_id_ = CONTROLLER_VENDOR_ID;
		product_id_ = CONTROLLER_PRODUCT_ID;
		interface_ = 0;
		endpoint_ = CONTROLLER_ENDPOINT;		
	}
	void create_handling_thread() override;
	void print_xbox_message(XboxMessage msg);

protected:
	void handle_message_func() override;
	void handle_xbox_button_event(uint16_t status);
};

#endif // XBOXCONT_H