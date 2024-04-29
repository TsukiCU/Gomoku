#include "tcp.h"
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
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <vector>

using namespace std;

bool GMKNetBase::SendPlayerInfo(const PlayerInfo &info)
{
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_PLAYER_INFO;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	return true;
}

void GMKNetBase::ResetBoard()
{
	game_.state=2;
	game_.clearBoard();
}

void GMKNetBase::CreateReceiveThread()
{
	recv_thread_=std::thread(&GMKNetBase::ReceiveThreadFunc,this);
}

void GMKNetBase::ReceiveThreadFunc()
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
		HandleMessage(msg);
	}
	printf("Connection closed!\n");
	connected_ = false;
	// TODO: Do some work after thread exits
	ReceiveThreadCallback();
	return;
}

bool GMKNetBase::CreateUdpSocket()
{
    struct sockaddr_in anyAddr;
	int reuse_port_val = 0;
	int result = 1;
	if ((udp_fd_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("UDP Socket");
		udp_fd_=-1;
        return false;
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
	udp_recv_thread_=std::thread(&GMKNetBase::UdpReceiveThreadFunc,this);
	return true;
}

void GMKNetBase::UdpReceiveThreadFunc()
{
    struct sockaddr_in srcAddr;
    socklen_t addrLen = sizeof(srcAddr);
	GMKNetMessage msg;
	int ret;

	printf("UDP Message handling thread created!\n");

	memset(&srcAddr, 0, sizeof(srcAddr));
	bzero(&msg, sizeof(msg));
	while((ret=recvfrom(udp_fd_, 
					   &msg, 
						 sizeof(GMKNetMessage),
					 0,
					  (struct sockaddr *)&srcAddr,
				  &addrLen))>=0){
		// Drop non GMK message
		if(ret!=sizeof(GMKNetMessage))
			continue;

		if(msg.magic==0x474D4B4D && IS_GMK_UDP_MSG(msg.type)){
			printf("UDP message received type %d.\n",msg.type);
			// Copy source Address, used for later reply.
			memcpy(msg.msg, &srcAddr, addrLen);
			HandleMessage(msg);
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
	ReceiveThreadCallback();
	return;
}

void GMKNetBase::StartLocalGame()
{
	game_.clearBoard();
	printf("Game started! You take %s piece!\n",
			local_player_.black?"Black":"White"
	);
	piece_count_=0;
	game_.state=0;
	game_.current_player=1;
	game_.displayBoard();
}

bool GMKNetBase::IsMoveValid(const GMKMoveInfo &info, const Player &player)
{
	// Game not started
	if(game_.state>0){
		printf("Game not started.\n");
		return false;
	}
	// Not player's turn
	// It is players's turn when:
	// 1. current number of pieces is even and player holds black pieces.
	// 2. current number of pieces is odd and player holds white pieces.
	// That is (piece_count_%2) XOR local_player_.black
	if(!(piece_count_%2^player.black)){
		printf("It's not your turn.\n");
		return false;
	}
	if(!game_.valid_move(info.x-1,info.y-1)){
		printf("Invalid move!\n");
		return false;
	}
	return true;
}

bool GMKNetBase::MakeMove(int x, int y)
{
	GMKMoveInfo info;
	info.idx=piece_count_;
	info.x=x;
	info.y=y;

	if(!IsMoveValid(info, local_player_))
		return false;

	// Local player takes move
	local_player_.makeMove(make_pair(x - 1, y - 1));
	++piece_count_;
	if(display_){
		display_->update_piece_info(x-1,y-1,local_player_.black?1:2,1);
		display_->update_select(x-1,y-1);
	}
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
	game_.displayBoard();
	return true;
}

int GMKNetBase::CheckGameResult()
{
	if(!connected_)
		return 3;
	// Game not started/ended.
	if(game_.state!=1)
		return -1;
	return local_player_.black^(game_.winner==2);
}

void GMKNetBase::HandleRemoteMove(const GMKMoveInfo &info)
{
	if(!IsMoveValid(info, remote_player_)){
		// TODO:Remote move not valid
	}
	// Remote player takes move
	remote_player_.makeMove(make_pair(info.x - 1, info.y - 1));
	++piece_count_;
	if(display_){
		printf("Remote move\n");
		display_->update_piece_info(info.x-1,info.y-1,remote_player_.black?1:2,1);
	}
	printf("%d:%s take a move at (%d,%d)\n", piece_count_,
		remote_player_.black?"Black":"White",
		info.x,info.y
	);
	game_.displayBoard();
}


void GMKServer::HandleMessage(const GMKNetMessage &msg)
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
		HandleRemoteMove(info);
	}
	else if(msg.type==GMK_MSG_UDP_DISCOVER){ // UDP Server discover
		GMKNetMessage reply;
		sockaddr_in addr;

		// Read reply address
		memcpy(&addr, msg.msg, sizeof(struct sockaddr_in));
		//addr.sin_port = GMK_UDP_PORT;
		reply.magic = 0x474D4B4D;
		// Send server status
		// Playing or ready to play
		reply.type = connected_?GMK_MSG_UDP_BUSY:GMK_MSG_UDP_READY;
		// Copy Server Port
		memcpy(reply.msg+GMK_UDP_DATA_OFFSET, &server_port_, sizeof(uint16_t));
		sendto(udp_fd_, &reply, sizeof(reply),0,(struct sockaddr *)&addr, sizeof(addr));
		printf("Sending server status to %s:%u\n",inet_ntoa(addr.sin_addr),addr.sin_port);
	}
}


bool GMKServer::Create()
{
    // socket create and verification 
    local_fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servaddr;
	if (local_fd_ == -1) { 
        printf("Gomoku Server: Socket creation failed...\n"); 
        return false;
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
	CreateUdpSocket();
	return true;
}

bool GMKServer::WaitForPlayer()
{
	struct sockaddr_in client;
	uint len;
	len = sizeof(client); 

	if(recv_thread_.joinable())
		recv_thread_.join();
	// Wait a player to join
	while(true){
		printf("Gomoku Server: Waiting for player to join...\n");
		// Accept the data packet from client and verification 
		remote_fd_ = accept(local_fd_, (struct sockaddr*)&client, &len); 
		if (remote_fd_ < 0) { 
			printf("Gomoku Server: Server accept failed...\n"); 
			return false;
		} 
		printf("Gomoku Server: Server accept the client...\n");

		// Create message handling thread
		CreateReceiveThread();
		RequestPlayerInfo();
		/* 
		* After connection established,
		* the gomoku client must send player info in 5s,
		* or the connection will be closed.
		*/
		if(!WaitForPlayerInfo()){
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

void GMKServer::RequestPlayerInfo()
{
	GMKNetMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_REQ_PLAYER_INFO;
	write(remote_fd_, &msg, sizeof(msg));
}

bool GMKServer::WaitForPlayerInfo()
{
	printf("Gomoku Server: Waiting for remote player info...\n");
	for(int i=0;i<100;++i){
		usleep(50000);
		if(is_player_joined_){
			printf("Gomoku Server: Player info received!\n");
			// Send server player info
			SendPlayerInfo(local_player_.info);
			// Send server game info
			SendGameInfo();
			return true;
		}
	}
	printf("Gomoku Server: Player info timeout!\n");
	return false;
}

bool GMKServer::SendGameInfo()
{
	GMKNetMessage msg;
	bzero(&msg, sizeof(msg));
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_GAME_INFO;

	GMKGameInfo info;
	info.board_size = game_.board_size;
	info.win_length = game_.WIN_LENGTH;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	return true;
}

bool GMKServer::StartGame()
{
	AssignPieces();

	// Send start signal
	GMKNetMessage msg;
	bzero(&msg, sizeof(msg));
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_GAME_START;
	msg.msg[0]=	local_player_.black;
	msg.msg[1] = remote_player_.black;
	write(remote_fd_, &msg, sizeof(msg));

	// Wait for remote acknowledge?
	StartLocalGame();
	return true;
}

void GMKServer::AssignPieces()
{
	srand(time(NULL));
	local_player_.black=((rand()%4)>=2);
	remote_player_.black=!local_player_.black;
}

bool GMKClient::Connect(const char *ip)
{
	GMKServerInfo info;
	info.address = ip;
	info.port = GMK_SERVER_PORT;
	return Connect(info);
}

bool GMKClient::Connect(const GMKServerInfo &info)
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
        printf("Gomoku Client: Connection with the server failed...\n");
        return false;
    }
    printf("Gomoku Client: Connected to the server..\n");
	connected_ = true;
	CreateReceiveThread();
	return true;
}

void GMKClient::ResetServerList()
{
	for(auto server: server_list_){
		server.status=0;
	}
}

bool GMKClient::SendServerDiscover()
{
	struct sockaddr_in broadcastAddr;
	int broadcastPermission = 1;
	struct GMKNetMessage msg;

	if(udp_fd_<0){
		if(!CreateUdpSocket())
			return false;
	}

    if (setsockopt(udp_fd_, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("UDP setsockopt");
        close(udp_fd_);
        return false;
    }

    // Reset server list
	ResetServerList();
	
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

void GMKClient::UpdateServerList(const GMKServerInfo & info)
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


void GMKClient::HandleMessage(const GMKNetMessage &msg)
{
	if(msg.type==GMK_MSG_GAME_INFO){// Game info
		GMKGameInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		game_.board_size=info.board_size;
		game_.WIN_LENGTH=info.win_length;
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
		StartLocalGame();
	}
	else if(msg.type==GMK_MSG_MOVE_INFO){ // Move info
		GMKMoveInfo info;
		memcpy(&info, msg.msg, sizeof(info));
		HandleRemoteMove(info);
	}
	else if(msg.type==GMK_MSG_REQ_PLAYER_INFO){ // Server requests player info
		SendPlayerInfo(local_player_.info);
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
		UpdateServerList(info);
	}
}


