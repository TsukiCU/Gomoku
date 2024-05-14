#ifndef TOUCHPAD_HH
#define TOUCHPAD_HH

#include "display.h"
#include <cstdint>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <thread>
#include "input.h"

#define TOUCHPAD_VENDOR_ID 0x28bd
#define TOUCHPAD_PRODUCT_ID 0x0928
#define TOUCHPAD_INTERFACE 1
#define TOUCHPAD_ENDPOINT 0x82
#define TOUCHPAD_SHOW_CURSOR(status) (((status)&0xf0)==0xa0)

#define TOUCHPAD_MOUSE_EVENT_COUNT 3
#define TOUCHPAD_MOUSE_LEFT 0
#define TOUCHPAD_MOUSE_RIGHT 1
#define TOUCHPAD_MOUSE_MID 2

struct libusb_device_handle;
struct XPPenMessage{
	// magic number 0x07
	unsigned char magic;
	// mouse status
	// 0xc0 - pen not detected
	// 0xa0 - pen detected
	// 0xa1 - left mouse down
	// 0xa2 - right mouse down
	// 0xa3 - left & right mouse down
	unsigned char status;
	
	// Coordinates, need to convert to display coordinate
	// Horizontal - (uint16_t)((horizontal)/32768.0*480)
	// Vertical - (uint16_t)((vertical)/51.2)

	// Horizontal, similar to hcount, 0~32767
	// 0 at the side that is close to the buttons
	uint16_t horizontal;
	// Vertical, similar to vcount, 0~32767
	uint16_t vertical;
	// pen pressure level
	uint16_t pressure;
	// currently unknown
	uint16_t unknown;
};

class Touchpad : public BaseInputDevice{
public:
	Touchpad()
	{
		device_name_ = "Touchpad";
		vendor_id_ = TOUCHPAD_VENDOR_ID;
		product_id_ = TOUCHPAD_PRODUCT_ID;
		interface_ = TOUCHPAD_INTERFACE;
		endpoint_ = TOUCHPAD_ENDPOINT;	
	}
	void print_touchpad_message(struct XPPenMessage msg);

	void create_handling_thread() override;

protected:
	void handle_message_func() override;
	void handle_touchpad_pen_event(struct XPPenMessage *msg);
};

#endif
