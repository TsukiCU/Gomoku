# Classes

## Class Player - player behaviors

### Property

* **PlayerInfo** info
  * pid
  * name
* **bool** black

### Interface

* makeMove()
* regretMove()
* resign()

## Class Gomoku - game board logics

### Property

####   game settings

* WIN_LENGTH
* board_size
* mode - 0 for pvp, 1 for pve.
* record_game - Whether to record game
* TODO max_regret_times - max regret times for player

 ####   game status

* state - game status, 0 for ongoing, 1 for we have a winner already.
* current_player - 1 for black's turn, 2 for white's turn.
* winner

* board
* record
* regretTimes - Times user has regretted

####  game display

* display

### Interface

####   game logic check

* on_board
* valid_move
* check_win
* is_draw

####    game actions

* make_move - if success, return 0, else return 1
* regret_move - if success, return 0, else return 1
* end_game
* switch_players
* resetGame
* recordGame

####    game display

* set_display
* displayBoard - display board on console (not VGA)

## Class GomokuAI - AI Algorithms

### Property

* maxDepth      - Maximum depth for minimax algorithm.
* strategy      - AI aggressiveness. Increases from 1 to 3.

### Interfaces

#### AI Algorithm

* shapeTable    - Table whcih stores all the valid shapes for future evaluation.
* getScorefromTable - Get the current score from the board.
* posToStr      - Used when traversing the board for evaluation. Turn a (x, y) pair to str for filling record.
* ratePos       - Rate the value of one stone in a given position.
* evaluate      - Give an evaluation for the current player.
* makeMove      - AI makes a move at a given position.
* undoMove      - AI removes a move in a given position. Used in minimax function.
* findBestMove  - AI finds the next move.
* MiniMax       - MiniMax with Alpha-Beta Pruning.

####  Opening

**First several moves for the AI is fixed to achieve the best performance.**

* OpeningMap        - AI looks up the opening map to play the most reasonable moves.
* decideThirdMove   - Decide the third move for AI
* decideFourthMove  - Decide the fourth move for AI.


## Class Display - VGA Display logic

### Property

####      driver info

* params_ - VGA driver arguments to write
* vga_gomoku_fd_ - VGA driver file descriptor
* dev_name_ - VGA driver name

####      display info

* select_x_, select_y - Currently selected board position, -1 means not selected
* message_index_ - Currently selected message index - -1 means not selected
* dialog_ - current displayed page, 0 means menu, 1 means board

### Interface

####   driver interface

* open_display - open VGA driver fd
* sync - write arguments to VGA driver
* update_register - update one VGA argument
  * uint index - VGA argument index
  * uint16 val - VGA argument value, 16 bits
  * bool sync=true - whether to do sync

####   paging interface

* show_menu - display menu page
* show_board - display board page
  * bool clear - whether to clear the board
* clear_board

####   message interface

* update_message_visibility - to show/hide messages
  * int, bool, bool
    * int index - message index
    * bool visible
    * bool sync
  * uint16_t, bool - update all messages' visibility by a 16-bit value
    * uint16_t val - visibility value for all messages
    * bool sync

####   piece interface

* update_piece_info - updates visibility option on a position of the board
  * int x, int y - board position
  * int piece - visibility option
    * 0 - no piece
    * 1 - white piece
    * 2 - black piece
  * bool current=1 - whether to show the last-move mark
  * bool sync

####   selection interface

* select_message - select a message by message index,  if a valid message is selected, the board position will be unselected
* update_select - select a board position; if a valid position is selected, the message will be unselected
* update_select_by_direction - update selection by direction
  * int direction - move selection
    * 0 - move up
    * 1 - move down
    * 2 - move left
    * 3 - move right
* select_up, select_down, select_left, select_right - update selection
* update_select_by_vga_xy - update selection by VGA coordinate
* check_select_by_vga_xy - check current selection by VGA coordinate
  * [Input] uint16 vga_x, vga_y - VGA coordinate
  * [Output] int select_x,select_y - output selected board position, -1 means not selected
  * [Output] int message_index - selected message index

####   touchpad interface

* update_touchpad_cursor - update the touchpad cursor
  * uint16 vga_x,vga_y - VGA coordinate
  * bool visible - whether to show the cursor

## Class InputEventHandler - virtual interface

A interface class for handling input events

### Enum InputEventType

* 0 - XBOX_UP
* 1 - XBOX_DOWN
* 2 - XBOX_LEFT
* 3 - XBOX_RIGHT
* 4 - XBOX_A
* 5 - XBOX_B
* 6 - XBOX_X
* 7 - XBOX_Y
* 0x18 - PAD_LEFT_PRESS
  * 0b1100
  * 0x1 or 0b1000 means pad events
  * 0x08 or 0b100 means pad press events
* 0x19 - PAD_RIGHT_PRESS
  * 0b1101
* 0x1a - PAD_MID_PRESS
  * 0b1110
* 0x10 - PAD_LEFT_RELEASE
  * 0b1000
* 0x11 - PAD_RIGHT_RELEASE
  * 0b1001
* 0x12 - PAD_MID_RELEASE
  * 0b1010

### Struct InputEvent

* InputEventType type - event type
* uint16 vga_x,vga_y - VGA coordinates, used in pad events

### Interface

* virtual handle_input_event - input event interface. **This should be implemented by inherited classes** 

