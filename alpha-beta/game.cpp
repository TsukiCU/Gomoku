#include "game.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include <unistd.h>

void* findPlayersWait(void *arg)
{
	sleep(3);
	return NULL;
}

void GameMenu::showMenu()
{
	if(display)
		display->show_menu();
	else{
		msg_group->update_group_visibility(0, false);
		msg_group->update_group_visibility(1, true);
		msg_group->update_group_visibility(2, false);
		msg_group->update_group_visibility(3, false);
		selected_msg_index = msg_group->first_selectable_message();
	}
}

void GameMenu::showBoard()
{
	if(display)
		display->show_board(true);
	else{
		msg_group->update_group_visibility(0, true);
		msg_group->update_group_visibility(1, false);
		msg_group->update_group_visibility(2, true);
		msg_group->update_group_visibility(3, true);
		selected_msg_index = msg_group->first_selectable_message();
	}
}

void GameMenu::displayMode()
{
    std::cout << "Select Game Mode:\n";
	for(auto select: msg_group->selectable){
		printf("%d. %s\n",select.index,select.content.c_str());
	}
}

int GameMenu::getChoice()
{
    int choice;
    std::cin >> choice;
    return choice;
}

void GameMenu::gameStart()
{
	showMenu();
    while (1) {
        std::cout << "\n\nChoose a mode to play. " <<std::endl;
        displayMode();
        uint16_t mode = wait_for_command();
		srand(time(NULL));
        switch(mode) {
			case 6: // EXIT
				return;
            case 3:	// PVE
				if((rand()%4)>=2)
					PvEMode(1);
				else
					PvEMode(2);
                break;
            case 2:	// PVP
                PvPMode();
                break;
            case 4: // CREATE LAN
                networkMode(1);
                break;
            case 5:	// JOIN LAN
                networkMode(0);
                break;
            default:
                std::cout << "Invalid mode. Type again\n";
        }
    }
}

void GameMenu::PvEMode(int player)
{
    /*
     * Player plays : black (if player == 1) , white (if player == 2)
     */
    game->mode = 1;
    Player p1(game, player);
    GomokuAI ai(game, 1);
    int aiFirst = (player == 1) ? 0 : 1;
    string playerColor = (player == 1) ? "black" : "white";
    pair<int, int> aiMove = make_pair(-1, -1);  // ai moves

	showBoard();
    std::cout << "\n\nGame started. You Play as " << playerColor << endl;
    std::cout << "\nType in (board_x, board_y) to make a move. (0, 0) to regret a move.\
    (-1, -1) to return to main menu. " << playerColor << endl;

    if (aiFirst) {
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);
        game->record.push_back(aiMove);
    }
    game->displayBoard();

	while (1) {
        uint16_t command = wait_for_command();
		switch (command) {
		case 10: // Quit
			// TODO: Confirm message?
			// should add display message
			// if(!wait_for_confirm())
			// 	continue;
			std::cout << "Returning to main menu ..." << std::endl;
            game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
            return;
		case 0: // Move
			break;
		case 7: // Regret
            if (!game->regret_move()) {
                std::cout << "regret a move, please continue" << endl;
                game->displayBoard();
            }
            continue;
		case 9:
			// Resign without confirm
			game->displayBoard();
            std::cout << "You lose!" << endl;
			printf("Resign not implemented.\n");
			continue;
		case 8:{
			// TODO: HINT
			std::pair<int, int> ai_hint = ai.findBestMove();
			// std::cout << "Hint: " << hint.first + 1 << ", " << hint.second + 1 << endl;
			// TODO: display hint on the board
			continue;
		}
		default:
			printf("Invalid command!\n");
			continue;
		}

        // NOTE: (0, 0) now refers to regretting a move, (-1, -1) stands for returning to main menu.
		if (p1.makeMove(make_pair(board_x - 1, board_y- 1))) {
            std::cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game->record.push_back(make_pair(board_x - 1, board_y- 1));

        if (game->state == 1) {
            game->displayBoard();
            std::cout << "You win!" << endl;
			// Wait for confirm command, quit to main menu.
			bool confirm = wait_for_confirm();
			if(confirm){
				game->resetGame();
				return;
			}
        }
        game->displayBoard();

        // AI makes a move.
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);
        game->record.push_back(aiMove);

        std::cout << "You made a move at " << board_x << ", " << board_y << ", " << "AI made a move at " 
        << aiMove.first + 1 << ", " << aiMove.second + 1 << "\n" << endl;

        if (game->state == 1) {
            game->displayBoard();
            std::cout << "You lose!" << endl;
			// Wait for confirm command, quit to main menu.
			bool confirm = wait_for_confirm();
			if(confirm){
				game->resetGame();
				return;
			}
        }
        game->displayBoard();
    }
}

