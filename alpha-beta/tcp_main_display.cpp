#include "../src/tcp.h"
#include <cstdio>
#include <string>
#include "../src/display.h"
#include "../kmod/vga_gomoku.h"

int main(int argc, char **argv)
{
	if(argc<2){
		printf("Usage ./tcp_main {0|1}\n");
		printf("0 - run as server\n");
		printf("1 - run as client\n");
	}
	if(atoi(argv[1])){ // Server
		// Usage - server:
		// 1. server.Create()
		// 2. server.WaitForPlayer()
		// 3. server.StartGame()
		// 4. use CheckGameResult() to see if game ends
		// 5. use MakeMove to place a piece
		GMKServer server;
		GMKDisplay display(VGA_DRIVER_FILENAME);
		if(!display.open_display())
			return -1;
		display.clear_board();
		server.SetDisplay(&display);
		if(!server.Create())
			return -1;
		if(!server.WaitForPlayer()){
			printf("Wait for player failed.\n");
			return -1;
		}
		while(true){
			server.StartGame();
			while(server.CheckGameResult()<0){
				int x,y;
				std::cin.clear();
				std::cin >> x >> y;
				server.MakeMove(x, y);
				display.update_piece_info(x-1,y-1,2,1);
				display.update_select(x-1,y-1);
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
	else{ // Client
		// Usage - client:
		// 1. client.Connect(ip)
		// 2. use CheckGameResult() to see if game ends
		// 3. use MakeMove to place a piece
		GMKClient client;
		if(!client.Connect("128.59.65.82"))
			return -1;
		while(true){
			while(client.CheckGameResult()<0){
				int x,y;
				std::cin.clear();
				fflush(stdin);
				std::cin >> x >> y;
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
}
