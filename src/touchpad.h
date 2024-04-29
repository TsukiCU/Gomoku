#ifndef TOUCHPAD_HH
#define TOUCHPAD_HH

#include <stdint.h>
#define TOUCHPAD_VENDOR_ID 0x28bd
#define TOUCHPAD_PRODUCT_ID 0x0928
#define TOUCHPAD_INTERFACE 1
#define TOUCHPAD_ENDPOINT 0x82

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
	// Horizontal - (uint16_t)((32767-vertical)/51.2)
	// Vertical - (uint16_t)((32767-msg.horizontal)/32768.0*480)

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

libusb_device_handle *open_touchpad_device();
void close_touchpad_device(libusb_device_handle *handle);
void print_touchpad_message(struct XPPenMessage msg);

#endif