void GameMenu::PvPMode()
{
    game->mode = 0;
    Player p1(game, 1);
    Player p2(game, 2);

	showBoard();
    cout << "\n\nGame started. " << endl;
    game->displayBoard();

    while (1) {
        uint16_t command = wait_for_command();
		switch (command) {
		case 10: // Quit without confirm
			std::cout << "Returning to main menu ..." << std::endl;
            game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
            return;
		case 0: // Move
			break;
		case 7: // Regret
            if (!game->regret_move()) {
                std::cout << ((game->current_player == 2) ? "Black" : "White") <<" regrets a move, please continue" << endl;
                game->displayBoard();
            }
            continue;
		case 8:
			// HINT. There should not be a hint in PVP mode.
			printf("Tip not implemented.\n");
			continue;
		case 5:
			// TODO: restart?
		default:
			printf("Invalid command!\n");
			continue;
		}
		
		std::string str_player = "White ";
		Player *p = &p2;
		if(game->current_player == 2){
			str_player = "Black ";
			p=&p1;
		}
        if (p->makeMove(make_pair(board_x - 1, board_y- 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game->record.push_back(make_pair(board_x-1, board_y-1));
        cout << str_player << "made a move at " << board_x << ", " << board_y << endl;

        if (game->state == 1) {
            game->displayBoard();
            cout << str_player << "wins! Returning to main menu. " << endl;
			// Wait for confirm command, quit to main menu.
			bool confirm = wait_for_confirm();
			if(confirm){
				game->resetGame();
				return;
			}
        }
        game->displayBoard();
    }
}

void GameMenu::networkMode(bool server)
{
    GomokuAI ai(game, 1); // AI is here to provide hints.
	std::pair<int, int> ai_hint;

    if (server) {
		GMKServer server(game);
		if(!server.create())
			return;
		showBoard();
		if(!server.wait_for_player()){
			printf("Wait for player failed.\n");
			return;
		}
		server.start_game();
		while(server.check_game_result()<0){
			uint16_t command = wait_for_command();
			switch (command) {
			case 10: // Quit
				// TODO: Confirm message?
				// should add display message
				// if(!wait_for_confirm())
				// 	continue;
				server.resign();
				std::cout << "Returning to main menu ..." << std::endl;
				game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
				return;
			case 0: // Move or remote resigns
				break;
			case 7: // Regret
				if (!game->regret_move()) {
					std::cout << ((game->current_player == 2) ? "Black" : "White") <<" regrets a move, please continue" << endl;
					game->displayBoard();
				}
				continue;
			case 9:
				// Resign without confirm
				if(server.resign())
					break;
				else
					continue;
			case 8:
				// HINT
				ai_hint = ai.findBestMove();
				// std::cout << "Hint: " << hint.first + 1 << ", " << hint.second + 1 << endl;
				printf("HINT disabled in PVP.\n");
			default:
				printf("Invalid command!\n");
				continue;
			}

			/*if (board_x == -1 && board_y == -1) {
				// FIXME: Need to send "resign" message to the other player, so that the other player can return to main menu.
				string confirm;
				std::cout << "Returning to main menu. Type \"yes\" to continue " << std::endl;
				std::cin >> confirm;
				if (confirm == "yes") {
					// FIXME: This will throw an error. Need to make the threads in tcp.cpp join.
					// Possibly set a gameEnd flag in GMKServer
					std::cout << "You lose. Returning to main menu." << std::endl;
					return;
				}
				else {
					std::cout << "Continuing." << std::endl;
					continue;
				}
			}*/
			// Remote player resign check, end the loop
			if(game->state==1)
				break;

			// Make move
			if (!server.make_move(board_x - 1, board_y- 1)){
				printf("Invalid move!\n");
			}
		}

		// Connection closed
		if(server.check_game_result()==3){
			server.wait_for_player();
			printf("Connection lost!\n");
			// Probably print you win.
		}
		else if(server.check_game_result()==1) {
			printf("You win!\n");
		}
		else{
			printf("You lose!\n");
		}

		// Wait for confirm command, quit to main menu.
		bool confirm = wait_for_confirm();
		if(confirm){
			game->resetGame();
			return;
		}
    }
    else {
		GMKClient client(game);
		client.send_server_discover();
		string null;
		string server_status[3] = {"Down","Gaming","Ready"};
		GMKServerInfo info;
		bool found=false;

		// TODO : Display some message on display
		printf("Discovering server...\n");
		
		// Wait for 3 seconds but without blocking the main thread.
		pthread_t waitThread;
		pthread_create(&waitThread, NULL, findPlayersWait, NULL);
		pthread_join(waitThread, NULL);
		// sleep(3);

		// TODO: should input command to cancel discover
		if(client.get_server_list().empty()){
			printf("No server discovered!\n");
			return;
		}
		printf("Server List\n%-15s\t%-4s\t%s\n","IP address","Port","Status");
		for(auto server:client.get_server_list()){
			printf("%s\t%-4u\t%-6s\n",server.address.c_str(),server.port,server_status[server.status].c_str());
			if(server.status==2){
				info = server;
				found = true;
			}
		}
		if(!found){
			printf("No available server found!\n");
			return;
		}
		printf("Connecting %s:%u\n",info.address.c_str(),info.port);
		if(!client.connect_to(info))
			return;

		/* Connected. Show board page. */
		showBoard();
		while(client.check_game_result()<0){
			uint16_t command = wait_for_command();
			switch (command) {
			case 10: // TODO: Quit confirm
				std::cout << "Returning to main menu. Type \"yes\" to continue " << std::endl;
				std::cout << "Returning to main menu ..." << std::endl;
				game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
				showMenu();
				return;
			case 0: // Move or remote resigns
				break;
			case 7: // Regret
				if (!game->regret_move()) {
					std::cout << ((game->current_player == 2) ? "Black" : "White") <<" regrets a move, please continue" << endl;
					game->displayBoard();
				}
				continue;
			case 9:
				// Resign without confirm
				if(client.resign())
					break;
				else
					continue;
			case 8:
				// HINT
				printf("HINT disabled in PVP.\n");
			default:
				printf("Invalid command!\n");
				continue;
			}
			// Resign check, end the loop
			if (game->state == 1)
				break;
			if (!client.make_move(board_x - 1, board_y - 1)){
				//TODO: error handling
			}
		}

		if(client.check_game_result()==3){ // Connection closed
			printf("Connection lost!\n");
			// Probably print you win.
			printf("You win!\n");
		}
		else if(client.check_game_result()==1){
			printf("You win!\n");
		}
		else{
			printf("You lose!\n");
		}

		bool confirm = wait_for_confirm();
		if(confirm){
			game->resetGame();
			return;
		}
    }
}

void GameMenu::handle_input_press(InputEvent event){
	uint16_t cursor;
	switch (event.type) {
	case XBOX_UP:
	case XBOX_DOWN:
	case XBOX_LEFT:
	case XBOX_RIGHT: // Direction buttons
		// Next message
		selected_msg_index = msg_group->next_message_by_direction(selected_msg_index, event.type);
		board_x = (selected_msg_index>>12)+1;
		board_y = ((selected_msg_index>>8)&0xf)+1;
		if(display)
			display->update_select(selected_msg_index);
		if(msg_group->is_board_selected(selected_msg_index)){
			printf("(%d,%d) selected!\n",(selected_msg_index>>12),((selected_msg_index>>8)&0xf));
		}
		else
			printf("%s selected!\n",msg_group->messages[msg_group->get_message_command(selected_msg_index)].content.c_str());
		break;
	case XBOX_A: // Confirm selection
		command_type_ = msg_group->get_message_command(selected_msg_index);
   		command_received_ = true;
		printf("%s selected!\n",msg_group->messages[msg_group->get_message_command(selected_msg_index)].content.c_str());
		break;
	case XBOX_B: // TODO: Cancel buttion
	case XBOX_X:
	case XBOX_Y:
	case PAD_MOUSE_LEFT: // Click uses current cursor position
		cursor = msg_group->message_select_by_vga_xy(event.vga_x, event.vga_y);
		// command_type_ = msg_group->get_message_command(cursor);
		if(cursor==0xffff){
			printf("None selected! (%d,%d)\n",event.vga_x,event.vga_y);
			break;
		}
		else if(msg_group->is_board_selected(cursor)){
			printf("(%d,%d) selected!\n",(selected_msg_index>>12),((selected_msg_index>>8)&0xf));
		}
		else
			printf("%s selected!\n",msg_group->messages[msg_group->get_message_command(cursor)].content.c_str());
		// command_received_ = true;
		break;
	case PAD_MOUSE_RIGHT:
	case PAD_MOUSE_MID:
	default:
		break;
    }

}

bool GameMenu::wait_for_confirm()
{
	// Wait for confirm command, quit to main menu.
	// TODO: Probably pop up a message on display
	uint16_t command;
	while(true){
		command = wait_for_command();
		switch (command) {
		case 0: // TODO: set correct confirm index
			return true;
		case 1: // TODO: set correct cancel index
			return false;
		default:
			printf("Invalid command!\n");
			break;
		};
	}
}