## Class Touchpad - touchpad device driver

**Asynchronously** receives touchpad device outputs and sends input events to the handler.

### Struct XPPenMessage - Touchpad USB message structure

This message has a length of 10 bytes.

* unsigned char magic - XPPen magic number
  * should always be 0x07
* unsigned char status - Touchpad pen status
  *  0xc0 - pen not detected
  * 0xa0 - pen detected
  * 0xa1 - pen touches pad
  * 0xa2 - pen bottom button clicked
  * 0xa3 - pen touches pad & pen bottom button clicked
* uint16 horizontal, vertical - Touchpad coordinates. These don't equal VGA coordinates.
  * vga_x = (uint16_t)(horizontal/51.2)
  * vga_y = (uint16_t)(vertical/32768.0*480)
* uint16 pressure - pen pressure level
* uint16 unknown

### Property

* display_ - Display class pointer
* handle_ - USB device handle
* input_handler_ - input event handler
* thread_ - touchpad message handling thread
* thread_stopped_ - Whether to stop the handling thread
* usb_timeout_ - Touchpad message read timeout in milliseconds. If read times out, we hide the cursor

### Interface

####   USB device handling

* open_touchpad_device
* close_touchpad_device

####   Thread handling

* create_touchpad_handling_thread
* stop_touchpad_handling_thread

####   USB Message handling

* handle_touchpad_message_func - handling thread main function, updates the cursor
* handle_touchpad_mouse_event - handles touchpad click events, wraps it to InputEvent and sends it to InputEventHandler

####   Debug function

* print_touchpad_message - print message info to the console

## Network Data structure

### Struct GMKNetMessage - message packet for network

* uint32 magic - Gomoku message magic number
  * Always be 0x474D4B4D - which stands for ASCII string "GMKM"
* u_char type - Gomoku message type, see network message protocol section
* u_char *msg - message buffer

### Struct GMKGameInfo - game settings information

* uint32 - board_size
* int32_t win_length

### Struct GMKMoveInfo - Piece move information for network

* uint32 idx - move index
* int x,y - board position of the move

### Struct GMKServerInfo - Gomoku Server Information

* string address - server IP address
* uint16 port - server port
* uint16 status - server current status
  * 0 - server is down
  * 1 - server is playing
  * 2 - server is ready to play

## Class GMKNetBase - base class for network game

### Property

####   game logic

* Gomoku *game_ - game board pointer
* Player local_player_, remote_player\_ - player info
* uint32 piece_count_ - current move index

####   network info

* int remote_fd_ - remote TCP socket file descriptor
* int udp_fd_ - Local UDP socket file descriptor, used for server discovery
* uint16 - udp_port_ - Local UDP listening port
* std::thread recv_thread_ - remote TCP message handling thread
* std::thread udp_recv_thread_ - remote UDP message handling thread
* TODO UNUSED bool loop_stopped_ - whether to stop threads
* bool connected_ - whether currently connected to remote TCP socket 

### Interface



* GMK_MSG_PLAYER_INFO 1
* GMK_MSG_GAME_INFO 2
* GMK_MSG_GAME_START 3
* GMK_MSG_MOVE_INFO 4
* GMK_MSG_REQ_PLAYER_INFO 5
* GMK_MSG_MOVE_REGRET 6
* GMK_MSG_GAME_RESIGN 7
* GMK_MSG_UDP_DISCOVER 0xF1
* GMK_MSG_UDP_READY   0xF2
* GMK_MSG_UDP_BUSY   0xF3



## TODO Message group

* 2^6-2 groups
* 10 messages in each group

### Struct GMKDisplayMessageInfo

* string content
* uint16 index
* uint16 group
* bool selectable
* bool visible
* BBOX bounding_box
* bool selected()
* uint16 up, down, left, right; 
  * The next selection message index of different location
  * All 1 means no message
  * If lower 8 bits == 0, select board
    * Board xy
* uint16 command - command type when clicked
  * resign
  * regret
  * quit
  * PVP
  * PVE
  * CREATE ROOM
  * JOIN ROOM
  * NONE

### Class DisplayMessageGroup

* BBOX selected_area_
* uint16 selected_index_
* update_message_visibility
  * uint16 index
* update_group_visibility
  * uint16 group
* protected update_selectable
* protected update_bbox
* int  message_select_by_vga_xy
  * ret - 0 means none is selected
* uint16 next_message_by_direction
  * uint16 index - could possibly parse board position

### Visibility

* Avalon ggggggvvvvvvvvvv 
  * g group index - 0 for no group, all 1 for all groups
  * v - message visibility
* HW
  * case gggggg
    * visible[group_end:group_start] <= writedata[group_count-1:0]
* Steps
  * group->update_message_visibility
  * display->update...
  * HW

### Selection

### Commands

####   Board

* move
* regret
* resign
* tip
* quit

####   Menu

* PVE
* PVP
* CREATE ROOM
* JOIN ROOM
* QUIT

### InputEvent

* Handler->wait_for_command()
  * while(command_received_);
* Handler->handle_event()
  * UP DOWN LEFT RIGHT
    * next_message_by_direction
    * update index
  * PAD_LEFT_PRESS
    * if(message_select_by_vga_xy) handle events
    * **Board only** get board index -> handle move
    * command_received_ = 1
  * XBOX_A
    * 

### Controller confirm event

