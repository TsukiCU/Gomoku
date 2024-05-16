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
	controller.stop_handling_thread();
	touchpad.stop_handling_thread();
	controller.close_device();
	touchpad.close_device();
    return 0;
}
void init_message_group(GMKDisplayMessageGroup &group)
{
	GMKDisplayMessageInfo info[23];
	
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
	info[3].bounding_box=BBOX(248,280,392,300);
	info[3].up=2;
	info[3].down=4;
	group.messages.push_back(info[3]);

	info[4].content="CREATE ROOM";
	info[4].group=1;
	info[4].group_idx=3;
	info[4].index=4;
	info[4].selectable=1;
	info[4].bounding_box=BBOX(232,310,408,330);
	info[4].up=3;
	info[4].down=5;
	group.messages.push_back(info[4]);

	info[5].content="JOIN ROOM";
	info[5].group=1;
	info[5].group_idx=4;
	info[5].index=5;
	info[5].selectable=1;
	info[5].bounding_box=BBOX(248,340,392,360);
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
	info[11].bounding_box=BBOX(573,142,605,162);
	group.messages.push_back(info[11]);

	info[12].content="P2";
	info[12].group=3;
	info[12].group_idx=1;
	info[12].index=12;
	info[12].selectable=0;
	info[12].bounding_box=BBOX(573,309,605,329);
	group.messages.push_back(info[12]);

	info[13].content="AI";
	info[13].group=3;
	info[13].group_idx=2;
	info[13].index=13;
	info[13].selectable=0;
	info[13].bounding_box=BBOX(573,309,605,329);
	group.messages.push_back(info[13]);

	info[14].content="YOUWIN";
	info[14].group=4;
	info[14].group_idx=0;
	info[14].index=14;
	info[14].selectable=0;
	info[14].bounding_box=BBOX(129,187,385,227);
	group.messages.push_back(info[14]);

	info[15].content="YOULOSE";
	info[15].group=4;
	info[15].group_idx=1;
	info[15].index=15;
	info[15].selectable=0;
	info[15].bounding_box=BBOX(125,187,381,227);
	group.messages.push_back(info[15]);

	info[16].content="P1WIN";
	info[16].group=4;
	info[16].group_idx=2;
	info[16].index=16;
	info[16].selectable=0;
	info[16].bounding_box=BBOX(150,187,374,227);
	group.messages.push_back(info[16]);

	info[17].content="P2WIN";
	info[17].group=4;
	info[17].group_idx=3;
	info[17].index=17;
	info[17].selectable=0;
	info[17].bounding_box=BBOX(125,187,381,227);
	group.messages.push_back(info[17]);

	info[18].content="AREYOUSURE";
	info[18].group=4;
	info[18].group_idx=4;
	info[18].index=18;
	info[18].selectable=0;
	info[18].bounding_box=BBOX(150,190,358,210);
	group.messages.push_back(info[18]);

	info[19].content="SCANNING...";
	info[19].group=4;
	info[19].group_idx=5;
	info[19].index=19;
	info[19].selectable=0;
	info[19].bounding_box=BBOX(240,230,416,250);
	group.messages.push_back(info[19]);

	info[20].content="EXIT";
	info[20].group=5;
	info[20].group_idx=0;
	info[20].index=20;
	info[20].selectable=1;
	info[20].bounding_box=BBOX(220,241,284,261);
	group.messages.push_back(info[20]);

	info[21].content="YES";
	info[21].group=5;
	info[21].group_idx=1;
	info[21].index=21;
	info[21].selectable=1;
	info[21].right=22;
	info[21].bounding_box=BBOX(164,244,212,264);
	group.messages.push_back(info[21]);

	info[22].content="NO";
	info[22].group=5;
	info[22].group_idx=2;
	info[22].index=22;
	info[22].selectable=1;
	info[22].left=21;
	info[22].bounding_box=BBOX(311,244,343,264);
	group.messages.push_back(info[22]);

	group.generate_visibility();
	for(auto msg:group.messages){
		printf("%s\n",msg.content.c_str());
	}
	for(auto gp:group.group_visibility){
		printf("Vis: 0x%04x\n",gp);
	}
}