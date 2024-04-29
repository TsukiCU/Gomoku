#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/touchpad.h"

int main()
{
	libusb_device_handle *handle;
	int r=0, data_len;
	struct XPPenMessage data;

	handle = open_touchpad_device();
	if(!handle){
		printf("Open touchpad device failed.\n");
		return -1;
	}

	printf("Read data from interface %d endpoint %02x\n",TOUCHPAD_INTERFACE,TOUCHPAD_ENDPOINT);
	while(1){
		// Read data
		r = libusb_interrupt_transfer(handle, TOUCHPAD_ENDPOINT, (unsigned char *)&data, sizeof(data), &data_len, 5000);
		switch (r) {
		case LIBUSB_ERROR_PIPE: printf("LIBUSB_ERROR_PIPE\n"); goto OUT;
		case LIBUSB_ERROR_OVERFLOW: printf("LIBUSB_ERROR_OVERFLOW\n"); goto OUT;
		case LIBUSB_ERROR_NO_DEVICE: printf("LIBUSB_ERROR_NO_DEVICE\n"); goto OUT;
		case LIBUSB_ERROR_BUSY: printf("LIBUSB_ERROR_BUSY\n"); goto OUT;
		case LIBUSB_ERROR_INVALID_PARAM: printf("LIBUSB_ERROR_INVALID_PARAM\n"); goto OUT;
		}
		
		printf("\nData: length %d\n",data_len);
		print_touchpad_message(data);
		printf("\n");
	}

OUT:
	close_touchpad_device(handle);
	return (r>=0) ? r : -r;
}