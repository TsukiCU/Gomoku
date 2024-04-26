#include "../src/tcp.h"
#include <cstdio>
#include <string>

#define PORT 12345
using namespace std;

long long getCurTimeStamp() {
    // Get the current timestamp in milliseconds.
    auto time = chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(since_epoch).count();
}

void broadcastPresence(int &role, string &myTimestamp) {
    int sockfd;
    struct sockaddr_in broadcastAddr;
    char sendString[64];
    long long myTime = getCurTimeStamp();
    printf("Searching for opponents\n");
    sprintf(sendString, "Presence %lld", myTime);
    myTimestamp = sendString;  // Save my timestamp for comparison

    int broadcastPermission = 1;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        return;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        return;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcastAddr.sin_port = htons(12345);

    sendto(sockfd, sendString, strlen(sendString), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
    close(sockfd);
}

// Listen for UDP broadcasts
void listenForBroadcast(int &role, string &myTimestamp) {
    int sockfd;
    struct sockaddr_in myAddr;
    char recvString[100];
    socklen_t addrLen = sizeof(myAddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        return;
    }

    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(12345);

    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return;
    }

    while (true) {
        int recvStringLen = recvfrom(sockfd, recvString, 100, 0, (struct sockaddr *)&myAddr, &addrLen);
        if (recvStringLen < 0) {
            perror("recvfrom failed");
            continue;
        }
        recvString[recvStringLen] = '\0';

        long long otherTime;
        sscanf(recvString, "Presence %lld", &otherTime);

        // Compare timestamps to decide role
        if (strcmp(recvString, myTimestamp.c_str()) != 0) {  // Check if it's not my own message
            if (otherTime < getCurTimeStamp()) {
                role = 1;  // This machine is client
                break;
            }
        }
    }
    close(sockfd);
}

int main()
{
    // UDP broadcast logic to find opponent in the local network.
    int role = -1;  // -1 for unknown, 0 for server, 1 for client;
    string myTimestamp;

    thread broadcaster(broadcastPresence, ref(role), ref(myTimestamp));
    thread listener(listenForBroadcast, ref(role), ref(myTimestamp));

    broadcaster.join();
    listener.join();

    // If received broadcast from others who started later than us, we are the server.
    // Otherwise, we are the client.

    if (!role) {
        printf("Running as server\n");
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
        printf("Running as client\n");
		GMKClient client;
		if(!client.Connect("10.206.99.5"))
			return -1;
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
        fprintf(stderr, "Unknown role\n");
        return 1;
    }
}