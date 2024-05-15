#include "game.h"
#include "../src/gomoku.h"
#include "../src/display.h"
#include "../src/touchpad.h"
#include "../src/xboxcont.h"
#include <cstdio>
#include <vector>
void init_message_group(GMKDisplayMessageGroup &group);
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
	controller.close_device();
	touchpad.close_device();
    return 0;
}
void init_message_group(GMKDisplayMessageGroup &group)
{
	GMKDisplayMessageInfo info[14];
	
	info[0].content="BOARD";
	info[0].group=0;
	info[0].group_idx=1;
	info[0].index=0;
	info[0].selectable=1;
	info[0].bounding_box=BBOX(0,0,40,5);
	info[0].right=7;
	group.messages.push_back(info[0]);

	info[1].content="GOMOKU";
	info[1].group=1;
	info[1].group_idx=0;
	info[1].index=1;
	info[1].selectable=0;
	info[1].bounding_box=BBOX(128,128,512,208);
	group.messages.push_back(info[1]);
	
	info[2].content="START PVP";
	info[2].group=1;
	info[2].group_idx=1;
	info[2].index=2;
	info[2].selectable=1;
	info[2].bounding_box=BBOX(248,250,392,270);
	info[2].down=3;
	group.messages.push_back(info[2]);

	info[3].content="START PVE";
	info[3].group=1;
	info[3].group_idx=2;
	info[3].index=3;
	info[3].selectable=1;
	info[3].bounding_box=BBOX(296,280,344,300);
	info[3].up=2;
	info[3].down=4;
	group.messages.push_back(info[3]);

	info[4].content="CREATE LAN";
	info[4].group=1;
	info[4].group_idx=3;
	info[4].index=4;
	info[4].selectable=1;
	info[4].bounding_box=BBOX(240,310,400,330);
	info[4].up=3;
	info[4].down=5;
	group.messages.push_back(info[4]);

	info[5].content="JOIN LAN";
	info[5].group=1;
	info[5].group_idx=4;
	info[5].index=5;
	info[5].selectable=1;
	info[5].bounding_box=BBOX(256,340,384,360);
	info[5].up=4;
	info[5].down=6;
	group.messages.push_back(info[5]);

	info[6].content="EXIT";
	info[6].group=1;
	info[6].group_idx=5;
	info[6].index=6;
	info[6].selectable=1;
	info[6].bounding_box=BBOX(288,370,352,390);
	info[6].up=5;
	group.messages.push_back(info[6]);

	info[7].content="Regret";
	info[7].group=2;
	info[7].group_idx=0;
	info[7].index=7;
	info[7].selectable=1;
	info[7].bounding_box=BBOX(520,355,616,375);
	info[7].down=8;
	info[7].left=0xea00;
	group.messages.push_back(info[7]);

	info[8].content="HINT";
	info[8].group=2;
	info[8].group_idx=1;
	info[8].index=8;
	info[8].selectable=1;
	info[8].bounding_box=BBOX(536,385,600,405);
	info[8].up=7;
	info[8].down=9;
	info[8].left=0xeb00;
	group.messages.push_back(info[8]);

	info[9].content="RESIGN";
	info[9].group=2;
	info[9].group_idx=2;
	info[9].index=9;
	info[9].selectable=1;
	info[9].bounding_box=BBOX(520,415,616,435);
	info[9].up=8;
	info[9].down=10;
	info[9].left=0xec00;
	group.messages.push_back(info[9]);

	info[10].content="EXIT";
	info[10].group=2;
	info[10].group_idx=3;
	info[10].index=10;
	info[10].selectable=1;
	info[10].bounding_box=BBOX(536,445,600,465);
	info[10].up=9;
	info[10].left=0xed00;
	group.messages.push_back(info[10]);

	info[11].content="P1";
	info[11].group=3;
	info[11].group_idx=0;
	info[11].index=11;
	info[11].selectable=0;
	info[11].bounding_box=BBOX(583,139,614,158);
	group.messages.push_back(info[11]);

	info[12].content="P2";
	info[12].group=3;
	info[12].group_idx=1;
	info[12].index=12;
	info[12].selectable=0;
	info[12].bounding_box=BBOX(583,309,614,328);
	group.messages.push_back(info[12]);

	info[13].content="P1";
	info[13].group=3;
	info[13].group_idx=2;
	info[13].index=13;
	info[13].selectable=0;
	info[13].bounding_box=BBOX(583,309,614,328);
	group.messages.push_back(info[13]);

	group.generate_visibility();
	for(auto msg:group.messages){
		printf("%s\n",msg.content.c_str());
	}
	for(auto gp:group.group_visibility){
		printf("Vis: 0x%04x\n",gp);
	}
}