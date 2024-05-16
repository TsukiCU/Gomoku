#include "tcp.h"
#include "input.h"
#include "players.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <vector>
#include <fcntl.h>

using namespace std;

bool GMKNetBase::send_player_info(const PlayerInfo &info)
{
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_PLAYER_INFO;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	return true;
}

void GMKNetBase::reset_board()
{
	game_->state=2;
	game_->resetGame();
}

void GMKNetBase::create_receive_thread()
{
	recv_thread_=std::thread(&GMKNetBase::receive_thread_func,this);
}

void GMKNetBase::receive_thread_func()
{
	GMKNetMessage msg;
	printf("TCP Message handling thread created!\n");
	struct pollfd fds[1];
	fds[0].fd = remote_fd_;
	fds[0].events = POLLIN;
	int ret;
	while((ret=poll(fds,1, 1000))>=0){
		if(!ret)
			continue;
		bzero(&msg, sizeof(msg));
		if(read(remote_fd_,&msg,sizeof(msg))<0)
			break;
		// printf("Message received. %x %d %02x %02x\n",msg.magic,msg.type,msg.msg[0],msg.msg[1]);
		if(msg.magic!=0x474D4B4D){
			printf("Message magic invalid, close connection.\n");
			close(remote_fd_);
			break;
		}
		handle_message(msg);
	}
	printf("connection closed!\n");
	connected_ = false;
	// TODO: Do some work after thread exits
	if(handler_){
		InputEvent event;
		event.type = NONE;
		handler_->handle_input_press(event);
	}
	receive_thread_callback();
	return;
}

