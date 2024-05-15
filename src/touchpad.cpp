#include <cstdint>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <thread>
#include "input.h"
#include "touchpad.h"

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
	printf("Coordinate:%u,%u\n",(uint16_t)((msg.horizontal)/51.2),(uint16_t)((msg.vertical)/32768.0*480));
	printf("Pressure:%u\n",msg.pressure);
	printf("Unknown:%u\n",msg.unknown);
}

void Touchpad::create_handling_thread()
{
	thread_stopped_ = 0;
	std::thread t(&Touchpad::handle_message_func,this);
	thread_.swap(t);
}

void Touchpad::handle_message_func()
{
	int data_len = 0;
	int r = 0;
	uint16_t vga_x,vga_y;
	bool show_cursor=true;
	XPPenMessage data;
	
	r = libusb_claim_interface(handle_, interface_);
	if (r < 0) {
			fprintf(stderr, "claim interface error %d - %s\n", r, libusb_strerror((libusb_error)r));
			goto OUT;
	}
	printf("claimed interface %d\n",interface_);

	while(!thread_stopped_){
		// Read data
		r = libusb_interrupt_transfer(handle_, endpoint_, (unsigned char *)&data, sizeof(data), &data_len, usb_timeout_);
		show_cursor = true;
		switch (r) {
		case LIBUSB_ERROR_TIMEOUT: show_cursor = false; break;
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
		vga_x = ((data.horizontal)/51.2);
		vga_y = ((data.vertical)/32768.0*480);
		if(display_){
			if(!display_->update_touchpad_cursor(vga_x,vga_y,show_cursor))
				perror("update touchpad cursor");
		}
		if(input_handler_)
			handle_touchpad_pen_event(&data);
	}
OUT:
	thread_stopped_ = 1;
	close_device();
	return;
}

void Touchpad::handle_touchpad_pen_event(struct XPPenMessage *msg)
{
	const int debounce_threshold = 0;
	static unsigned int debounce = 0;
	static unsigned int debounce_status = 0xc0;
	static unsigned char prev_status = 0xc0;
	if(msg->status==debounce_status)
		++debounce;
	else {
		debounce_status = msg->status;
		debounce = 0;
	}
	if(debounce>=debounce_threshold && prev_status!=msg->status){
		// DEBUG
		print_touchpad_message(*msg);
		printf("\n\n");

		InputEvent event;
		// Set event PAD type 1<<5
		unsigned char event_type=16;

		event.vga_x = ((msg->horizontal)/51.2);
		event.vga_y = ((msg->vertical)/32768.0*480);
		// Check pen press status
		for(int i=0;i<TOUCHPAD_MOUSE_EVENT_COUNT;++i){
			unsigned char pressed = (msg->status>>i)%2;
			if(pressed!=((prev_status>>i)%2)){
				// Set press/release and button status
				event_type+=i;
				event.type = InputEventType(event_type);
				// Send pen input event
				if(pressed)
					input_handler_->handle_input_press(event);
				else
					input_handler_->handle_input_release(event);
			}
		}

		prev_status = msg->status;
	}
}