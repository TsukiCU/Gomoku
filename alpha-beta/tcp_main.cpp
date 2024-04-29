#include "../src/tcp.h"
#include <cstdio>
#include <string>
#include <unistd.h>

int main(int argc, char **argv)
{
	if(argc<2){
		printf("Usage ./tcp_main {0|1}\n");
		printf("0 - run as client\n");
		printf("1 - run as server\n");
		return 1;
	}
	if(atoi(argv[1])){ // Server
		// Usage - server:
		// 1. server.Create()
		// 2. server.WaitForPlayer()
		// 3. server.StartGame()
		// 4. use CheckGameResult() to see if game ends
		// 5. use MakeMove to place a piece
		GMKServer server;
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
		client.SendServerDiscover();
		string null;
		string server_status[3] = {"Down","Gaming","Ready"};
		GMKServerInfo info;
		bool found=false;
		printf("Discovering server...\n");
		sleep(3);
		if(client.GetServerList().empty()){
			printf("No server discovered!\n");
			return -1;
		}
		printf("Server List\n%-15s\t%-4s\t%s\n","IP address","Port","Status");
		for(auto server:client.GetServerList()){
			printf("%s\t%-4u\t%-6s\n",server.address.c_str(),server.port,server_status[server.status].c_str());
			if(server.status==2){
				info = server;
				found = true;
			}
		}
		if(!found){
			printf("No available server found!\n");
			return -1;
		}
		printf("Connecting %s:%u\n",info.address.c_str(),info.port);
		if(!client.Connect(info))
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