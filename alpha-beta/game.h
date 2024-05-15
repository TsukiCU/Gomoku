#ifndef _MAIN_MENU_HH
#define _MAIN_MENU_HH

#include "../src/gomoku.h"
#include "gomokuAI.h"
#include "network.h"
#include "../src/players.h"
#include "../src/display.h"
#include <cstdint>

class GameMenu : public InputEventHandler{
private:
    Gomoku *game;
	GMKDisplay *display=NULL;
    int currentMode;
	uint16_t board_x, board_y;
	uint16_t selected_msg_index;
	GMKDisplayMessageGroup *msg_group;

public:
    GameMenu(Gomoku *game) : game(game), currentMode(0) {}

    void displayMode();         	// Display mode options.
    int  getChoice();           	// Get user's choice.

    // Different modes
    void PvPMode();             	// Player vs player. Integrated with pvp.cpp. Mode code : 1
    void PvEMode(int player);   	// Player vs AI. Integrated with pve.cpp, evp.cpp. Mode code : 2, 3
    void networkMode(bool server);	// Players play over the network. Intergrated with network.cpp. Mode code : 4

	void handle_input_press(InputEvent event) override;
	void handle_input_release(InputEvent event) override {}

    void gameStart();           	// Game start.

	void showMenu();
	void showBoard();

	void setDisplay(GMKDisplay *display);
	void setMessageGroup(GMKDisplayMessageGroup *group){this->msg_group=group;}

	
	bool wait_for_confirm();		// Wait for confirm message. True means confirm, False means cancel.
};

#endif