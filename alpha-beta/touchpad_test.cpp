#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/touchpad.h"

int main()
{
	Touchpad pad;
	int r=0, data_len;
	struct XPPenMessage data;

	if(!pad.open_touchpad_device()){
		printf("Open touchpad device failed.\n");
		return -1;
	}
	pad.create_touchpad_handling_thread();

	return 0;
}