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
#include "display.h"

#define GMK_SERVER_PORT 18253
#define GMK_SERVER_PORT_MAX_OFFSET 100
#define GMK_MSG_PLAYER_INFO 1
#define GMK_MSG_GAME_INFO 2
#define GMK_MSG_GAME_START 3
#define GMK_MSG_MOVE_INFO 4
#define GMK_MSG_REQ_PLAYER_INFO 5

struct GMKTCPMessage{
	uint32_t magic;  // Gomoku message magic 0x474D4B4D
	u_char type;
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

class GMKTCPBase{
public:
	GMKTCPBase() :
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
	void ReceiveThreadFunc();
	virtual void HandleMessage(const GMKTCPMessage &msg) {};
	virtual void ReceiveThreadCallback(){};
	void StartLocalGame();
	bool IsMoveValid(const GMKMoveInfo &info, const Player &player);
	void HandleRemoteMove(const GMKMoveInfo &info);
	int remote_fd_;
	std::thread recv_thread_;
	Gomoku game_;
	Player local_player_, remote_player_;
	uint32_t piece_count;
	GMKDisplay *display_=nullptr;

	bool connected_ = false;
};

class GMKServer : public GMKTCPBase{
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
	void HandleMessage(const GMKTCPMessage &msg) override;
	//void ReceiveThreadCallback() override;
	void AssignPieces();
	int local_fd_;
	bool is_player_joined_ = false;
};

class GMKClient : public GMKTCPBase{
public:
	bool Connect(const char *ip);
	//bool Disconnect();
	// UDP Broadcast;
	//bool FindServer();
protected:
	void HandleMessage(const GMKTCPMessage &msg) override;
	//void ReceiveThreadCallback() override;
};