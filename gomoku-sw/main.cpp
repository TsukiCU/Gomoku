#include "game/game.h"
#include "game/gomoku.h"
#include "display/display.h"
#include "input/touchpad.h"
#include "input/xboxcont.h"
#include "display/message_group_init.h"
#include <cstdio>
#include <vector>

int main()
{
    Gomoku game(1);
    GameMenu menu(&game);
	GMKDisplayMessageGroup msg_group;
	GMKDisplay display;
	XboxController controller;
	Touchpad touchpad;

	init_message_group(msg_group);
	display.set_message_group(&msg_group);
	menu.setMessageGroup(&msg_group);

	if(!display.open_display()){
		perror("Open VGA display");
	}
	else {
		touchpad.set_display(&display);
		menu.setDisplay(&display);
	}

	if(!touchpad.open_device()){
		printf("Open touchpad device failed.\n");
	}
	else{
		touchpad.set_input_handler(&menu);
		touchpad.create_handling_thread();
	}

	if(!controller.open_device()){
		printf("Open controller device failed.\n");
	}
	else {
		controller.set_input_handler(&menu);
		controller.create_handling_thread();
	}

	//while (1);
    menu.gameStart();
	controller.stop_handling_thread();
	touchpad.stop_handling_thread();
	controller.close_device();
	touchpad.close_device();
    return 0;
}
