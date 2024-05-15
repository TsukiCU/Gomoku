#include <cstdint>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <time.h> 
#include <unistd.h>

#include "xboxcont.h"
#include "input.h"

void XboxController::create_handling_thread()
{
	thread_stopped_ = 0;
	std::thread t(&XboxController::handle_message_func,this);
	thread_.swap(t);
}

void XboxController::handle_message_func()
{
	int data_len = 0;
	int r = 0;
	XboxMessage data;
	
	r = libusb_claim_interface(handle_, interface_);
	if (r < 0) {
			fprintf(stderr, "claim interface error %d - %s\n", r, libusb_strerror((libusb_error)r));
			goto OUT;
	}
	printf("claimed interface %d\n",interface_);

	while(!thread_stopped_){
		// Read data
		r = libusb_interrupt_transfer(handle_, endpoint_, (unsigned char *)&data, sizeof(XboxMessage), &data_len, usb_timeout_);
		switch (r) {
		case LIBUSB_ERROR_TIMEOUT: break;
		case LIBUSB_ERROR_PIPE: printf("LIBUSB_ERROR_PIPE\n"); goto OUT;
		case LIBUSB_ERROR_OVERFLOW: printf("LIBUSB_ERROR_OVERFLOW\n"); goto OUT;
		case LIBUSB_ERROR_NO_DEVICE: printf("LIBUSB_ERROR_NO_DEVICE\n"); goto OUT;
		case LIBUSB_ERROR_BUSY: printf("LIBUSB_ERROR_BUSY\n"); goto OUT;
		case LIBUSB_ERROR_INVALID_PARAM: printf("LIBUSB_ERROR_INVALID_PARAM\n"); goto OUT;
		}
		// print_xbox_message(data);
		if(input_handler_)
			handle_xbox_button_event(data.buttons);
	}
OUT:
	thread_stopped_ = 1;
	close_device();
	return;
}

void XboxController::handle_xbox_button_event(uint16_t status)
{
	const int debounce_threshold = 1;
	static unsigned int debounce = 0;
	static unsigned int debounce_status = 0xc0;
	static uint16_t prev_status = 0xc0;
	// If 1, only sends up/down events
	// If 2, only sends left/right events
	static int pad_dir = 0;

	if(status==debounce_status)
		++debounce;
	else {
		debounce_status = status;
		debounce = 0;
	}
	if(debounce>=debounce_threshold && prev_status!=status){
		InputEvent event;
		unsigned char event_type=0;
		// Check pen press status
		for(int i=0;i<16;++i){
			unsigned char pressed = (status>>i)%2;
			if(pressed!=((prev_status>>i)%2)){
				// Up/Down
				if(i==0||i==1){
					if(pad_dir==2)
						continue;
					pad_dir=pressed?1:0;
				}
				// Left/Right
				if(i==2||i==3){
					if(pad_dir==1)
						continue;
					pad_dir=pressed?2:0;
				}
				event_type=i;
				event.type = InputEventType(event_type);
				// Handle pen input event
				if(pressed)
					input_handler_->handle_input_press(event);
				else
					input_handler_->handle_input_release(event);
			}
		}

		prev_status = status;
	}
}

void XboxController::print_xbox_message(XboxMessage msg)
{
	printf("Header: 0x%04x\n",msg.header);
	printf("Buttons: 0x%04x\n",msg.buttons);
	printf("lt_level: %u\n",msg.lt_level);
	printf("rt_level: %u\n",msg.rt_level);
	printf("lstick_dir_right: %d\n",msg.lstick_dir_right);
	printf("lstick_dir_up: %d\n",msg.lstick_dir_up);
	printf("rstick_dir_right: %d\n",msg.rstick_dir_right);
	printf("rstick_dir_up: %d\n\n",msg.rstick_dir_up);
}