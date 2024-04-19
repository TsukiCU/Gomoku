#include "tcp.h"
#include "players.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/poll.h>

using namespace std;

bool GMKTCPBase::SendPlayerInfo(const PlayerInfo &info)
{
	GMKTCPMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_PLAYER_INFO;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	return true;
}

void GMKTCPBase::ResetBoard()
{
	game_.state=2;
	game_.clearBoard();
}

void GMKTCPBase::CreateReceiveThread()
{
	recv_thread_=std::thread(&GMKTCPBase::ReceiveThreadFunc,this);
}

void GMKTCPBase::ReceiveThreadFunc()
{
	GMKTCPMessage msg;
	printf("Message handling thread created!\n");
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

void GMKTCPBase::StartLocalGame()
{
	game_.clearBoard();
	printf("Game started! You take %s piece!\n",
			local_player_.black?"Black":"White"
	);
	piece_count=0;
	game_.state=0;
	game_.current_player=1;
	game_.displayBoard();
}

bool GMKTCPBase::IsMoveValid(const GMKMoveInfo &info, const Player &player)
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
	// That is (piece_count%2) XOR local_player_.black
	if(!(piece_count%2^player.black)){
		printf("It's not your turn.\n");
		return false;
	}
	if(!game_.valid_move(info.x-1,info.y-1)){
		printf("Invalid move!\n");
		return false;
	}
	return true;
}

bool GMKTCPBase::MakeMove(int x, int y)
{
	GMKMoveInfo info;
	info.idx=piece_count;
	info.x=x;
	info.y=y;

	if(!IsMoveValid(info, local_player_))
		return false;

	// Local player takes move
	local_player_.makeMove(make_pair(x - 1, y - 1));
	++piece_count;
	if(display){
		display->update_piece_info(x-1,y-1,local_player_.black?1:2,1);
		display->update_select(x-1,y-1);
	}
	// Send move message
	GMKTCPMessage msg;
	msg.magic = 0x474D4B4D;
	msg.type = GMK_MSG_MOVE_INFO;
	memcpy(msg.msg, &info, sizeof(info));
	write(remote_fd_, &msg, sizeof(msg));
	
	printf("%d:%s (You) take a move at (%d,%d)\n", piece_count,
		   local_player_.black?"Black":"White",
		   x,y
	);
	game_.displayBoard();
	return true;
}

int GMKTCPBase::CheckGameResult()
{
	if(!connected_)
		return 3;
	// Game not started/ended.
	if(game_.state!=1)
		return -1;
	return local_player_.black^(game_.winner==2);
}

void GMKTCPBase::HandleRemoteMove(const GMKMoveInfo &info)
{
	if(!IsMoveValid(info, remote_player_)){
		// TODO:Remote move not valid
	}
	// Remote player takes move
	remote_player_.makeMove(make_pair(info.x - 1, info.y - 1));
	++piece_count;
	if(display){
		display->update_piece_info(info.x-1,info.y-1,remote_player_.black?1:2,1);
	}
	printf("%d:%s take a move at (%d,%d)\n", piece_count,
		remote_player_.black?"Black":"White",
		info.x,info.y
	);
	game_.displayBoard();
}


void GMKServer::HandleMessage(const GMKTCPMessage &msg)
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

    // Binding newly created socket to given IP and verification 
	int result = 1;
	int i=0;
	for(;i<GMK_SERVER_PORT_MAX_OFFSET;++i){
		result = bind(local_fd_, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if(!result)
			break;
		servaddr.sin_port = htons(GMK_SERVER_PORT+i);
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
	GMKTCPMessage msg;
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
	GMKTCPMessage msg;
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
	GMKTCPMessage msg;
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
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(GMK_SERVER_PORT);
 
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


void GMKClient::HandleMessage(const GMKTCPMessage &msg)
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
}


