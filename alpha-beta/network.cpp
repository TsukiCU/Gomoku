#include "../src/tcp.h"
#include <cstdio>
#include <string>

#define LOBBY_PORT 12345
using namespace std;

// get local ip. pretty much like socket.gethostbyname(socket.gethostname()) in Python.
// This is hacky and hurts portability deeply, but it does work(at least on my machine)
string getLocalIP() {
    char buffer[256];
    string ip = "";
    FILE *fp = popen("hostname -I", "r");
    if (fp == NULL) {
        perror("popen failed");
        return ip;
    }
    fgets(buffer, sizeof(buffer), fp);
    pclose(fp);
    ip = buffer;
    return ip.substr(0, ip.length() - 1);
}

string getIPfromMsg(string msg) {
    // Message format: "CONFIRM: <Timestamp> | Server_IP: <Server Ip>"
    return msg.substr(msg.find("Server_IP: ") + 11);
}

long long getCurTimeStamp() {
    // Get the current timestamp in milliseconds.
    auto time = chrono::system_clock::now();
    auto since_epoch = time.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(since_epoch).count();
}

long long getTimeFromStamp(string msg) {
    // Current message format: "TIMESTAMP: <Created time>"
    return stoll(msg.substr(11));
}

void broadcastPresence(int &role, string &myTimestamp, bool &gameStart) {
    int sockfd;
    struct sockaddr_in broadcastAddr;
    char sendString[64];
    long long myTime = getCurTimeStamp();
    sprintf(sendString, "TIMESTAMP: %lld", myTime); // Timestamp to be sent
    myTimestamp = sendString;  // Save for comparison

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
    broadcastAddr.sin_port = htons(LOBBY_PORT);

    //sendto(sockfd, sendString, strlen(sendString), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
    while (!gameStart) {
        sendto(sockfd, sendString, strlen(sendString), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
        //sleep(1);
        this_thread::sleep_for(chrono::seconds(1));
    }
    close(sockfd);
}

void sendConfirm(long long otherTime) {
    // CONFIRM message format: "CONFIRM: <Timestamp> | <Server Ip>"
    int sockfd;
    struct sockaddr_in broadcastAddr;
    char sendString[64];
    string serverIp = getLocalIP();
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
    broadcastAddr.sin_port = htons(LOBBY_PORT);

    sprintf(sendString, "CONFIRM: %lld | Server_IP: %s", otherTime, serverIp.c_str());
    sendto(sockfd, sendString, strlen(sendString), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));

    close(sockfd);
}

// Listen for UDP broadcasts
void listenForBroadcast(int &role, string &myTimestamp, bool &gameStart, string &server_ip) {
    /*
     * Two types of messages can be received:
     *
     * 1. TIMESTAMP: <Created time>
     *      - TIMESTAMP message from others with their timestamps.
     * 
     * 2. CONFIRM: <Timestamp> | Server_IP: <Server Ip> 
     *      - CONFIRM message from server (who starts to broadcast before us), 
     *  indicating the game is ready to start and that we are the client. In
     *  this case, stop listening, connect to the server ip and start the game.
     */

    int sockfd;
    struct sockaddr_in myAddr;
    char recvString[100];
    long long myTime = getTimeFromStamp(myTimestamp);
    long long otherTime;
    socklen_t addrLen = sizeof(myAddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        return;
    }

    memset(&myAddr, 0, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myAddr.sin_port = htons(LOBBY_PORT);

    if (bind(sockfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        return;
    }
    while (!gameStart) {
        int recvStringLen = recvfrom(sockfd, recvString, 100, 0, (struct sockaddr *)&myAddr, &addrLen);
        if (recvStringLen < 0) {
            perror("recvfrom failed");
            continue;
        }
        recvString[recvStringLen] = '\0';

        // TIMESTAMP message
        if (strncmp(recvString, "TIMESTAMP: ", 11) == 0) {
            otherTime = getTimeFromStamp(recvString);

            // Compare timestamps to decide role
            if (otherTime > myTime) {
                //printf("Received: %s\n", recvString);
                role = 0;  // Server
                gameStart = true;
                sendConfirm(otherTime);
            } else if (otherTime < myTime) {
                // XXX: This is actually unreachable. Because once the server starts,
                // we will no longer receive broadcast from it. So I put a XXX here.
                role = 1;  // Client
                gameStart = true;
            } else
                // Almost impossible that two devices have the same timestamp.
                // So consider this only happens when receiving broadcast sent
                // by ourselves. This could happen in UDP, simply ignore it.
                continue;
        }

        // CONFIRM message
        else if (strncmp(recvString, "CONFIRM", 7) == 0) {
            // Confirm message format: "CONFIRM: <Timestamp> | Server_IP: <Server Ip>"
            long long time = stoll(string(recvString).substr(9));
            if (time == myTime) {
                role = 1;  // Client
                gameStart = true;
            }
            // get the ip of the server and store it in server_ip
            server_ip = getIPfromMsg(recvString);
        }
    }

    close(sockfd);
}

int main()
{
    int role = -1;  // -1 for unknown, 0 for server, 1 for client;
    bool gameStart = false; // If game starts, stop listening and broadcasting threads.
    string server_ip = "";
    string myTimestamp =  "TIMESTAMP: " + to_string(getCurTimeStamp());

    thread broadcaster(broadcastPresence, ref(role), ref(myTimestamp), ref(gameStart));
    thread listener(listenForBroadcast, ref(role), ref(myTimestamp), ref(gameStart), ref(server_ip));

    broadcaster.join();
    listener.join();

    // If received broadcast from others who started later than us, we are the server.
    // Otherwise, we are the client.
    if (!role) {
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
		GMKClient client;
		if(!client.Connect(server_ip.c_str()))
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
        fprintf(stderr, "Roles Undetermined Error.\n");
        return 1;
    }
}