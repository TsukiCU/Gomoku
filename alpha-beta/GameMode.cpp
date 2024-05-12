#include "game.h"

void GameMenu::displayMode()
{
    std::cout << "Select Game Mode:\n";
    std::cout << "1. PVP \n";
    std::cout << "2. VS AI. You play black \n";
    std::cout << "3. VS AI. You play white \n";
    std::cout << "4. Play over the network \n";
    std::cout << "-1. Exit\n";
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
        std::cout << "\n\nChoose a mode to play. " <<std::endl;
        displayMode();
        int mode = getChoice();
        if (mode == -1) break;
        switch(mode) {
            case 1:
                PvPMode();
                break;
            case 2:
                PvEMode(1);
                break;
            case 3:
                PvEMode(2);
                break;
            case 4:
                networkMode();
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

    std::cout << "\n\nGame started. You Play as " << playerColor << endl;
    std::cout << "\nType in (x, y) to make a move. (0, 0) to regret a move.\
    (-1, -1) to return to main menu. " << playerColor << endl;

    if (aiFirst) {
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);
        game->record.push_back(aiMove);
    }
    game->displayBoard();

    while (1) {
        int x, y;
        std::cin >> x >> y;
        //getCommandXb(&handle,x,y);

        // NOTE: (0, 0) now refers to regretting a move, (-1, -1) stands for returning to main menu.
        if (x == -1 && y == -1) {
            std::cout << "Returning to main menu ..." << std::endl;
            game->resetGame();  // NOTE: modified from clearBoard method in class Gomoku.
            break;
        }
        if (x == 0 && y == 0) {
            if (!game->regret_move()) {
                std::cout << "regret a move, please continue" << endl;
                game->displayBoard();
            }
            continue;
        }
        else if (p1.makeMove(make_pair(x - 1, y - 1))) {
            std::cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game->record.push_back(make_pair(x - 1, y - 1));

        if (game->state == 1) {
            game->displayBoard();
            std::cout << "You win!" << endl;
            break;
        }

        // AI makes a move.
        aiMove = ai.findBestMove();
        ai.makeMove(aiMove);
        game->record.push_back(aiMove);

        std::cout << "You made a move at " << x << ", " << y << ", " << "AI made a move at " 
        << aiMove.first + 1 << ", " << aiMove.second + 1 << "\n" << endl;

        game->displayBoard();

        if (game->state == 1) {
            game->resetGame();
            std::cout << "You lose!" << endl;
            break;
        }
    }
}

void GameMenu::PvPMode()
{
    game->mode = 0;
    Player p1(game, 1);
    Player p2(game, 2);

    cout << "\n\nGame started. " << endl;
    game->displayBoard();

    while (1) {
        // Player 1 makes a move.
        int x, y;
        std::cin >> x >> y;

        if (x == -1 && y == -1) {
            game->resetGame();
            std::cout << "Player 1 quitting. " << std::endl;
            break;
        }

        if (x == 0 && y == 0) {
            if (!game->regret_move()) {
                std::cout << "White regrets a move, please continue" << endl;
                game->displayBoard();
            }
            continue;
        }

        if (p1.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game->record.push_back(make_pair(x-1, y-1));
        cout << "Black made a move at " << x << ", " << y << endl;

        if (game->state == 1) {
            game->displayBoard();
            cout << "Black wins! Returning to main menu. " << endl;
            game->resetGame();
            break;
        }
        game->displayBoard();

        // Player 2 makes a move.
        std::cin >> x >> y;

        if (x == -1 && y == -1) {
            game->resetGame();
            std::cout << "Player 2 quitting. " << std::endl;
            break;
        }

        if (x == 0 && y == 0) {
            if (!game->regret_move()) {
                std::cout << "Black regrets a move, please continue" << endl;
                game->displayBoard();
            }
            continue;
        }

        if (p2.makeMove(make_pair(x - 1, y - 1))) {
            cout << "Invalid move!" << '\n' << endl;
            continue;
        }
        game->record.push_back(make_pair(x-1, y-1));
        cout << "White made a move at " << x << ", " << y << endl;

        if (game->state == 1) {
            game->displayBoard();
            cout << "White wins! Returning to main menu. " << endl;
            game->resetGame();
            break;
        }
        game->displayBoard();
    }
}

void GameMenu::networkMode()
{
    int role = -1;  // -1 for unknown, 0 for server, 1 for client;
    bool gameStart = false; // If game starts, stop listening and broadcasting threads.
    bool noPlayersFound = false; // If waiting too long(more than 15 seconds), set this to true and report this message.
    string server_ip = "";
    string myTimestamp =  "TIMESTAMP: " + to_string(getCurTimeStamp());

    // My basic info
    printf("My IP: %s | My Timestamp: %lld\n\n", getLocalIP().c_str(), getTimeFromStamp(myTimestamp));
    printf("Welcome to Gomoku, actively looking for opponents...\n"); 

    thread broadcaster(broadcastPresence, ref(role), ref(myTimestamp), ref(gameStart), ref(noPlayersFound));
    thread listener(listenForBroadcast, ref(role), ref(myTimestamp), ref(gameStart), ref(server_ip), ref(noPlayersFound));

    broadcaster.join();
    listener.join();

    if (noPlayersFound) {
        printf("You know the game is too highbrow. Nobody's around here for now. But you can always start later.\n");
        return;
    }

    // If received broadcast from others who started later than us, we are the server.
    // Otherwise, we are the client.
    if (!role) {
		GMKServer server;
		if(!server.Create())
			return;
		if(!server.WaitForPlayer()){
			printf("Wait for player failed.\n");
			return;
		}
		while(true){
			server.StartGame();
			while(server.CheckGameResult()<0){
				int x,y;
				cin.clear();
				cin >> x >> y;
				server.MakeMove(x, y);
			}
			// Connection closed
			if(server.CheckGameResult()==3){
				server.WaitForPlayer();
				continue;
			}
			else if(server.CheckGameResult()==1){
				printf("You win!\n");
			}
			else{
				printf("You lose!\n");
			}
			server.ResetBoard();
			printf("Press enter to restart.\n");
			fflush(stdin);
			getchar();
		}
    }

    else if (role == 1) {
		GMKClient client;
		if(!client.Connect(server_ip.c_str()))
			return;
		while(true){
			while(client.CheckGameResult()<0){
				int x,y;
				cin.clear();
				fflush(stdin);
				cin >> x >> y;
				client.MakeMove(x, y);
			}
			if(client.CheckGameResult()==1){
				printf("You win!\n");
			}
			else{
				printf("You lose!\n");
			}
			printf("Waiting for server to restart...\n");
			client.ResetBoard();
		}
    }

    else {
        // Shouldn't reach here.
        fprintf(stderr, "Roles Undetermined Error.\n");
    }
}