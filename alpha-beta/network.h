#include "../src/tcp.h"

#define LOBBY_PORT 12345

string getLocalIP();
string getIPfromMsg(string msg);
long long getCurTimeStamp();
long long getTimeFromStamp(string msg);
void sendConfirm(long long otherTime);
void broadcastPresence(int &role, string &myTimestamp, bool &gameStart, bool &noPlayersFound);
void listenForBroadcast(int &role, string &myTimestamp, bool &gameStart, string &server_ip, bool &noPlayersFound);