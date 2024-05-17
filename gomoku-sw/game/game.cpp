#include "game.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>
#include <unistd.h>
#include "../network/network.h"

void* findPlayersWait(void *arg)
{
	sleep(3);
	return NULL;
}

void GameMenu::setDisplay(GMKDisplay *display)
{
	if(game)
		game->set_display(display);
	this->display=display;
}

void GameMenu::showMenu()
{
	if(display){
		display->show_game_result(0,false);
		display->show_menu();
	}
	else{
		msg_group->update_group_visibility(0, false);
		msg_group->update_group_visibility(1, true);
		msg_group->update_group_visibility(2, false);
		msg_group->update_group_visibility(3, false);
	}
	selected_msg_index = msg_group->first_selectable_message();
	if(display)
		display->update_select(selected_msg_index);
	//msg_group->display_selectable();
}

void GameMenu::showBoard(bool top_first)
{
	if(display){
		display->show_game_result(0,false);
		display->show_confirm_message(false);
		display->clear_board();
		display->set_player_piece(top_first,false);
		display->set_turn_mark(top_first,false);
		display->show_board(false);
	}
	else{
		msg_group->update_group_visibility(0, true);
		msg_group->update_group_visibility(1, false);
		msg_group->update_group_visibility(2, true);
		msg_group->update_group_visibility(3, true);
	}
	selected_msg_index = 0x7700;
	board_x = 8;
	board_y = 8;
	if(display)
		display->update_select(selected_msg_index);
	//msg_group->display_selectable();
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
    while (1) {
		showMenu();
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

	showBoard(!aiFirst);
	if(display)
		display->update_p2_profile(0);
    std::cout << "\n\nGame started. You Play as " << playerColor << endl;
    std::cout << "\nType in (board_x, board_y) to make a move. (0, 0) to regret a move.\
    (-1, -1) to return to main menu. " << playerColor << endl;

    if (aiFirst) {
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);
    }
    game->displayBoard();

	while (1) {
        uint16_t command = wait_for_command();
		switch (command) {
		case 10: // Quit
			// should add display message			// 	continue;
			if(display)
				display->show_confirm_message();
			selected_msg_index=msg_group->first_selectable_message();
			if(!wait_for_confirm()){
				if(display)
					display->show_confirm_message(false);
				selected_msg_index=msg_group->first_selectable_message();
				continue;
			}
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
			game->end_game(!(player==1));
			if(display)
				display->show_game_result(1);
			selected_msg_index = msg_group->first_selectable_message();
			// Wait for confirm command, quit to main menu.
			while(wait_for_confirm()){
				game->resetGame();
				return;
			}
			continue;
		case 8:{
			std::pair<int, int> ai_hint = ai.findBestMove();
			if(display)
				display->show_hint(ai_hint.first, ai_hint.second);
			// std::cout << "Hint: " << hint.first + 1 << ", " << hint.second + 1 << endl;
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
		board_x=-1;
		board_y=-1;

        if (game->state == 1) {
            game->displayBoard();
			playAnime();
            std::cout << "You win!" << endl;
			if(display)
				display->show_game_result(0);
			selected_msg_index = msg_group->first_selectable_message();
			// Wait for confirm command, quit to main menu.
			while(wait_for_confirm()){
				game->resetGame();
				return;
			}
        }
        game->displayBoard();

        // AI makes a move.
		assert(game->current_player+player==3);
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);

        std::cout << "You made a move at " << board_x << ", " << board_y << ", " << "AI made a move at " 
        << aiMove.first + 1 << ", " << aiMove.second + 1 << "\n" << endl;

        if (game->state == 1) {
            game->displayBoard();
			playAnime();
            std::cout << "You lose!" << endl;
			if(display)
				display->show_game_result(1);
			selected_msg_index = msg_group->first_selectable_message();
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
	if(display)
		display->update_p2_profile(1);
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
        cout << str_player << "made a move at " << board_x << ", " << board_y << endl;
		board_x=-1;
		board_y=-1;

        if (game->state == 1) {
            game->displayBoard();
			playAnime();
			if(display)
				// current_player=1 -> black -> p1 -> result=2
				// current_player=2 -> white -> p2 -> result=3
				display->show_game_result(game->current_player+1);
			selected_msg_index=msg_group->first_selectable_message();
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
	game->mode=1;

    if (server) {
		GMKServer server(game);
		server.set_cancel(&cancel);
		server.set_event_handler(this);

		if(!server.create())
			return;
		showBoard();
		if(display)
			display->update_p2_profile(2);
		waiting = true;
		if(!server.wait_for_player()){
			printf("Wait for player failed.\n");
			game->resetGame();
			waiting = false;
			command_type_=0;
			return;
		}
		bool top_first = server.start_game();
		showBoard(top_first);
		if(display)
			display->update_p2_profile(1);
		while(server.check_game_result()<0){
			uint16_t command = wait_for_command();
			switch (command) {
			case 10: // Quit
				if(display)
					display->show_confirm_message();
				selected_msg_index=msg_group->first_selectable_message();
				if(!wait_for_confirm())
					continue;
				server.resign();
				std::cout << "Returning to main menu ..." << std::endl;
				game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
				return;
			case 0: // Move
				// Make move
				if (!server.make_move(board_x - 1, board_y- 1)){
					printf("Invalid move!\n");
				}
				board_x=-1;
				board_y=-1;
				break;
			case 7: // Regret
				// if (!game->regret_move()) {
				// 	std::cout << ((game->current_player == 2) ? "Black" : "White") <<" regrets a move, please continue" << endl;
				// 	game->displayBoard();
				// }
				printf("Regret disabled in net work mode.\n");
				break;
			case 9:
				// Resign without confirm
				server.resign();
				break;
			case 8:
				// HINT
				ai_hint = ai.findBestMove();
				// std::cout << "Hint: " << hint.first + 1 << ", " << hint.second + 1 << endl;
				if(display)
					display->show_hint(ai_hint.first, ai_hint.second);
				break;
			default:
				printf("Invalid command!\n");
				break;
			}
			// Remote player resign check, end the loop
			if(game->state==1)
				break;
		}

		// Connection closed
		if(server.check_game_result()==3){
			printf("Connection lost!\n");
			// Probably print you win.
			if(display)
				display->show_game_result(0);
			selected_msg_index=msg_group->first_selectable_message();
		}
		else if(server.check_game_result()==1) {
			printf("You win!\n");
			playAnime();
			if(display)
				display->show_game_result(0);
			selected_msg_index=msg_group->first_selectable_message();
		}
		else{
			printf("You lose!\n");
			playAnime();
			if(display)
				display->show_game_result(1);
			selected_msg_index=msg_group->first_selectable_message();
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
		client.set_event_handler(this);
		client.set_cancel(&cancel);
		client.send_server_discover();
		
		string null;
		string server_status[3] = {"Down","Gaming","Ready"};
		GMKServerInfo info;
		bool found=false;

		printf("Discovering server...\n");
		if(display)
			display->show_scanning_message();
		// Wait for 3 seconds but without blocking the main thread.
		client.wait_for_scan();
		display->show_scanning_message(false);

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
		//client.connect_to("128.59.65.165");

		/* Connected. Show board page. */
		showBoard();
		if(display)
			display->update_p2_profile(1);
		while(client.check_game_result()<0){
			uint16_t command = wait_for_command();
			switch (command) {
			case 10: // Quit confirm
				if(display)
					display->show_confirm_message();
				selected_msg_index=msg_group->first_selectable_message();
				if(!wait_for_confirm())
					continue;
				std::cout << "Returning to main menu ..." << std::endl;
				game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
				showMenu();
				return;
			case 0: // Move or remote resigns
				if (!client.make_move(board_x - 1, board_y - 1)){
				}
				board_x=-1;
				board_y=-1;
				break;
			case 7: // Regret
				// if (!game->regret_move()) {
				// 	std::cout << ((game->current_player == 2) ? "Black" : "White") <<" regrets a move, please continue" << endl;
				// 	game->displayBoard();
				// }
				printf("Regret disabled in net work mode.\n");
				continue;
			case 9:
				// Resign without confirm
				client.resign();
				break;
			case 8:
				// HINT
				ai_hint = ai.findBestMove();
				// std::cout << "Hint: " << hint.first + 1 << ", " << hint.second + 1 << endl;
				if(display)
					display->show_hint(ai_hint.first, ai_hint.second);
				continue;
			default:
				printf("Invalid command!\n");
				break;
			}
			// Resign check, end the loop
			if (game->state == 1)
				break;
		}

		if(client.check_game_result()==3){ // Connection closed
			printf("Connection lost!\n");
			// Probably print you win.
			printf("You win!\n");
			if(display)
				display->show_game_result(0);
			selected_msg_index=msg_group->first_selectable_message();
		}
		else if(client.check_game_result()==1){
			printf("You win!\n");
			playAnime();
			if(display)
				display->show_game_result(0);
			selected_msg_index=msg_group->first_selectable_message();
		}
		else{
			printf("You lose!\n");
			playAnime();
			if(display)
				display->show_game_result(1);
			selected_msg_index=msg_group->first_selectable_message();
		}

		bool confirm = wait_for_confirm();
		if(confirm){
			game->resetGame();
			return;
		}
    }
}

void GameMenu::playAnime()
{
	if(!display)
		return;
	block_input=true;
	// Hide last mark
	display->update_register(2, display->get_register(2)|0x00ff);
	int count=0;
	for(auto piece:game->winArray){
		display->update_piece_info(piece.first, piece.second, 3,0);
		usleep(500*1000);
		++count;
		if(count>=5)
			break;
	}
	block_input=false;
}

void GameMenu::handle_input_press(InputEvent event){
	if(block_input)
		return;
	uint16_t cursor;
	switch (event.type) {
	case XBOX_UP:
	case XBOX_DOWN:
	case XBOX_LEFT:
	case XBOX_RIGHT: // Direction buttons
		// Next message
		selected_msg_index = msg_group->next_message_by_direction(selected_msg_index, event.type);
		board_x = 0xf;
		board_y = 0xf;
		if(msg_group->is_board_selected(selected_msg_index)){
			printf("(%d,%d) selected!\n",(selected_msg_index>>12),((selected_msg_index>>8)&0xf));
			board_x = (selected_msg_index>>12)+1;
			board_y = ((selected_msg_index>>8)&0xf)+1;
		}
		else
			printf("%s selected!\n",msg_group->messages[msg_group->get_message_command(selected_msg_index)].content.c_str());
		if(display)
			display->update_select(selected_msg_index);
		break;
	case XBOX_A: // Confirm selection
		command_type_ = msg_group->get_message_command(selected_msg_index);
		if(command_type_==10 && waiting)
			cancel = true;
		else
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
		selected_msg_index = cursor;
		board_x = 0xf;
		board_y = 0xf;
		if(msg_group->is_board_selected(selected_msg_index)){
			printf("(%d,%d) selected!\n",(selected_msg_index>>12),((selected_msg_index>>8)&0xf));
			board_x = (selected_msg_index>>12)+1;
			board_y = ((selected_msg_index>>8)&0xf)+1;
		}
		else
			printf("%s selected!\n",msg_group->messages[msg_group->get_message_command(cursor)].content.c_str());
		command_type_ = msg_group->get_message_command(selected_msg_index);
		if(command_type_==10 && waiting)
			cancel = true;
		else
			command_received_ = true;
		break;
	case PAD_MOUSE_RIGHT:
	case PAD_MOUSE_MID:
	case NONE:
		command_type_ = 0xFF;
		command_received_ = true;
	default:
		break;
    }

}

bool GameMenu::wait_for_confirm()
{
	// Wait for confirm command, quit to main menu.
	uint16_t command;
	while(true){
		command = wait_for_command();
		switch (command) {
		case 20: // MESSAGE_BOX_EXIT
			return true;
		case 21: // MESSAGE_YES
			return true;
		case 22: // MESSAGE_NO
			return false;
		default:
			printf("Invalid command! %d\n",command);
			break;
		};
	}
}