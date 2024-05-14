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
#include "input.h"

#define GMK_UDP_PORT 33261
#define GMK_SERVER_PORT 18253
#define GMK_SERVER_PORT_MAX_OFFSET 100
#define GMK_MSG_PLAYER_INFO 1
#define GMK_MSG_GAME_INFO 2
#define GMK_MSG_GAME_START 3
#define GMK_MSG_MOVE_INFO 4
#define GMK_MSG_REQ_PLAYER_INFO 5
#define GMK_MSG_MOVE_REGRET 6
#define GMK_MSG_GAME_RESIGN 7

#define GMK_MSG_UDP_DISCOVER 0xF1
#define GMK_MSG_UDP_READY	 0xF2
#define GMK_MSG_UDP_BUSY	 0xF3
#define GMK_UDP_DATA_OFFSET (sizeof(struct sockaddr_in))
#define IS_GMK_UDP_MSG(x)	((x)&0xF0)

struct GMKNetMessage{
	// Gomoku message magic 0x474D4B4D
	const uint32_t magic = 0x474D4B4D;
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

class GMKNetBase : public InputEventHandler{
public:
	GMKNetBase(Gomoku *game) :
	game_(game),
	local_player_(game_, 0),
	remote_player_(game_, 1)
	{
		game_->state=2;
		// Set game mode to PVP
		game_->mode=0;
	}
	bool make_move(int x, int y);
	bool regret_move();
	bool resgin();
	int check_game_result();
	void reset_board();
	int start_game_loop();

protected:
	bool send_player_info(const PlayerInfo &info);
	
	void create_receive_thread();
	virtual void handle_message(const GMKNetMessage &msg) {};
	virtual void receive_thread_callback(){};
	// TCP thread functions
	void receive_thread_func();
	// UDP thread functions
	bool create_udp_socket();
	void udp_receive_thread_func();

	void start_local_game();
	bool is_move_valid(const GMKMoveInfo &info, const Player &player);
	bool is_players_turn(const Player &player);

	void handle_remote_move(const GMKMoveInfo &info);
	void handle_remote_regret();
	void handle_remote_resign();

	int remote_fd_=-1, udp_fd_=-1;
	std::thread recv_thread_, udp_recv_thread_;
	Gomoku *game_;
	Player local_player_, remote_player_;
	uint32_t piece_count_;
	uint16_t udp_port_;
	bool loop_stopped_ = true;

	bool connected_ = false;
};

class GMKServer : public GMKNetBase{
public:
	GMKServer(Gomoku *game) : GMKNetBase(game) {}
	// create Server
	bool create();
	// Wait for a player to join the game.
	bool wait_for_player();
	// Send information of this game, board size, etc.
	bool send_game_info();
	// Send game start message
	bool start_game();
	// bool Close();
protected:
	// Wait for the player to send player info.
	bool wait_for_player_info();
	void request_player_info();
	void handle_message(const GMKNetMessage &msg) override;
	//void receive_thread_callback() override;
	void assign_pieces();
	int local_fd_;
	uint16_t server_port_;
	bool is_player_joined_ = false;
};

class GMKClient : public GMKNetBase{
public:
	GMKClient(Gomoku *game) : GMKNetBase(game) {}
	bool connect_to(const char *ip);
	bool connect_to(const GMKServerInfo &info);
	//bool Disconnect();
	// UDP Broadcast;
	bool send_server_discover();
	std::vector<GMKServerInfo> get_server_list() const {return server_list_;}
protected:
	void handle_message(const GMKNetMessage &msg) override;
	void update_server_list(const GMKServerInfo &info);
	void reset_server_list();

	std::vector<GMKServerInfo> server_list_;
	//void receive_thread_callback() override;
};