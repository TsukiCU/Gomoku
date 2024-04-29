#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "touchpad.h"

void print_touchpad_message(struct XPPenMessage msg)
{
	const char *touchpad_status_str[]={"NO PEN","PEN","LEFT","RIGHT","LEFT&RIGHT","UNKNOWN"};
	int status;
	switch (msg.status) {
	case 0xc0: status=0;break;
	case 0xa0: status=1;break;
	case 0xa1: status=2;break;
	case 0xa2: status=3;break;
	case 0xa3: status=4;break;
	default: status=5;break;
	}
	printf("Info:\n");
	printf("Magic:0x%02x\n",msg.magic);
	printf("Status:0x%02x %s\n",msg.status,touchpad_status_str[status]);
	printf("Coordinate:%u,%u\n",(uint16_t)((32767-msg.vertical)/51.2),(uint16_t)((32767-msg.horizontal)/32768.0*480));
	printf("Pressure:%u\n",msg.pressure);
	printf("Unknown:%u\n",msg.unknown);
}

libusb_device_handle *open_touchpad_device()
{
	libusb_context *ctx;
	int r;
	libusb_device_handle *handle;

    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "fail to init libusb: %d\n", r);
        return NULL;
    }

	handle = libusb_open_device_with_vid_pid(ctx,TOUCHPAD_VENDOR_ID,TOUCHPAD_PRODUCT_ID);
	if(!handle){
		perror("Open touchpad usb device");
		goto ERR;
	}

	// Use interface 0
	r = libusb_set_auto_detach_kernel_driver(handle,1);
	if(r!=LIBUSB_SUCCESS){
		printf("Detach kernel driver failed.\n");
		goto ERR;
	}
	if (libusb_kernel_driver_active(handle, TOUCHPAD_INTERFACE) == 1) {
		r = libusb_detach_kernel_driver(handle, TOUCHPAD_INTERFACE);
		if (r == 0) {
			printf("Kernel Driver Detached\n");
		} else {
			fprintf(stderr, "Error detaching kernel driver: %d\n", r);
		}
	}
	r = libusb_claim_interface(handle, TOUCHPAD_INTERFACE);
	if (r < 0) {
		fprintf(stderr, "claim interface error %d - %s\n", r, libusb_strerror(r));
		goto ERR;
	}
	printf("claimed interface %d\n",TOUCHPAD_INTERFACE);

	return handle;

ERR:
	libusb_close(handle);
	libusb_exit(NULL);
	return NULL;
}

void close_touchpad_device(libusb_device_handle *handle)
{
	libusb_close(handle);
	libusb_exit(NULL);	
}