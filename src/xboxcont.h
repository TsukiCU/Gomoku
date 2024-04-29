// xboxcont.h
#ifndef XBOXCONT_H
#define XBOXCONT_H

#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <time.h>
#include <unistd.h>

#include "display.h"

// Global variable declaration
extern int eraseDouble;

// Function declarations
void processDirection(unsigned char data, int &a, int &b,GMKDisplay& display);
char processFunction(unsigned char data, int &a, int &b);
char printInput(unsigned char arr[], int size, int &a, int &b,GMKDisplay& display);
int find_xbox_controller();
void getCommandXb(libusb_device_handle **handle, int &x, int &y,GMKDisplay& display);
void close_controller(libusb_device ***devs, libusb_context **ctx);

#endif // XBOXCONT_H