bool GMKNetBase::create_udp_socket()
{
    struct sockaddr_in anyAddr;
	int reuse_port_val = 0;
	int result = 1;
	if ((udp_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket");
		udp_fd_=-1;
        return false;
    }
	if (fcntl(udp_fd_, F_SETFL, O_NONBLOCK) < 0) {
        perror("server create fcntl");
    }
    if (setsockopt(udp_fd_, SOL_SOCKET, SO_REUSEPORT, &reuse_port_val, sizeof(reuse_port_val)) < 0) {
        perror("UDP setsockopt(SO_REUSEPORT) failed");
        return false;
    }

    memset(&anyAddr, 0, sizeof(anyAddr));
    anyAddr.sin_family = AF_INET;
    anyAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    anyAddr.sin_port = htons(GMK_UDP_PORT);
	udp_port_ = GMK_UDP_PORT;

	for(int i=0;i<GMK_SERVER_PORT_MAX_OFFSET;++i){
		result = bind(udp_fd_, (struct sockaddr*)&anyAddr, sizeof(anyAddr));
		if(!result)
			break;
		anyAddr.sin_port = htons(GMK_UDP_PORT+i);
		udp_port_ = GMK_UDP_PORT + i;
	}
    if (result != 0) { 
        perror("UDP bind address");
		udp_fd_=-1;
        return false;
    } 

	printf("UDP Socket created on port %u!\n",udp_port_);

	// Create UDP Receive thread
	udp_recv_thread_=std::thread(&GMKNetBase::udp_receive_thread_func,this);
	return true;
}

void GMKNetBase::udp_receive_thread_func()
{
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(srcAddr);
	GMKNetMessage msg;
	int ret;

	printf("UDP Message handling thread created!\n");

	memset(&srcAddr, 0, sizeof(srcAddr));
	bzero(&msg, sizeof(msg));
	while(true){
		ret=recvfrom(udp_fd_, 
					   &msg, 
						 sizeof(GMKNetMessage),
					 0,
					  (struct sockaddr *)&srcAddr,
				  &addrLen);
		if(ret<0){
			if (errno == EWOULDBLOCK || errno == EAGAIN){
				usleep(1000); // Sleep briefly to prevent high CPU usage
                continue;
			}
			break;
		}
		// Drop non GMK message
		if(ret!=sizeof(GMKNetMessage))
			continue;

		if(msg.magic==0x474D4B4D && IS_GMK_UDP_MSG(msg.type)){
			printf("UDP message received type %d.\n",msg.type);
			// Copy source Address, used for later reply.
			memcpy(msg.msg, &srcAddr, addrLen);
			handle_message(msg);
		}
		memset(&srcAddr, 0, sizeof(srcAddr));
		bzero(&msg, sizeof(msg));
	}

	close(udp_fd_);
	printf("UDP socket closed!\n");
	if(ret<0){
		perror("UDP Recv:");
	}
	// TODO: Do some work after thread exits
	receive_thread_callback();
	return;
}

void GMKNetBase::start_local_game()
{
	game_->resetGame();
	printf("Game started! You take %s piece!\n",
			local_player_.black?"Black":"White"
	);
	piece_count_=0;
	game_->state=0;
	game_->current_player=1;
	game_->displayBoard();
}

bool GMKNetBase::is_players_turn(const Player &player)
{
	// Not player's turn
	// It is players's turn when:
	// 1. current number of pieces is even and player holds black pieces.
	// 2. current number of pieces is odd and player holds white pieces.
	// That is (piece_count_%2) XOR local_player_.black
	if(!(piece_count_%2^player.black)){
		printf("It's not your turn.\n");
		return false;
	}
	return true;
}

bool GMKNetBase::is_move_valid(const GMKMoveInfo &info, const Player &player)
{
	// Game not started
	if(game_->state>0){
		printf("Game not started.\n");
		return false;
	}

	if(!is_players_turn(player))
		return false;

	if(!game_->valid_move(info.x,info.y))
		return false;
	return true;
}

bool GMKNetBase::make_move(int x, int y)
{
	GMKMoveInfo info;
	info.idx=piece_count_;
	info.x=x;
	info.y=y;

	if(!is_move_valid(info, local_player_))
		return false;

	// Local player takes move
	local_player_.makeMove(make_pair(x, y));
	++piece_count_;
	// Send move message
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_MOVE_INFO;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	
	printf("%d:%s (You) take a move at (%d,%d)\n", piece_count_,
		   local_player_.black?"Black":"White",
		   x,y
	);
	game_->displayBoard();
	return true;
}

bool GMKNetBase::regret_move()
{
	// Only allow local player to regret when it's player's turn
	if(!is_players_turn(local_player_))
		return false;
	// Local player regrets
	game_->regret_move();
	// Fall back 2 moves
	piece_count_-=2;

	// Send move message
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_MOVE_REGRET;
	write(remote_fd_, &msg, sizeof(msg));
	
	printf("%d:%s (You) regret a move!\n", piece_count_,
		   local_player_.black?"Black":"White"
	);
	game_->displayBoard();
	return true;
}

bool GMKNetBase::resign()
{
	// local player resigns
	local_player_.resign();

	// Send move message
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_GAME_RESIGN;
	write(remote_fd_, &msg, sizeof(msg));

	printf("%d:%s (You) resigned the game!\n", piece_count_,
		   local_player_.black?"Black":"White"
	);
	game_->displayBoard();
	return true;
}

int GMKNetBase::check_game_result()
{
	if(!connected_)
		return 3;
	// Game not started/ended.
	if(game_->state!=1)
		return -1;
	return local_player_.black^(game_->winner==2);
}

void GMKNetBase::handle_remote_move(const GMKMoveInfo &info)
{
	if(!is_move_valid(info, remote_player_)){
		// TODO:Remote move not valid
		return ;
	}
	// Remote player takes move
	remote_player_.makeMove(make_pair(info.x, info.y));
	++piece_count_;
	printf("%d:%s takes a move at (%d,%d)\n", piece_count_,
		remote_player_.black?"Black":"White",
		info.x,info.y
	);
	game_->displayBoard();

	if(handler_){
		InputEvent event;
		event.type = NONE;
		handler_->handle_input_press(event);
	}
}

void GMKNetBase::handle_remote_regret()
{
	if(!is_players_turn(remote_player_)){
		return ;
	}
	if(remote_player_.regretMove()){
	// if return value is not 0, an error occurs
		return;
	}
	piece_count_-=2;
	printf("%d:%s regrets a move!\n", piece_count_,
		   remote_player_.black?"Black":"White"
	);
	game_->displayBoard();
	if(handler_){
		InputEvent event;
		event.type = NONE;
		handler_->handle_input_press(event);
	}
}

void GMKNetBase::handle_remote_resign()
{
	// TODO: NOTIFY MAIN THREAD
	remote_player_.resign();
	printf("%d:%s resigns!\n", piece_count_,
		   remote_player_.black?"Black":"White"
	);
	game_->displayBoard();
	if(handler_){
		InputEvent event;
		event.type = NONE;
		handler_->handle_input_press(event);
	}
}

void GMKServer::handle_message(const GMKNetMessage &msg)
{
	if(msg.type==GMK_MSG_PLAYER_INFO){// Player info
		PlayerInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		remote_player_.info=info;
		is_player_joined_ = true;
	}
	else if(msg.type==GMK_MSG_MOVE_INFO){ // Move info
		GMKMoveInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		handle_remote_move(info);
	}
	else if(msg.type==GMK_MSG_MOVE_REGRET){ // Remote player regrets
		handle_remote_regret();
	}
	else if(msg.type==GMK_MSG_GAME_RESIGN){ // Remote player resigns
		handle_remote_resign();
	}
	else if(msg.type==GMK_MSG_UDP_DISCOVER){ // UDP Server discover
		GMKNetMessage reply;
		sockaddr_in addr;

		// Read reply address
		memcpy(&addr, msg.msg, sizeof(struct sockaddr_in));
		//addr.sin_port = GMK_UDP_PORT;
		// Send server status
		// Playing or ready to play
		reply.type = connected_?GMK_MSG_UDP_BUSY:GMK_MSG_UDP_READY;
		// Copy Server Port
		memcpy(reply.msg+GMK_UDP_DATA_OFFSET, &server_port_, sizeof(uint16_t));
		sendto(udp_fd_, &reply, sizeof(reply),0,(struct sockaddr *)&addr, sizeof(addr));
		printf("Sending server status to %s:%u\n",inet_ntoa(addr.sin_addr),addr.sin_port);
	}
}


bool GMKServer::create()
{
    // socket create and verification 
    local_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
	if (local_fd_ == -1) { 
        printf("Gomoku Server: Socket creation failed...\n"); 
        return false;
    } 
	if (fcntl(local_fd_, F_SETFL, O_NONBLOCK) < 0) {
        perror("server create fcntl");
    }
    printf("Gomoku Server: Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(GMK_SERVER_PORT);
	server_port_ = GMK_SERVER_PORT;

    // Binding newly created socket to given IP and verification 
	int result = 1;
	int i=0;
	for(;i<GMK_SERVER_PORT_MAX_OFFSET;++i){
		result = bind(local_fd_, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if(!result)
			break;
		servaddr.sin_port = htons(GMK_SERVER_PORT+i);
		server_port_ = GMK_SERVER_PORT+i;
	}
    if (result != 0) { 
        printf("Gomoku Server: Socket bind failed...\n"); 
        return false;
    } 
    printf("Gomoku Server: Socket successfully binded..\n");  

    // Now server is ready to listen and verification 
    if ((listen(local_fd_, 5)) != 0) { 
        printf("Gomoku Server: Listen failed...\n"); 
        return false;
    } 
	printf("Gomoku Server: Server listening at port %d..\n",GMK_SERVER_PORT+i);
	create_udp_socket();
	return true;
}
GMKServer::~GMKServer()
{
	printf("close(remote_fd_);\n");
	if(remote_fd_>0)
		close(remote_fd_);
	printf("close(local_fd_);\n");
	if(local_fd_>0)
		close(local_fd_);
	printf("close(udp_fd_);\n");
	if(udp_fd_>0)
		close(udp_fd_);
	printf("udp_recv_thread_.join();\n");
	if(udp_recv_thread_.joinable())
		udp_recv_thread_.join();
	printf("recv_thread_.join();\n");
	if(recv_thread_.joinable())
		recv_thread_.join();
}

bool GMKServer::wait_for_player()
{
	struct sockaddr_in client;
	uint len;
	len = sizeof(client);

	if(recv_thread_.joinable())
		recv_thread_.join();
	printf("Gomoku Server: Waiting for player to join...\n");
	if(cancel_)
		*cancel_=false;
	// Wait a player to join
	while(true){
		// Accept the data packet from client and verification 
		remote_fd_ = accept(local_fd_, (struct sockaddr*)&client, &len); 
		if (remote_fd_ < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				if(cancel_)
					if(*cancel_){
						printf("Gomoku Server: Wait for player cancelled!\n");
						return false;
					}
                // No pending connections, continue to next iteration
                usleep(1000); // Sleep briefly to prevent high CPU usage
                continue;
            }
			printf("Gomoku Server: Server accept failed...\n"); 
			return false;
		} 
		printf("Gomoku Server: Server accept the client...\n");

		// Create message handling thread
		create_receive_thread();
		request_player_info();
		/* 
		* After connection established,
		* the gomoku client must send player info in 5s,
		* or the connection will be closed.
		*/
		if(!wait_for_player_info()){
			printf("Gomoku Server: Not a valid player.\n");
			close(remote_fd_);
			recv_thread_.join();
		}
		else
			break;
	}
	connected_ = true;
	return true;
}

void GMKServer::request_player_info()
{
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_REQ_PLAYER_INFO;
	write(remote_fd_, &msg, sizeof(msg));
}

bool GMKServer::wait_for_player_info()
{
	printf("Gomoku Server: Waiting for remote player info...\n");
	if(cancel_)
		*cancel_=false;
	for(int i=0;i<100;++i){
		usleep(50000);
		if(cancel_)
			if(*cancel_)
				break;
		if(is_player_joined_){
			printf("Gomoku Server: Player info received!\n");
			// Send server player info
			send_player_info(local_player_.info);
			// Send server game info
			send_game_info();
			return true;
		}
	}
	printf("Gomoku Server: Player info timeout!\n");
	return false;
}

bool GMKServer::send_game_info()
{
	GMKNetMessage msg;
	bzero(&msg, sizeof(msg));
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_GAME_INFO;

	GMKGameInfo info;
	info.board_size = game_->board_size;
	info.win_length = game_->WIN_LENGTH;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	return true;
}

bool GMKServer::start_game()
{
	bool local_first = assign_pieces();

	// Send start signal
	GMKNetMessage msg;
	bzero(&msg, sizeof(msg));
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_GAME_START;
	msg.msg[0]=	local_player_.black;
	msg.msg[1] = remote_player_.black;
	write(remote_fd_, &msg, sizeof(msg));

	// Wait for remote acknowledge?
	start_local_game();
	return local_first;
}

bool GMKServer::assign_pieces()
{
	srand(time(NULL));
	local_player_.black=((rand()%4)>=2);
	remote_player_.black=!local_player_.black;
	return local_player_.black;
}

bool GMKClient::connect_to(const char *ip)
{
	GMKServerInfo info;
	info.address = ip;
	info.port = GMK_SERVER_PORT;
	return connect_to(info);
}

bool GMKClient::connect_to(const GMKServerInfo &info)
{
	struct sockaddr_in servaddr;
	// socket create and verification
    remote_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (remote_fd_ == -1) {
        printf("Gomoku Client: Socket creation failed...\n");
        return false;
    }
	printf("Gomoku Client: Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(info.address.c_str());
    servaddr.sin_port = htons(info.port);
 
    // connect the client socket to server socket
    if (connect(remote_fd_, (struct sockaddr*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("Gomoku Client: connect_toion with the server failed...\n");
        return false;
    }
    printf("Gomoku Client: connect_toed to the server..\n");
	connected_ = true;
	create_receive_thread();
	return true;
}

void GMKClient::reset_server_list()
{
	for(auto server: server_list_){
		server.status=0;
	}
}

bool GMKClient::send_server_discover()
{
	struct sockaddr_in broadcastAddr;
	int broadcastPermission = 1;
	struct GMKNetMessage msg;

	if(udp_fd_<0){
		if(!create_udp_socket())
			return false;
	}

    if (setsockopt(udp_fd_, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("UDP setsockopt");
        close(udp_fd_);
        return false;
    }

    // Reset server list
	reset_server_list();
	
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));
	broadcastAddr.sin_family = AF_INET;
	broadcastAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	bzero(&msg, sizeof(msg));
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_UDP_DISCOVER;
	for(int i=0;i<GMK_SERVER_PORT_MAX_OFFSET;++i){
		broadcastAddr.sin_port = htons(GMK_UDP_PORT+i);
		sendto(udp_fd_, &msg, sizeof(GMKNetMessage), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
	}

	return true;
}

void GMKClient::update_server_list(const GMKServerInfo & info)
{
	for(auto element: server_list_){
		if(element.address==info.address&&element.port==info.port){
			element.status=info.status;
			return;
		}
	}
	server_list_.push_back(info);

	// TODO: probably call some notify function here.
}


void GMKClient::handle_message(const GMKNetMessage &msg)
{
	if(msg.type==GMK_MSG_GAME_INFO){// Game info
		GMKGameInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		game_->board_size=info.board_size;
		game_->WIN_LENGTH=info.win_length;
		printf("Game info received! Waiting game to start...\n");
	}
	else if(msg.type==GMK_MSG_PLAYER_INFO){
		PlayerInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		remote_player_.info=info;
	}
	else if(msg.type==GMK_MSG_GAME_START){// Game start signal
		remote_player_.black=msg.msg[0];
		local_player_.black=msg.msg[1];
		// FIXME: Move display logic
		if(game_->display){
			game_->display->set_player_piece(local_player_.black);
			game_->display->set_turn_mark(local_player_.black);
		}
		start_local_game();
	}
	else if(msg.type==GMK_MSG_MOVE_INFO){ // Move info
		GMKMoveInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		handle_remote_move(info);
	}
	else if(msg.type==GMK_MSG_MOVE_REGRET){ // Remote player regrets
		handle_remote_regret();
	}
	else if(msg.type==GMK_MSG_GAME_RESIGN){ // Remote player resigns
		handle_remote_resign();
	}
	else if(msg.type==GMK_MSG_REQ_PLAYER_INFO){ // Server requests player info
		send_player_info(local_player_.info);
		printf("Server requests player info\n");
	}
	else if(IS_GMK_UDP_MSG(msg.type)){ // Handle UDP message
		// Read server port
		// Read Server Port
		GMKServerInfo info;
		sockaddr_in addr;

		// Read server address
		memcpy(&addr, msg.msg, sizeof(struct sockaddr_in));
		info.address = inet_ntoa(addr.sin_addr);
		// Read server port
		memcpy(&info.port, msg.msg+GMK_UDP_DATA_OFFSET, sizeof(uint16_t));
		
		switch (msg.type) {
		case GMK_MSG_UDP_READY: info.status=2; break;
		case GMK_MSG_UDP_BUSY: info.status=1;  break;
		default: return;	//Drop Other messages
		}
		update_server_list(info);
	}
}
GMKClient::~GMKClient()
{
	if(remote_fd_>0)
		close(remote_fd_);
	if(udp_fd_>0)
		close(udp_fd_);
	if(udp_recv_thread_.joinable())
		udp_recv_thread_.join();
	if(recv_thread_.joinable())
		recv_thread_.join();
}

bool GMKClient::wait_for_scan()
{
	if(cancel_)
		*cancel_ = false;
	for(size_t i=0;i<50;++i){
		if(cancel_)
			if(*cancel_)
				break;
		usleep(100000);
		for(auto server:get_server_list()){
			if(server.status==2){
				return true;
			}
		}
	}
	if(server_list_.empty())
		return false;
	return true;
}