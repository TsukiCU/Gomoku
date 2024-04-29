#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <thread>
#include "touchpad.h"

int stop_touchpad_thread = 0;

void Touchpad::print_touchpad_message(struct XPPenMessage msg)
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

bool Touchpad::open_touchpad_device()
{
	libusb_context *ctx;
	int r;

    r = libusb_init(&ctx);
    if (r < 0) {
        fprintf(stderr, "fail to init libusb: %d\n", r);
        return false;
    }

	handle_ = libusb_open_device_with_vid_pid(ctx,TOUCHPAD_VENDOR_ID,TOUCHPAD_PRODUCT_ID);
	if(!handle_){
		perror("Open touchpad usb device");
		goto ERR;
	}

	// Use interface 0
	r = libusb_set_auto_detach_kernel_driver(handle_,1);
	if(r!=LIBUSB_SUCCESS){
		printf("Detach kernel driver failed.\n");
		goto ERR;
	}
	if (libusb_kernel_driver_active(handle_, TOUCHPAD_INTERFACE) == 1) {
		r = libusb_detach_kernel_driver(handle_, TOUCHPAD_INTERFACE);
		if (r == 0) {
			printf("Kernel Driver Detached\n");
		} else {
			fprintf(stderr, "Error detaching kernel driver: %d\n", r);
		}
	}
	r = libusb_claim_interface(handle_, TOUCHPAD_INTERFACE);
	if (r < 0) {
		fprintf(stderr, "claim interface error %d - %s\n", r, libusb_strerror(r));
		goto ERR;
	}
	printf("claimed interface %d\n",TOUCHPAD_INTERFACE);

	return true;

ERR:
	close_touchpad_device();
	return false;
}

void Touchpad::close_touchpad_device()
{
	libusb_close(handle_);
	libusb_exit(NULL);	
}

void Touchpad::handle_touchpad_message_func()
{
	int data_len = 0;
	int r = 0;
	XPPenMessage data;
	while(!stop_touchpad_thread){
		// Read data
		r = libusb_interrupt_transfer(handle_, TOUCHPAD_ENDPOINT, (unsigned char *)&data, sizeof(data), &data_len, 5000);
		switch (r) {
		case LIBUSB_ERROR_PIPE: printf("LIBUSB_ERROR_PIPE\n"); goto OUT;
		case LIBUSB_ERROR_OVERFLOW: printf("LIBUSB_ERROR_OVERFLOW\n"); goto OUT;
		case LIBUSB_ERROR_NO_DEVICE: printf("LIBUSB_ERROR_NO_DEVICE\n"); goto OUT;
		case LIBUSB_ERROR_BUSY: printf("LIBUSB_ERROR_BUSY\n"); goto OUT;
		case LIBUSB_ERROR_INVALID_PARAM: printf("LIBUSB_ERROR_INVALID_PARAM\n"); goto OUT;
		}
		
		// printf("\nData: length %d\n",data_len);
		// print_touchpad_message(data);
		// printf("\n");
		// TODO: Message handling
		if(display_){
			display_->update_touchpad_cursor(data.horizontal, data.vertical,TOUCHPAD_SHOW_CURSOR(data.status));
		}
	}
OUT:
	stop_touchpad_thread = 1;
	close_touchpad_device();
	return;
}

void Touchpad::create_touchpad_handling_thread()
{
	stop_touchpad_thread = 0;
	thread_ = std::thread(&Touchpad::handle_touchpad_message_func,this);
}

void stop_touchpad_handling_thread()
{
	stop_touchpad_thread = 1;
}