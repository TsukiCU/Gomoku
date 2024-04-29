#pragma once

#include "gomoku.h"
#include "players.h"
#include <cstdint>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include "display.h"

#define GMK_UDP_PORT 33261
#define GMK_SERVER_PORT 18253
#define GMK_SERVER_PORT_MAX_OFFSET 100
#define GMK_MSG_PLAYER_INFO 1
#define GMK_MSG_GAME_INFO 2
#define GMK_MSG_GAME_START 3
#define GMK_MSG_MOVE_INFO 4
#define GMK_MSG_REQ_PLAYER_INFO 5
#define GMK_MSG_UDP_DISCOVER 0xF1
#define GMK_MSG_UDP_READY	 0xF2
#define GMK_MSG_UDP_BUSY	 0xF3
#define GMK_UDP_DATA_OFFSET (sizeof(struct sockaddr_in))
#define IS_GMK_UDP_MSG(x)	((x)&0xF0)

struct GMKNetMessage{
	uint32_t magic;  // Gomoku message magic 0x474D4B4D
	u_char type;
	// For UDP Message, data should start from msg+sizeof(struct sockaddr_in).
	// Because the first several bytes are preserved for source IP.
	u_char msg[251];
};

struct GMKGameInfo{
	uint32_t board_size;
	int32_t win_length;
};

struct GMKMoveInfo{
	uint32_t idx;
	int x,y;
};

struct GMKServerInfo{
	string address;
	uint16_t port;
	// Server status
	// 0 - down
	// 1 - busy
	// 2 - ready
	uint16_t status;
};

class GMKNetBase{
public:
	GMKNetBase() :
	game_(0),
	local_player_(&game_, 0),
	remote_player_(&game_, 1)
	{
		game_.state=2;
	}
	bool MakeMove(int x, int y);
	int CheckGameResult();
	void ResetBoard();
	void SetDisplay(GMKDisplay *display) {display_=display;}
protected:
	bool SendPlayerInfo(const PlayerInfo &info);
	
	void CreateReceiveThread();
	virtual void HandleMessage(const GMKNetMessage &msg) {};
	virtual void ReceiveThreadCallback(){};
	// TCP thread functions
	void ReceiveThreadFunc();
	// UDP thread functions
	bool CreateUdpSocket();
	void UdpReceiveThreadFunc();

	void StartLocalGame();
	bool IsMoveValid(const GMKMoveInfo &info, const Player &player);
	void HandleRemoteMove(const GMKMoveInfo &info);
	int remote_fd_=-1, udp_fd_=-1;
	std::thread recv_thread_, udp_recv_thread_;
	Gomoku game_;
	Player local_player_, remote_player_;
	uint32_t piece_count_;
	GMKDisplay *display_=nullptr;
	uint16_t udp_port_;

	bool connected_ = false;
};

class GMKServer : public GMKNetBase{
public:
	// Create Server
	bool Create();
	// Wait for a player to join the game.
	bool WaitForPlayer();
	// Send information of this game, board size, etc.
	bool SendGameInfo();
	// Send game start message
	bool StartGame();
	// bool Close();
protected:
	// Wait for the player to send player info.
	bool WaitForPlayerInfo();
	void RequestPlayerInfo();
	void HandleMessage(const GMKNetMessage &msg) override;
	//void ReceiveThreadCallback() override;
	void AssignPieces();
	int local_fd_;
	uint16_t server_port_;
	bool is_player_joined_ = false;
};

class GMKClient : public GMKNetBase{
public:
	bool Connect(const char *ip);
	bool Connect(const GMKServerInfo &info);
	//bool Disconnect();
	// UDP Broadcast;
	bool SendServerDiscover();
	std::vector<GMKServerInfo> GetServerList() const {return server_list_;}
protected:
	void HandleMessage(const GMKNetMessage &msg) override;
	void UpdateServerList(const GMKServerInfo &info);
	void ResetServerList();

	std::vector<GMKServerInfo> server_list_;
	//void ReceiveThreadCallback() override;
};