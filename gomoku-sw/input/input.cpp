#include "input.h"
#include <cstdint>
#include <unistd.h>

uint16_t InputEventHandler::wait_for_command()
{
	command_received_ = false;
	while(!command_received_)
		usleep(INPUT_WAIT_INTERVAL_US);

	return command_type_;
}


bool BaseInputDevice::open_device()
{
	printf("%u %u %d %02x\n",vendor_id_,product_id_,interface_,endpoint_);
	int r;

    r = libusb_init(&context_);
    if (r < 0) {
        fprintf(stderr, "fail to init libusb: %d\n", r);
        return false;
    }

	handle_ = libusb_open_device_with_vid_pid(context_,vendor_id_,product_id_);
	if(!handle_){
		std::string error = "Open "+device_name_+" usb device";
		perror(error.c_str());
		goto ERR;
	}

	// Use interface 0
	r = libusb_set_auto_detach_kernel_driver(handle_,1);
	if(r!=LIBUSB_SUCCESS){
		printf("Detach kernel driver failed.\n");
		goto ERR;
	}
	if (libusb_kernel_driver_active(handle_, interface_) == 1) {
		r = libusb_detach_kernel_driver(handle_, interface_);
		if (r == 0) {
			printf("Kernel Driver Detached\n");
		} else {
			fprintf(stderr, "Error detaching kernel driver: %d\n", r);
		}
	}

	return true;

ERR:
	close_device();
	return false;
}

void BaseInputDevice::close_device()
{
	if(handle_!=NULL)
		libusb_close(handle_);
	if(context_)
		libusb_exit(context_);	
}

void BaseInputDevice::stop_handling_thread()
{
	thread_stopped_ = 1;
	if(thread_.joinable())
		thread_.join();
}

void Console::create_handling_thread()
{
	thread_stopped_ = 0;
	thread_ = std::thread(&Console::handle_message_func,this);
}

void Console::handle_message_func()
{
	while(!thread_stopped_){

	}
}
