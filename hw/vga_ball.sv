/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module vga_ball(input logic 		clk,
				input logic 		reset,
				input logic [15:0]	writedata,
				input logic 		write,
				input 				chipselect,
				input logic [2:0]	address,

				input 				L_READY,
				input 				R_READY,
				output logic [15:0] L_DATA,
				output logic [15:0] R_DATA,
				output logic 		L_VALID,
				output logic 		R_VALID,

				output logic [7:0]	VGA_R, VGA_G, VGA_B,
				output logic 		VGA_CLK, VGA_HS, VGA_VS,
									VGA_BLANK_n,
				output logic 		VGA_SYNC_n);

	logic [10:0]		hcount;
	logic [9:0]			vcount;
	vga_counters counters(.clk50(clk), .*);
	
	/**************************************************************/
	/*****************************Audio ***************************/
	/**************************************************************/

	logic [11:0] move_addr;
	logic [15:0] move_data;
	logic play_move;

	logic [12:0] victory_addr;
	logic [15:0] victory_data;
	logic play_victory;

	logic [10:0] menu_addr;
	logic [15:0] menu_data;
	logic play_menu;

	logic [13:0] defeat_addr;
	logic [15:0] defeat_data;
	logic play_defeat;

	reg [11:0] counter;

	soc_system_move_sound move(.address(move_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(move_data));
	soc_system_victory_sound victory(.address(victory_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(victory_data));
	soc_system_menu_sound menu(.address(menu_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(menu_data));
	soc_system_defeat_sound defeat(.address(defeat_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(defeat_data));

	always_ff @(posedge clk) begin

		if (reset) begin
			counter <= 0;
			play_move <= 0;
			play_victory <= 0;
			play_menu <= 0;
			play_defeat <= 0;
			p2_profile_cat <= 1;
		end
		else if (L_READY == 1 && R_READY == 1 && counter < 3125) begin
			counter <= counter + 1;
			L_VALID <= 0;
			R_VALID <= 0;
		end
		else if (L_READY == 1 && R_READY == 1 && counter >= 3125) begin
			counter <= 0;
			L_VALID <= 1;
			R_VALID <= 1;
			if(play_move) begin
				if(move_addr < 12'd2871) begin
					move_addr <= move_addr + 1;
				end
				else begin
					move_addr <= 0;				
				end
				L_DATA <= move_data;
				R_DATA <= move_data;
			end
			else if(play_victory) begin
				if(victory_addr < 13'd6628) begin
					victory_addr <= victory_addr + 1;
				end
				else begin
					victory_addr <= 0;				
				end
				L_DATA <= victory_data;
				R_DATA <= victory_data;
			end
			else if(play_menu) begin
				if(menu_addr < 11'd1497) begin
					menu_addr <= menu_addr + 1;
				end
				else begin
					menu_addr <= 0;				
				end
				L_DATA <= menu_data;
				R_DATA <= menu_data;
			end
			else if(play_defeat) begin
				if(defeat_addr < 14'd11096) begin
					defeat_addr <= defeat_addr + 1;
				end
				else begin
					defeat_addr <= 0;				
				end
				L_DATA <= defeat_data;
				R_DATA <= defeat_data;
			end
		end
		else begin
			L_VALID <= 0;
			R_VALID <= 0;
		end
	end

	/*******************************************************/
	/******************* Board Display *********************/
	/*******************************************************/

	/************ Board & piece **************/
	// Board Piece information array 15*15*info_length
	// Index - x,y,info, x is vertial, y is horizontal
	// 0 - unused
	// 1,2 - piece info
	//       00 - no piece
	//       10 - draw white piece
	//       01 - draw black piece
	logic [14:0][14:0][2:0]	board;
	logic[4:0] 	piece_v;		// Piece image offset v
	logic[5:0] 	piece_h;		// Piece image offset h
	logic[3:0] 	piece_x;		// Piece coordinate x
	logic[3:0] 	piece_y;		// Piece coordinate y
	logic[2:0] 	piece_info;		// Piece image information
	logic[3:0] 	selected_x;		// Selected mark x position
	logic[3:0] 	selected_y;		// Selected mark y position
	logic[3:0] 	last_piece_x;	// Last piece mark x position
	logic[3:0] 	last_piece_y;	// Last piece mark y position
	logic 		is_piece_area;
	logic 		is_board_area;
	logic 		is_last_piece_area;
	logic		is_selected_area;

	assign piece_info = board[piece_x][piece_y];
	// Board Area [0,499]
	assign is_board_area = (hcount[10:1]>=10'd0) && (hcount[10:1]<=10'd500);
	// Piece 33x31
	// Piece Area: h[4,498] v[7,472]
	assign is_piece_area = (hcount[10:1]>=10'd4) && (hcount[10:1]<=10'd498) && (vcount[9:0]>=10'd7) && (vcount[9:0]<=10'd472);
	assign is_selected_area  = ((piece_h <= 6'd1) || (piece_h >= 6'd31) || (piece_v <= 5'd1) || (piece_v >= 5'd29)) && (piece_h <= 6'd5 || piece_h >= 6'd27) && (piece_v <= 5'd5 || piece_v >= 5'd25) && is_piece_area;
	assign is_last_piece_area = (piece_h >=6'd16) && (piece_v >= 5'd15) && (piece_h+piece_v <= 6'd43) && is_piece_area;

	/************ Board & piece ROM ************/
	logic [17:0] 	bg_addr;	// Background image ROM address
	logic [7:0] 	bg_rgb;		// Background image ROM output, each output has 2 palette values
	logic [3:0] 	bg_palette;	// Background image palette value
	logic [9:0] 	piece_addr;	// Piece image ROM address
	logic [7:0] 	black_rgb;	// Black image ROM output
	logic [7:0] 	white_rgb;	// Piece image ROM output

	assign bg_addr = is_board_area?((vcount[9:0]*17'd500+hcount[10:1])>>1):0;
	assign bg_palette = hcount[1]?bg_rgb[7:4]:bg_rgb[3:0];

	// Image ROMs
	soc_system_background background(.address(bg_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(bg_rgb));
	soc_system_white_piece white(.address(piece_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(white_rgb));
	soc_system_black_piece black(.address(piece_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(black_rgb));

	// Set board info
	initial begin
		for(int i=0;i<15;i++) begin
			for(int j=0;j<15;j++) begin
				board[i][j] <= 3'b0;
			end
		end
		selected_y = 4'h7;
		selected_x = 4'h7;
		last_piece_x = 4'hf;
		last_piece_y = 4'hf;
		msg_visible_ingame_options <= -1;
		msg_visible_ingame_players <= -1;
		msg_visible_ingame_message <= -1;
		msg_visible_ingame_confirm <= -1;
		//msg_display_menu <= 0;
		//msg_display_ingame <= 0;
		msg_selected = 9; // P1 selected
		//msg_visible_ingame_players[1] <= 0;
		//msg_visible_ingame_message[0] <= 1;
		black_on_top <= 0;
	end

	// Set piece counter
	always_ff @(posedge clk) begin
		// piece left 4
		if(hcount[10:1]<=10'd4) begin
			piece_h <= 6'd0;
			piece_y <= 4'd0;
		end
		// piece right 498
		else if(hcount[10:1]>=10'd498) begin
			piece_h <= 6'd32;
			piece_y <= 4'd14;
		end
		else if(hcount[0]==0) begin
			if(piece_h==6'd32) begin
				piece_y <= piece_y + 4'd1;
				piece_h <= 0;
			end else
				piece_h <= piece_h + 6'd1;
		end
		if(hcount[10:0]==11'd0)
			// piece top 7
			if(vcount[9:0]<=10'd7) begin
				piece_v <= 5'd0;
				piece_x <= 4'd0;
			end
			// piece bottom 472
			else if(vcount[9:0]>=10'd472) begin
				piece_v <= 5'd30;
				piece_x <= 4'd14;
			end
			else if(piece_v==5'd30) begin
				piece_x <= piece_x + 4'd1;
				piece_v <= 0;
			end
			else begin
				piece_v <= piece_v + 5'd1;
			end
	end



	/*******************************************************/
	/***************** Player UI Display *******************/
	/*******************************************************/

	/*********** Piece Icon **********/
		logic[4:0] icon_v;			// Player piece icon image offset v
		logic[5:0] icon_h;			// Player piece icon image offset h
		logic	is_piece_top; 		// Piece for Player 1, top piece
		logic	is_piece_bottom; 	// Piece for Player 1, bottom piece
		logic	black_on_top;		// Whether Player 1 takes black piece
		logic	is_black_icon_area;
		logic	is_white_icon_area;

		assign is_black_icon_area = black_on_top?is_piece_top:is_piece_bottom;
		assign is_white_icon_area = black_on_top?is_piece_bottom:is_piece_top;

		// piece top area: h=[510,543],v=[137(0+3+128+6),165(137+31))
		assign is_piece_top = (hcount[10:1]>=10'd510) && (hcount[10:1]<=10'd543) && (vcount[9:0]>=10'd137) && (vcount[9:0]<=10'd168);
		// piece bottom area: h=[510,543],v=[304(170+3+128+3),335(304+31))
		assign is_piece_bottom = (hcount[10:1]>=10'd510) && (hcount[10:1]<=10'd543) && (vcount[9:0]>=10'd304) && (vcount[9:0]<=10'd335);

	/********* Piece Icon end ********/


	/********* Player profile *********/
		logic [12:0] p1_profile_addr;	// Player 1 profile Image ROM address
		logic [4:0]  p1_profile_palette; // Player 1 profile Image palette value
		logic [12:0] p2_profile_addr;	// Player 2 profile Image ROM address
		logic [4:0]  p2_profile_palette; // Player 2 profile Image palette value

		logic [7:0]  panda_rgb;			// Panda Image ROM output, contains 2 palette values
		logic [7:0]  dog_rgb;			// Dog Image ROM output, contains 2 palette values
		logic [7:0]  cat_rgb;			// Cat Image ROM output, contains 2 palette values

		logic p2_profile_cat;			// Whether P2 profile is cat or dog
		logic is_p1_profile_area;
		logic is_p2_profile_area;

		/* p2_profile_palette is assign in the always comb below. */
		assign p1_profile_palette = hcount[1]?panda_rgb[7:4]:panda_rgb[3:0];
		assign p1_profile_addr = is_p1_profile_area? ((((vcount[9:0] - 13'd3) << 7) + (hcount[10:1] - 13'd506)) >> 1):0;
		assign p2_profile_addr = is_p2_profile_area? ((((vcount[9:0] - 13'd173) << 7) + (hcount[10:1] - 13'd506)) >> 1):0;
		/* 
			Player 1 Picture 128x128
			Pic 1 Area: h[506,634] v[3,134] 
		*/
		assign is_p1_profile_area = (hcount[10:1]>=10'd506) && (hcount[10:1]<=10'd634) && (vcount[9:0]>=10'd3) && (vcount[9:0]<=10'd134);
		/* 
			Player 2 Picture 128x128
			Pic 2 Area: h[506,634] v[174,302]
		*/
		assign is_p2_profile_area = (hcount[10:1]>=10'd506) && (hcount[10:1]<=10'd634) && (vcount[9:0]>=10'd174) && (vcount[9:0]<=10'd302);

		soc_system_panda_avatar panda(.address(p1_profile_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(panda_rgb));
		soc_system_dog_avatar dog(.address(p2_profile_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(dog_rgb));
		soc_system_cat_avatar cat(.address(p2_profile_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(cat_rgb));

		// draw player 1's profile photo
		    // Include the image data
		    //`include "panda110x110_data.v"

		    // Coordinates for the top-left corner of the image
		    //localparam IMAGE_X = 535;
		    //localparam IMAGE_Y = 100;

	/******* Player profile end *********/

	/************* Cursor *************/
		logic is_cursor_visible;
		logic [9:0] cursor_v;
		logic [9:0] cursor_h;
		logic is_cursor_area;

		assign is_cursor_area = (hcount[10:1]>=(cursor_h-2)) && (hcount[10:1]<=(cursor_h+2)) && (vcount[9:0]>=(cursor_v-2)) && (vcount[9:0]<=(cursor_v+2));

	/*********** Cursor End ***********/

	/************* Win, loss message *************/
		// Message
		logic is_p1_win; // "P1 WIN" = h[150,], v[187(164+23),]
		logic is_p2_win; // "P2 WIN" = h[150,], v[187(164+23),]
		logic is_pve_win; // "YOU WIN!" = h[129(121+8)], v[187(164+23),227(187+40(height))]
		logic is_pve_loss; // "YOU LOSS" = h[125(121+4),], v[187(164+23),]
		logic are_you_sure; // "Are You Sure?" = h[150(24+126),358(150+208(total width))], v[164(148+16),184(164+20(height))]
		logic is_message;
		logic is_message_area;
		// Message Area: h[121(22+3*33),381(481-3*33)], v[179(24+31x5),270(179+31x3-2)]
		assign is_message_area = (hcount[10:1]>=10'd121) && (hcount[10:1]<=10'd382) && (vcount[9:0]>=10'd179) && (vcount[9:0]<=10'd270);	

		// confirmation
		logic is_exit; // h[220,], v[241,] , click 'exit' from in-game options on the lower right corner
		logic is_exit_yes; // h[164,], v[244,]
		logic is_exit_no;  // h[311(164+48+33*3),], v[244,]


	/************* Other UI ************/
		logic	is_menu;      		// Whether current page is menu
		logic	is_board; 			// Whether current page is board
		logic	is_player1_area; 	// Player 1 UI area
		logic	is_player2_area; 	// Player 2 UI area
		logic	is_option_area; 	// Board option area
		logic	division_line;		// Division Line

		// player 1 - top area: h=[500,680],v=[0,170)
		assign is_player1_area = (hcount[10:1]>=10'd500) && (hcount[10:1]<=10'd680) && (vcount[9:0]>=10'd0) && (vcount[9:0]<=10'd170);
		// player 2 - bottom area: h=[500,680],v=[170,340)
		assign is_player2_area = (hcount[10:1]>=10'd500) && (hcount[10:1]<=10'd680) && (vcount[9:0]>=10'd170) && (vcount[9:0]<=10'd340);
		// option area: h=[500,680],v=[340,480]
		assign is_option_area = (hcount[10:1]>=10'd500) && (hcount[10:1]<=10'd680) && (vcount[9:0]>=10'd340) && (vcount[9:0]<=10'd480);
		// division line
		assign division_line = (vcount[9:0] == 10'd170) && (hcount[10:1] >= 10'd500) && (hcount[10:1] < 10'd680);

	/*********** Other UI End ***********/




	/*******************************************************/
	/**** Message display block, generated by sv_gen.py ****/
	/*******************************************************/

    // Message visibility and display signal of group menu
	logic [5:0] msg_visible_menu;
	logic [5:0] msg_display_menu;
    // Message visibility and display signal of group ingame_options
	logic [3:0] msg_visible_ingame_options;
	logic [3:0] msg_display_ingame_options;
    // Message visibility and display signal of group ingame_players
	logic [2:0] msg_visible_ingame_players;
	logic [2:0] msg_display_ingame_players;
    // Message visibility and display signal of group ingame_message
	logic [4:0] msg_visible_ingame_message;
	logic [4:0] msg_display_ingame_message;
    // Message visibility and display signal of group ingame_confirm
	logic [2:0] msg_visible_ingame_confirm;
	logic [2:0] msg_display_ingame_confirm;




	// Selected message index, 0 means none is selected
	logic [5:0] msg_selected;
	// Whether the current message is selected
	logic cur_msg_selected;
	// Font pixel index of a line, [0,8)
	logic [2:0] font_pix_idx;
	logic [7:0] font_addr;
	logic [7:0] font_val;
	soc_system_font font(.address(font_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(font_val));

	always_ff @(posedge clk) begin
		/**
		* message: GOMOKU
		* group 1: menu
		* group_index: 0
		* select_index: 1
		* h_start: 128
		* v_start: 128
		* font_width: 64
		* font_height: 80
		*/
		if((hcount[10:1] >= 10'd128) && (hcount[10:1] < 10'd512) && (vcount >= 10'd128) && (vcount < 10'd208) && msg_visible_menu[0]) begin
			msg_display_menu[0] <= 1;
			cur_msg_selected <= (msg_selected==1);
			case((hcount[10:1]-128)>>6)
				8'd0: font_addr <= 8'd30+((vcount[9:0]-128)>>4);
				8'd1: font_addr <= 8'd70+((vcount[9:0]-128)>>4);
				8'd2: font_addr <= 8'd60+((vcount[9:0]-128)>>4);
				8'd3: font_addr <= 8'd70+((vcount[9:0]-128)>>4);
				8'd4: font_addr <= 8'd50+((vcount[9:0]-128)>>4);
				8'd5: font_addr <= 8'd100+((vcount[9:0]-128)>>4);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-128)>>3;
		end
		else
			msg_display_menu[0] <= 0;

		/**
		* message: START PVP
		* group 1: menu
		* group_index: 1
		* select_index: 2
		* h_start: 248
		* v_start: 250
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd248) && (hcount[10:1] < 10'd392) && (vcount >= 10'd250) && (vcount < 10'd270) && msg_visible_menu[1]) begin
			msg_display_menu[1] <= 1;
			cur_msg_selected <= (msg_selected==2);
			case((hcount[10:1]-248)>>4)
				8'd0: font_addr <= 8'd90+((vcount[9:0]-250)>>2);
				8'd1: font_addr <= 8'd95+((vcount[9:0]-250)>>2);
				8'd2: font_addr <= 8'd0+((vcount[9:0]-250)>>2);
				8'd3: font_addr <= 8'd85+((vcount[9:0]-250)>>2);
				8'd4: font_addr <= 8'd95+((vcount[9:0]-250)>>2);
				8'd5: font_addr <= 8'd180+((vcount[9:0]-250)>>2);
				8'd6: font_addr <= 8'd75+((vcount[9:0]-250)>>2);
				8'd7: font_addr <= 8'd105+((vcount[9:0]-250)>>2);
				8'd8: font_addr <= 8'd75+((vcount[9:0]-250)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-248)>>1;
		end
		else
			msg_display_menu[1] <= 0;

		/**
		* message: START PVE
		* group 1: menu
		* group_index: 2
		* select_index: 3
		* h_start: 248
		* v_start: 280
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd248) && (hcount[10:1] < 10'd392) && (vcount >= 10'd280) && (vcount < 10'd300) && msg_visible_menu[2]) begin
			msg_display_menu[2] <= 1;
			cur_msg_selected <= (msg_selected==3);
			case((hcount[10:1]-248)>>4)
				8'd0: font_addr <= 8'd90+((vcount[9:0]-280)>>2);
				8'd1: font_addr <= 8'd95+((vcount[9:0]-280)>>2);
				8'd2: font_addr <= 8'd0+((vcount[9:0]-280)>>2);
				8'd3: font_addr <= 8'd85+((vcount[9:0]-280)>>2);
				8'd4: font_addr <= 8'd95+((vcount[9:0]-280)>>2);
				8'd5: font_addr <= 8'd180+((vcount[9:0]-280)>>2);
				8'd6: font_addr <= 8'd75+((vcount[9:0]-280)>>2);
				8'd7: font_addr <= 8'd105+((vcount[9:0]-280)>>2);
				8'd8: font_addr <= 8'd20+((vcount[9:0]-280)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-248)>>1;
		end
		else
			msg_display_menu[2] <= 0;

		/**
		* message: CREATE LAN
		* group 1: menu
		* group_index: 3
		* select_index: 4
		* h_start: 240
		* v_start: 310
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd240) && (hcount[10:1] < 10'd400) && (vcount >= 10'd310) && (vcount < 10'd330) && msg_visible_menu[3]) begin
			msg_display_menu[3] <= 1;
			cur_msg_selected <= (msg_selected==4);
			case((hcount[10:1]-240)>>4)
				8'd0: font_addr <= 8'd10+((vcount[9:0]-310)>>2);
				8'd1: font_addr <= 8'd85+((vcount[9:0]-310)>>2);
				8'd2: font_addr <= 8'd20+((vcount[9:0]-310)>>2);
				8'd3: font_addr <= 8'd0+((vcount[9:0]-310)>>2);
				8'd4: font_addr <= 8'd95+((vcount[9:0]-310)>>2);
				8'd5: font_addr <= 8'd20+((vcount[9:0]-310)>>2);
				8'd6: font_addr <= 8'd180+((vcount[9:0]-310)>>2);
				8'd7: font_addr <= 8'd55+((vcount[9:0]-310)>>2);
				8'd8: font_addr <= 8'd0+((vcount[9:0]-310)>>2);
				8'd9: font_addr <= 8'd65+((vcount[9:0]-310)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-240)>>1;
		end
		else
			msg_display_menu[3] <= 0;

		/**
		* message: JOIN LAN
		* group 1: menu
		* group_index: 4
		* select_index: 5
		* h_start: 256
		* v_start: 340
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd256) && (hcount[10:1] < 10'd384) && (vcount >= 10'd340) && (vcount < 10'd360) && msg_visible_menu[4]) begin
			msg_display_menu[4] <= 1;
			cur_msg_selected <= (msg_selected==5);
			case((hcount[10:1]-256)>>4)
				8'd0: font_addr <= 8'd45+((vcount[9:0]-340)>>2);
				8'd1: font_addr <= 8'd70+((vcount[9:0]-340)>>2);
				8'd2: font_addr <= 8'd40+((vcount[9:0]-340)>>2);
				8'd3: font_addr <= 8'd65+((vcount[9:0]-340)>>2);
				8'd4: font_addr <= 8'd180+((vcount[9:0]-340)>>2);
				8'd5: font_addr <= 8'd55+((vcount[9:0]-340)>>2);
				8'd6: font_addr <= 8'd0+((vcount[9:0]-340)>>2);
				8'd7: font_addr <= 8'd65+((vcount[9:0]-340)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-256)>>1;
		end
		else
			msg_display_menu[4] <= 0;

		/**
		* message: EXIT
		* group 1: menu
		* group_index: 5
		* select_index: 6
		* h_start: 288
		* v_start: 370
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd288) && (hcount[10:1] < 10'd352) && (vcount >= 10'd370) && (vcount < 10'd390) && msg_visible_menu[5]) begin
			msg_display_menu[5] <= 1;
			cur_msg_selected <= (msg_selected==6);
			case((hcount[10:1]-288)>>4)
				8'd0: font_addr <= 8'd20+((vcount[9:0]-370)>>2);
				8'd1: font_addr <= 8'd115+((vcount[9:0]-370)>>2);
				8'd2: font_addr <= 8'd40+((vcount[9:0]-370)>>2);
				8'd3: font_addr <= 8'd95+((vcount[9:0]-370)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-288)>>1;
		end
		else
			msg_display_menu[5] <= 0;

		/**
		* message: Regret
		* group 2: ingame_options
		* group_index: 0
		* select_index: 7
		* h_start: 520
		* v_start: 355
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd520) && (hcount[10:1] < 10'd616) && (vcount >= 10'd355) && (vcount < 10'd375) && msg_visible_ingame_options[0]) begin
			msg_display_ingame_options[0] <= 1;
			cur_msg_selected <= (msg_selected==7);
			case((hcount[10:1]-520)>>4)
				8'd0: font_addr <= 8'd85+((vcount[9:0]-355)>>2);
				8'd1: font_addr <= 8'd20+((vcount[9:0]-355)>>2);
				8'd2: font_addr <= 8'd30+((vcount[9:0]-355)>>2);
				8'd3: font_addr <= 8'd85+((vcount[9:0]-355)>>2);
				8'd4: font_addr <= 8'd20+((vcount[9:0]-355)>>2);
				8'd5: font_addr <= 8'd95+((vcount[9:0]-355)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-520)>>1;
		end
		else
			msg_display_ingame_options[0] <= 0;

		/**
		* message: HINT
		* group 2: ingame_options
		* group_index: 1
		* select_index: 8
		* h_start: 536
		* v_start: 385
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd536) && (hcount[10:1] < 10'd600) && (vcount >= 10'd385) && (vcount < 10'd405) && msg_visible_ingame_options[1]) begin
			msg_display_ingame_options[1] <= 1;
			cur_msg_selected <= (msg_selected==8);
			case((hcount[10:1]-536)>>4)
				8'd0: font_addr <= 8'd35+((vcount[9:0]-385)>>2);
				8'd1: font_addr <= 8'd40+((vcount[9:0]-385)>>2);
				8'd2: font_addr <= 8'd65+((vcount[9:0]-385)>>2);
				8'd3: font_addr <= 8'd95+((vcount[9:0]-385)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-536)>>1;
		end
		else
			msg_display_ingame_options[1] <= 0;

		/**
		* message: RESIGN
		* group 2: ingame_options
		* group_index: 2
		* select_index: 9
		* h_start: 520
		* v_start: 415
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd520) && (hcount[10:1] < 10'd616) && (vcount >= 10'd415) && (vcount < 10'd435) && msg_visible_ingame_options[2]) begin
			msg_display_ingame_options[2] <= 1;
			cur_msg_selected <= (msg_selected==9);
			case((hcount[10:1]-520)>>4)
				8'd0: font_addr <= 8'd85+((vcount[9:0]-415)>>2);
				8'd1: font_addr <= 8'd20+((vcount[9:0]-415)>>2);
				8'd2: font_addr <= 8'd90+((vcount[9:0]-415)>>2);
				8'd3: font_addr <= 8'd40+((vcount[9:0]-415)>>2);
				8'd4: font_addr <= 8'd30+((vcount[9:0]-415)>>2);
				8'd5: font_addr <= 8'd65+((vcount[9:0]-415)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-520)>>1;
		end
		else
			msg_display_ingame_options[2] <= 0;

		/**
		* message: EXIT
		* group 2: ingame_options
		* group_index: 3
		* select_index: 10
		* h_start: 536
		* v_start: 445
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd536) && (hcount[10:1] < 10'd600) && (vcount >= 10'd445) && (vcount < 10'd465) && msg_visible_ingame_options[3]) begin
			msg_display_ingame_options[3] <= 1;
			cur_msg_selected <= (msg_selected==10);
			case((hcount[10:1]-536)>>4)
				8'd0: font_addr <= 8'd20+((vcount[9:0]-445)>>2);
				8'd1: font_addr <= 8'd115+((vcount[9:0]-445)>>2);
				8'd2: font_addr <= 8'd40+((vcount[9:0]-445)>>2);
				8'd3: font_addr <= 8'd95+((vcount[9:0]-445)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-536)>>1;
		end
		else
			msg_display_ingame_options[3] <= 0;

		/**
		* message: P1
		* group 3: ingame_players
		* group_index: 0
		* select_index: 11
		* h_start: 573
		* v_start: 142
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd573) && (hcount[10:1] < 10'd605) && (vcount >= 10'd142) && (vcount < 10'd162) && msg_visible_ingame_players[0]) begin
			msg_display_ingame_players[0] <= 1;
			cur_msg_selected <= (msg_selected==11);
			case((hcount[10:1]-573)>>4)
				8'd0: font_addr <= 8'd75+((vcount[9:0]-142)>>2);
				8'd1: font_addr <= 8'd135+((vcount[9:0]-142)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-573)>>1;
		end
		else
			msg_display_ingame_players[0] <= 0;

		/**
		* message: P2
		* group 3: ingame_players
		* group_index: 1
		* select_index: 12
		* h_start: 573
		* v_start: 309
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd573) && (hcount[10:1] < 10'd605) && (vcount >= 10'd309) && (vcount < 10'd329) && msg_visible_ingame_players[1]) begin
			msg_display_ingame_players[1] <= 1;
			cur_msg_selected <= (msg_selected==12);
			case((hcount[10:1]-573)>>4)
				8'd0: font_addr <= 8'd75+((vcount[9:0]-309)>>2);
				8'd1: font_addr <= 8'd140+((vcount[9:0]-309)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-573)>>1;
		end
		else
			msg_display_ingame_players[1] <= 0;

		/**
		* message: AI
		* group 3: ingame_players
		* group_index: 2
		* select_index: 13
		* h_start: 573
		* v_start: 309
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd573) && (hcount[10:1] < 10'd605) && (vcount >= 10'd309) && (vcount < 10'd329) && msg_visible_ingame_players[2]) begin
			msg_display_ingame_players[2] <= 1;
			cur_msg_selected <= (msg_selected==13);
			case((hcount[10:1]-573)>>4)
				8'd0: font_addr <= 8'd0+((vcount[9:0]-309)>>2);
				8'd1: font_addr <= 8'd40+((vcount[9:0]-309)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-573)>>1;
		end
		else
			msg_display_ingame_players[2] <= 0;

		/**
		* message: You Win!
		* group 4: ingame_message
		* group_index: 0
		* select_index: 14
		* h_start: 129
		* v_start: 187
		* font_width: 32
		* font_height: 40
		*/
		if((hcount[10:1] >= 10'd129) && (hcount[10:1] < 10'd385) && (vcount >= 10'd187) && (vcount < 10'd227) && msg_visible_ingame_message[0]) begin
			msg_display_ingame_message[0] <= 1;
			cur_msg_selected <= (msg_selected==14);
			case((hcount[10:1]-129)>>5)
				8'd0: font_addr <= 8'd120+((vcount[9:0]-187)>>3);
				8'd1: font_addr <= 8'd70+((vcount[9:0]-187)>>3);
				8'd2: font_addr <= 8'd100+((vcount[9:0]-187)>>3);
				8'd3: font_addr <= 8'd180+((vcount[9:0]-187)>>3);
				8'd4: font_addr <= 8'd110+((vcount[9:0]-187)>>3);
				8'd5: font_addr <= 8'd40+((vcount[9:0]-187)>>3);
				8'd6: font_addr <= 8'd65+((vcount[9:0]-187)>>3);
				8'd7: font_addr <= 8'd185+((vcount[9:0]-187)>>3);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-129)>>2;
		end
		else
			msg_display_ingame_message[0] <= 0;

		/**
		* message: You Loss
		* group 4: ingame_message
		* group_index: 1
		* select_index: 15
		* h_start: 125
		* v_start: 187
		* font_width: 32
		* font_height: 40
		*/
		if((hcount[10:1] >= 10'd125) && (hcount[10:1] < 10'd381) && (vcount >= 10'd187) && (vcount < 10'd227) && msg_visible_ingame_message[1]) begin
			msg_display_ingame_message[1] <= 1;
			cur_msg_selected <= (msg_selected==15);
			case((hcount[10:1]-125)>>5)
				8'd0: font_addr <= 8'd120+((vcount[9:0]-187)>>3);
				8'd1: font_addr <= 8'd70+((vcount[9:0]-187)>>3);
				8'd2: font_addr <= 8'd100+((vcount[9:0]-187)>>3);
				8'd3: font_addr <= 8'd180+((vcount[9:0]-187)>>3);
				8'd4: font_addr <= 8'd55+((vcount[9:0]-187)>>3);
				8'd5: font_addr <= 8'd70+((vcount[9:0]-187)>>3);
				8'd6: font_addr <= 8'd90+((vcount[9:0]-187)>>3);
				8'd7: font_addr <= 8'd90+((vcount[9:0]-187)>>3);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-125)>>2;
		end
		else
			msg_display_ingame_message[1] <= 0;



		/**
		* message: P1 Win!
		* group 4: ingame_message
		* group_index: 2
		* select_index: 16
		* h_start: 150
		* v_start: 187
		* font_width: 32
		* font_height: 40
		*/
		if((hcount[10:1] >= 10'd150) && (hcount[10:1] < 10'd374) && (vcount >= 10'd187) && (vcount < 10'd227) && msg_visible_ingame_message[2]) begin
			msg_display_ingame_message[2] <= 1;
			cur_msg_selected <= (msg_selected==16);
			case((hcount[10:1]-150)>>5)
				8'd0: font_addr <= 8'd75+((vcount[9:0]-187)>>3);
				8'd1: font_addr <= 8'd135+((vcount[9:0]-187)>>3);
				8'd2: font_addr <= 8'd180+((vcount[9:0]-187)>>3);
				8'd3: font_addr <= 8'd110+((vcount[9:0]-187)>>3);
				8'd4: font_addr <= 8'd40+((vcount[9:0]-187)>>3);
				8'd5: font_addr <= 8'd65+((vcount[9:0]-187)>>3);
				8'd6: font_addr <= 8'd185+((vcount[9:0]-187)>>3);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-150)>>2;
		end
		else
			msg_display_ingame_message[2] <= 0;

		/**
		* message: P2 Win!
		* group 4: ingame_message
		* group_index: 3
		* select_index: 17
		* h_start: 150
		* v_start: 187
		* font_width: 32
		* font_height: 40
		*/
		if((hcount[10:1] >= 10'd150) && (hcount[10:1] < 10'd374) && (vcount >= 10'd187) && (vcount < 10'd227) && msg_visible_ingame_message[3]) begin
			msg_display_ingame_message[3] <= 1;
			cur_msg_selected <= (msg_selected==17);
			case((hcount[10:1]-150)>>5)
				8'd0: font_addr <= 8'd75+((vcount[9:0]-187)>>3);
				8'd1: font_addr <= 8'd140+((vcount[9:0]-187)>>3);
				8'd2: font_addr <= 8'd180+((vcount[9:0]-187)>>3);
				8'd3: font_addr <= 8'd110+((vcount[9:0]-187)>>3);
				8'd4: font_addr <= 8'd40+((vcount[9:0]-187)>>3);
				8'd5: font_addr <= 8'd65+((vcount[9:0]-187)>>3);
				8'd6: font_addr <= 8'd185+((vcount[9:0]-187)>>3);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-150)>>2;
		end
		else
			msg_display_ingame_message[3] <= 0;


		/**
		* message: Are You Sure?
		* group 4: ingame_message
		* group_index: 4
		* select_index: 18
		* h_start: 150
		* v_start: 190
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd150) && (hcount[10:1] < 10'd358) && (vcount >= 10'd190) && (vcount < 10'd210) && msg_visible_ingame_message[4]) begin
			msg_display_ingame_message[4] <= 1;
			cur_msg_selected <= (msg_selected==18);
			case((hcount[10:1]-150)>>4)
				8'd0: font_addr <= 8'd0+((vcount[9:0]-190)>>2);
				8'd1: font_addr <= 8'd85+((vcount[9:0]-190)>>2);
				8'd2: font_addr <= 8'd20+((vcount[9:0]-190)>>2);
				8'd3: font_addr <= 8'd180+((vcount[9:0]-190)>>2);
				8'd4: font_addr <= 8'd120+((vcount[9:0]-190)>>2);
				8'd5: font_addr <= 8'd70+((vcount[9:0]-190)>>2);
				8'd6: font_addr <= 8'd100+((vcount[9:0]-190)>>2);
				8'd7: font_addr <= 8'd180+((vcount[9:0]-190)>>2);
				8'd8: font_addr <= 8'd90+((vcount[9:0]-190)>>2);
				8'd9: font_addr <= 8'd100+((vcount[9:0]-190)>>2);
				8'd10: font_addr <= 8'd85+((vcount[9:0]-190)>>2);
				8'd11: font_addr <= 8'd20+((vcount[9:0]-190)>>2);
				8'd12: font_addr <= 8'd190+((vcount[9:0]-190)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-150)>>1;
		end
		else
			msg_display_ingame_message[4] <= 0;


		/**
		* message: EXIT
		* group 5: ingame_confirm
		* group_index: 0
		* select_index: 19
		* h_start: 220
		* v_start: 241
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd220) && (hcount[10:1] < 10'd284) && (vcount >= 10'd241) && (vcount < 10'd261) && msg_visible_ingame_confirm[0]) begin
			msg_display_ingame_confirm[0] <= 1;
			cur_msg_selected <= (msg_selected==19);
			case((hcount[10:1]-220)>>4)
				8'd0: font_addr <= 8'd20+((vcount[9:0]-241)>>2);
				8'd1: font_addr <= 8'd115+((vcount[9:0]-241)>>2);
				8'd2: font_addr <= 8'd40+((vcount[9:0]-241)>>2);
				8'd3: font_addr <= 8'd95+((vcount[9:0]-241)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-220)>>1;
		end
		else
			msg_display_ingame_confirm[0] <= 0;

		/**
		* message: yes
		* group 5: ingame_confirm
		* group_index: 1
		* select_index: 20
		* h_start: 164
		* v_start: 244
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd164) && (hcount[10:1] < 10'd212) && (vcount >= 10'd244) && (vcount < 10'd264) && msg_visible_ingame_confirm[1]) begin
			msg_display_ingame_confirm[1] <= 1;
			cur_msg_selected <= (msg_selected==20);
			case((hcount[10:1]-164)>>4)
				8'd0: font_addr <= 8'd120+((vcount[9:0]-244)>>2);
				8'd1: font_addr <= 8'd20+((vcount[9:0]-244)>>2);
				8'd2: font_addr <= 8'd90+((vcount[9:0]-244)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-164)>>1;
		end
		else
			msg_display_ingame_confirm[1] <= 0;

		/**
		* message: no
		* group 5: ingame_confirm
		* group_index: 2
		* select_index: 21
		* h_start: 311
		* v_start: 244
		* font_width: 16
		* font_height: 20
		*/
		if((hcount[10:1] >= 10'd311) && (hcount[10:1] < 10'd343) && (vcount >= 10'd244) && (vcount < 10'd264) && msg_visible_ingame_confirm[2]) begin
			msg_display_ingame_confirm[2] <= 1;
			cur_msg_selected <= (msg_selected==21);
			case((hcount[10:1]-311)>>4)
				8'd0: font_addr <= 8'd65+((vcount[9:0]-244)>>2);
				8'd1: font_addr <= 8'd70+((vcount[9:0]-244)>>2);
				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-311)>>1;
		end
		else
			msg_display_ingame_confirm[2] <= 0;





	end

	/*******************************************************/
	/************** Message display block end **************/
	/*******************************************************/


	/*******************************************************/
	/***************** Avalon Interface ********************/
	/*******************************************************/

	always_ff @(posedge clk)
		/************ Initial **************/
		if (reset) begin
			is_menu <= 0; 	// Show Menu on boot
			is_board <= 1; 	// Show Board on boot
			is_message<= 1; // Show Message on boot
			
			// show message 
			is_p1_win <= 0;
			is_p2_win <= 0;
			is_pve_win <= 1;
			is_pve_loss <= 0;
			are_you_sure <= 0;

			// Clear board
			for(int i=0;i<15;i++) begin
				for(int j=0;j<15;j++) begin
					board[i][j] <= 3'b0;
				end
			end
			//selected_y = 4'h7;
			//selected_x = 4'h7;
			//last_piece_x = 4'hf;
			//last_piece_y = 4'hf;
			//msg_visible_ingame_options <= -1;
			//msg_selected = 9; // P1 selected
			msg_visible_ingame_players <= 3'b101;
			msg_visible_ingame_message <= 5'b10000;
			msg_visible_ingame_confirm <= 3'b110;
			black_on_top <= 0;
		end 
		/*********** Initial end ************/
		else if (chipselect && write) begin
			case (address)
				3'h0 : begin
					// Reset command
					if(writedata==16'hffff) begin
						for(int i=0;i<15;i++) begin
							for(int j=0;j<15;j++) begin
								board[i][j] <= 3'b0;
							end
						end
						selected_y = 4'h7;
						selected_x = 4'h7;
						last_piece_x = 4'hf;
						last_piece_y = 4'hf;
					end
					else begin // Display options
						is_menu <= writedata[0]; 
						is_cursor_visible <= writedata[1];
						//black_on_top <= writedata[1];
					end
				end
				3'h1 : board[writedata[3:0]][writedata[7:4]][2:0] <= writedata[10:8];		
				3'h2 : {selected_y,selected_x,last_piece_y,last_piece_x} <= writedata[15:0];
				3'h3 : begin
					case(writedata[15:10])
						6'h1:msg_visible_menu <= writedata[5:0];
						6'h2:msg_visible_ingame_options <= writedata[3:0];
						6'h3:msg_visible_ingame_players <= writedata[2:0];
						6'h4:msg_visible_ingame_message <= writedata[4:0];
						6'h5:msg_visible_ingame_confirm <= writedata[2:0];
						default:;
					endcase
				end
				3'h4 : msg_selected <= writedata;
				3'h5 : cursor_h <= ((writedata>2)?writedata:2);
				3'h6 : cursor_v <= ((writedata>2)?writedata:2);
				// 4'h4 : circle_x[7:0] <= writedata;
				// 4'h5 : circle_y[15:8] <= writedata;
				// 4'h6 : circle_y[7:0] <= writedata;
				// 4'h7 : circle_radius[7:0] <= writedata;
				// 4'h8 : background_r <= writedata;
				// 4'h9 : background_g <= writedata;
				// 4'ha : background_b <= writedata;
				default:;
			endcase
		end

	/*******************************************************/
	/************** Main Display Logic *********************/
	/*******************************************************/

   always_comb begin
		{VGA_R, VGA_G, VGA_B} = 24'h0;
		/* Piece Image address */
		piece_addr = piece_v*33+piece_h;
		icon_h = 0;
		icon_v = 0;
		if(is_piece_top) begin
			icon_h = hcount[10:1] - 510;
			icon_v = vcount[9:0] - 137;
			piece_addr = icon_v*33+icon_h;
		end
		else if (is_piece_bottom) begin
			icon_h = hcount[10:1] - 510;
			icon_v = vcount[9:0] - 304;
			piece_addr = icon_v*33+icon_h;
		end
		/* p2_profile palette */
		if(p2_profile_cat)
			p2_profile_palette = hcount[10:1]?cat_rgb[7:4]:cat_rgb[3:0];
		else
			p2_profile_palette = hcount[10:1]?dog_rgb[7:4]:dog_rgb[3:0];
		
		/************** Draw Graphics *************/
		if (VGA_BLANK_n) begin
			/*************** Cursor ***************/
			if(is_cursor_area && is_cursor_visible) begin
				{VGA_R, VGA_G, VGA_B} = 24'hf5a6d0;
			end
			/************** Menu Page *************/
			else if (is_menu) begin //draw menu 菜单
				{VGA_R, VGA_G, VGA_B} = 24'h00DDAA; //菜单背景颜色
				// Draw menu message
				if(msg_display_menu && font_val[font_pix_idx]) begin
					// Font selected			
					if(cur_msg_selected)
						{VGA_R, VGA_G, VGA_B} = 24'h00ffff;
					else
						{VGA_R, VGA_G, VGA_B} = 24'h000000;
				end
			end
			/************** Board Page *************/
			else if(is_board) begin
				/************** Message Area *************/
				if(is_message_area) begin
					if (is_message) begin
						{VGA_R, VGA_G, VGA_B} = 24'he4e6ed;  // message background: light grey

						// Draw ingame_message
						if(msg_display_ingame_message && font_val[font_pix_idx]) begin			
							{VGA_R, VGA_G, VGA_B} = 24'h000000;
						end
						// Draw ingame_confirm message
						else if(msg_display_ingame_confirm && font_val[font_pix_idx]) begin
							// Font selected			
							if(cur_msg_selected)
								{VGA_R, VGA_G, VGA_B} = 24'h00ffff;
							else
								{VGA_R, VGA_G, VGA_B} = 24'h000000;
						end

					end
					
				end
				/************** Board Area *************/
				// Draw selected border
				else if(is_selected_area && (piece_x==selected_x) && (piece_y==selected_y)) begin
					{VGA_R, VGA_G, VGA_B} = 24'hffffff;
				end
				// Draw last piece mark
				else if (is_last_piece_area && (piece_x==last_piece_x) && (piece_y==last_piece_y)) begin
					{VGA_R, VGA_G, VGA_B} = 24'h256bd3;
				end
				// Draw White Piece, but don't draw transparent
				// Or draw player piece icon
				else if((piece_info[2]==1'b1 && is_piece_area) || (is_white_icon_area) && white_rgb!=8'h00) begin
					case(white_rgb)
						8'h00 : {VGA_R, VGA_G, VGA_B} = 24'h000000;
						8'h01 : {VGA_R, VGA_G, VGA_B} = 24'hcecece;
						8'h02 : {VGA_R, VGA_G, VGA_B} = 24'hc5c5c5;
						8'h03 : {VGA_R, VGA_G, VGA_B} = 24'hd8d8d8;
						8'h04 : {VGA_R, VGA_G, VGA_B} = 24'hdfdfdf;
						8'h05 : {VGA_R, VGA_G, VGA_B} = 24'hdcdcdc;
						8'h06 : {VGA_R, VGA_G, VGA_B} = 24'hc6c6c6;
						8'h07 : {VGA_R, VGA_G, VGA_B} = 24'ha8a8a8;
						8'h08 : {VGA_R, VGA_G, VGA_B} = 24'h8a8a8a;
						8'h09 : {VGA_R, VGA_G, VGA_B} = 24'hbababa;
						8'h0a : {VGA_R, VGA_G, VGA_B} = 24'he7e7e7;
						8'h0b : {VGA_R, VGA_G, VGA_B} = 24'hdadada;
						8'h0c : {VGA_R, VGA_G, VGA_B} = 24'hdedede;
						8'h0d : {VGA_R, VGA_G, VGA_B} = 24'he0e0e0;
						8'h0e : {VGA_R, VGA_G, VGA_B} = 24'hc7c7c7;
						8'h0f : {VGA_R, VGA_G, VGA_B} = 24'hc8c8c8;
						8'h10 : {VGA_R, VGA_G, VGA_B} = 24'hd2d2d2;
						8'h11 : {VGA_R, VGA_G, VGA_B} = 24'hcccccc;
						8'h12 : {VGA_R, VGA_G, VGA_B} = 24'ha2a2a2;
						8'h13 : {VGA_R, VGA_G, VGA_B} = 24'h777777;
						8'h14 : {VGA_R, VGA_G, VGA_B} = 24'he9e9e9;
						8'h15 : {VGA_R, VGA_G, VGA_B} = 24'he4e4e4;
						8'h16 : {VGA_R, VGA_G, VGA_B} = 24'hd5d5d5;
						8'h17 : {VGA_R, VGA_G, VGA_B} = 24'hd9d9d9;
						8'h18 : {VGA_R, VGA_G, VGA_B} = 24'hc4c4c4;
						8'h19 : {VGA_R, VGA_G, VGA_B} = 24'hd7d7d7;
						8'h1a : {VGA_R, VGA_G, VGA_B} = 24'hd4d4d4;
						8'h1b : {VGA_R, VGA_G, VGA_B} = 24'hcacaca;
						8'h1c : {VGA_R, VGA_G, VGA_B} = 24'ha4a4a4;
						8'h1d : {VGA_R, VGA_G, VGA_B} = 24'hdddddd;
						8'h1e : {VGA_R, VGA_G, VGA_B} = 24'hececec;
						8'h1f : {VGA_R, VGA_G, VGA_B} = 24'hdbdbdb;
						8'h20 : {VGA_R, VGA_G, VGA_B} = 24'hebebeb;
						8'h21 : {VGA_R, VGA_G, VGA_B} = 24'he2e2e2;
						8'h22 : {VGA_R, VGA_G, VGA_B} = 24'hd3d3d3;
						8'h23 : {VGA_R, VGA_G, VGA_B} = 24'hcfcfcf;
						8'h24 : {VGA_R, VGA_G, VGA_B} = 24'hd1d1d1;
						8'h25 : {VGA_R, VGA_G, VGA_B} = 24'hbbbbbb;
						8'h26 : {VGA_R, VGA_G, VGA_B} = 24'he8e8e8;
						8'h27 : {VGA_R, VGA_G, VGA_B} = 24'hededed;
						8'h28 : {VGA_R, VGA_G, VGA_B} = 24'he5e5e5;
						8'h29 : {VGA_R, VGA_G, VGA_B} = 24'hf2f2f2;
						8'h2a : {VGA_R, VGA_G, VGA_B} = 24'hd0d0d0;
						8'h2b : {VGA_R, VGA_G, VGA_B} = 24'hb7b7b7;
						8'h2c : {VGA_R, VGA_G, VGA_B} = 24'hf6f6f6;
						8'h2d : {VGA_R, VGA_G, VGA_B} = 24'hf8f8f8;
						8'h2e : {VGA_R, VGA_G, VGA_B} = 24'hf1f1f1;
						8'h2f : {VGA_R, VGA_G, VGA_B} = 24'hfafafa;
						8'h30 : {VGA_R, VGA_G, VGA_B} = 24'hd6d6d6;
						8'h31 : {VGA_R, VGA_G, VGA_B} = 24'hc1c1c1;
						8'h32 : {VGA_R, VGA_G, VGA_B} = 24'hc3c3c3;
						8'h33 : {VGA_R, VGA_G, VGA_B} = 24'hbdbdbd;
						8'h34 : {VGA_R, VGA_G, VGA_B} = 24'hc9c9c9;
						8'h35 : {VGA_R, VGA_G, VGA_B} = 24'hffffff;
						8'h36 : {VGA_R, VGA_G, VGA_B} = 24'hfefefe;
						8'h37 : {VGA_R, VGA_G, VGA_B} = 24'hfcfcfc;
						8'h38 : {VGA_R, VGA_G, VGA_B} = 24'hf9f9f9;
						8'h39 : {VGA_R, VGA_G, VGA_B} = 24'hefefef;
						8'h3a : {VGA_R, VGA_G, VGA_B} = 24'he1e1e1;
						8'h3b : {VGA_R, VGA_G, VGA_B} = 24'hbcbcbc;
						8'h3c : {VGA_R, VGA_G, VGA_B} = 24'hb5b5b5;
						8'h3d : {VGA_R, VGA_G, VGA_B} = 24'h898989;
						8'h3e : {VGA_R, VGA_G, VGA_B} = 24'hcdcdcd;
						8'h3f : {VGA_R, VGA_G, VGA_B} = 24'hf0f0f0;
						8'h40 : {VGA_R, VGA_G, VGA_B} = 24'hf7f7f7;
						8'h41 : {VGA_R, VGA_G, VGA_B} = 24'hfbfbfb;
						8'h42 : {VGA_R, VGA_G, VGA_B} = 24'h7a7a7a;
						8'h43 : {VGA_R, VGA_G, VGA_B} = 24'ha7a7a7;
						8'h44 : {VGA_R, VGA_G, VGA_B} = 24'hbebebe;
						8'h45 : {VGA_R, VGA_G, VGA_B} = 24'ha3a3a3;
						8'h46 : {VGA_R, VGA_G, VGA_B} = 24'h363636;
						8'h47 : {VGA_R, VGA_G, VGA_B} = 24'he3e3e3;
						8'h48 : {VGA_R, VGA_G, VGA_B} = 24'hcbcbcb;
						8'h49 : {VGA_R, VGA_G, VGA_B} = 24'hc0c0c0;
						8'h4a : {VGA_R, VGA_G, VGA_B} = 24'hb2b2b2;
						8'h4b : {VGA_R, VGA_G, VGA_B} = 24'hb0b0b0;
						8'h4c : {VGA_R, VGA_G, VGA_B} = 24'h565656;
						8'h4d : {VGA_R, VGA_G, VGA_B} = 24'hb9b9b9;
						8'h4e : {VGA_R, VGA_G, VGA_B} = 24'h676767;
						8'h4f : {VGA_R, VGA_G, VGA_B} = 24'h353535;
						8'h50 : {VGA_R, VGA_G, VGA_B} = 24'heaeaea;
						8'h51 : {VGA_R, VGA_G, VGA_B} = 24'he6e6e6;
						8'h52 : {VGA_R, VGA_G, VGA_B} = 24'hc2c2c2;
						8'h53 : {VGA_R, VGA_G, VGA_B} = 24'h6f6f6f;
						8'h54 : {VGA_R, VGA_G, VGA_B} = 24'h313131;
						8'h55 : {VGA_R, VGA_G, VGA_B} = 24'hb8b8b8;
						8'h56 : {VGA_R, VGA_G, VGA_B} = 24'hb6b6b6;
						8'h57 : {VGA_R, VGA_G, VGA_B} = 24'h797979;
						8'h58 : {VGA_R, VGA_G, VGA_B} = 24'h252525;
						8'h59 : {VGA_R, VGA_G, VGA_B} = 24'hbfbfbf;
						8'h5a : {VGA_R, VGA_G, VGA_B} = 24'hb3b3b3;
						8'h5b : {VGA_R, VGA_G, VGA_B} = 24'h787878;
						8'h5c : {VGA_R, VGA_G, VGA_B} = 24'h222222;
						8'h5d : {VGA_R, VGA_G, VGA_B} = 24'h6c6c6c;
						8'h5e : {VGA_R, VGA_G, VGA_B} = 24'h272727;
						8'h5f : {VGA_R, VGA_G, VGA_B} = 24'hadadad;
						8'h60 : {VGA_R, VGA_G, VGA_B} = 24'hababab;
						8'h61 : {VGA_R, VGA_G, VGA_B} = 24'h5d5d5d;
						8'h62 : {VGA_R, VGA_G, VGA_B} = 24'h292929;
						8'h63 : {VGA_R, VGA_G, VGA_B} = 24'h9e9e9e;
						8'h64 : {VGA_R, VGA_G, VGA_B} = 24'hb4b4b4;
						8'h65 : {VGA_R, VGA_G, VGA_B} = 24'ha6a6a6;
						8'h66 : {VGA_R, VGA_G, VGA_B} = 24'h404040;
						8'h67 : {VGA_R, VGA_G, VGA_B} = 24'h323232;
						8'h68 : {VGA_R, VGA_G, VGA_B} = 24'hafafaf;
						8'h69 : {VGA_R, VGA_G, VGA_B} = 24'h8c8c8c;
						8'h6a : {VGA_R, VGA_G, VGA_B} = 24'h2e2e2e;
						8'h6b : {VGA_R, VGA_G, VGA_B} = 24'h333333;
						8'h6c : {VGA_R, VGA_G, VGA_B} = 24'haeaeae;
						8'h6d : {VGA_R, VGA_G, VGA_B} = 24'hacacac;
						8'h6e : {VGA_R, VGA_G, VGA_B} = 24'ha9a9a9;
						8'h6f : {VGA_R, VGA_G, VGA_B} = 24'h5c5c5c;
						8'h70 : {VGA_R, VGA_G, VGA_B} = 24'h2f2f2f;
						8'h71 : {VGA_R, VGA_G, VGA_B} = 24'h8e8e8e;
						8'h72 : {VGA_R, VGA_G, VGA_B} = 24'hb1b1b1;
						8'h73 : {VGA_R, VGA_G, VGA_B} = 24'ha0a0a0;
						8'h74 : {VGA_R, VGA_G, VGA_B} = 24'h515151;
						8'h75 : {VGA_R, VGA_G, VGA_B} = 24'h2c2c2c;
						8'h76 : {VGA_R, VGA_G, VGA_B} = 24'haaaaaa;
						8'h77 : {VGA_R, VGA_G, VGA_B} = 24'h686868;
						8'h78 : {VGA_R, VGA_G, VGA_B} = 24'h2d2d2d;
						8'h79 : {VGA_R, VGA_G, VGA_B} = 24'h303030;
						8'h7a : {VGA_R, VGA_G, VGA_B} = 24'h8f8f8f;
						8'h7b : {VGA_R, VGA_G, VGA_B} = 24'h909090;
						8'h7c : {VGA_R, VGA_G, VGA_B} = 24'h2b2b2b;
						8'h7d : {VGA_R, VGA_G, VGA_B} = 24'h525252;
						8'h7e : {VGA_R, VGA_G, VGA_B} = 24'ha5a5a5;
						8'h7f : {VGA_R, VGA_G, VGA_B} = 24'h8d8d8d;
						8'h80 : {VGA_R, VGA_G, VGA_B} = 24'h585858;
						8'h81 : {VGA_R, VGA_G, VGA_B} = 24'h373737;
						8'h82 : {VGA_R, VGA_G, VGA_B} = 24'h282828;
						8'h83 : {VGA_R, VGA_G, VGA_B} = 24'h737373;
						8'h84 : {VGA_R, VGA_G, VGA_B} = 24'h5e5e5e;
						8'h85 : {VGA_R, VGA_G, VGA_B} = 24'h424242;
						default:{VGA_R, VGA_G, VGA_B} = 24'hffffff;
					endcase
				end
				// Draw Black Piece
				// Or draw player piece icon
				else if((piece_info[1]==1'b1 && is_piece_area) || (is_black_icon_area) && black_rgb!=8'h00) begin
					case(black_rgb)
						8'h00 : {VGA_R, VGA_G, VGA_B} = 24'h000000;
						8'h01 : {VGA_R, VGA_G, VGA_B} = 24'h535353;
						8'h02 : {VGA_R, VGA_G, VGA_B} = 24'h4b4b4b;
						8'h03 : {VGA_R, VGA_G, VGA_B} = 24'h454545;
						8'h04 : {VGA_R, VGA_G, VGA_B} = 24'h3d3d3d;
						8'h05 : {VGA_R, VGA_G, VGA_B} = 24'h373737;
						8'h06 : {VGA_R, VGA_G, VGA_B} = 24'h343434;
						8'h07 : {VGA_R, VGA_G, VGA_B} = 24'h323232;
						8'h08 : {VGA_R, VGA_G, VGA_B} = 24'h272727;
						8'h09 : {VGA_R, VGA_G, VGA_B} = 24'h5e5e5e;
						8'h0a : {VGA_R, VGA_G, VGA_B} = 24'h5c5c5c;
						8'h0b : {VGA_R, VGA_G, VGA_B} = 24'h575757;
						8'h0c : {VGA_R, VGA_G, VGA_B} = 24'h515151;
						8'h0d : {VGA_R, VGA_G, VGA_B} = 24'h4a4a4a;
						8'h0e : {VGA_R, VGA_G, VGA_B} = 24'h424242;
						8'h0f : {VGA_R, VGA_G, VGA_B} = 24'h383838;
						8'h10 : {VGA_R, VGA_G, VGA_B} = 24'h2f2f2f;
						8'h11 : {VGA_R, VGA_G, VGA_B} = 24'h282828;
						8'h12 : {VGA_R, VGA_G, VGA_B} = 24'h202020;
						8'h13 : {VGA_R, VGA_G, VGA_B} = 24'h1c1c1c;
						8'h14 : {VGA_R, VGA_G, VGA_B} = 24'h666666;
						8'h15 : {VGA_R, VGA_G, VGA_B} = 24'h6b6b6b;
						8'h16 : {VGA_R, VGA_G, VGA_B} = 24'h6f6f6f;
						8'h17 : {VGA_R, VGA_G, VGA_B} = 24'h6e6e6e;
						8'h18 : {VGA_R, VGA_G, VGA_B} = 24'h676767;
						8'h19 : {VGA_R, VGA_G, VGA_B} = 24'h606060;
						8'h1a : {VGA_R, VGA_G, VGA_B} = 24'h585858;
						8'h1b : {VGA_R, VGA_G, VGA_B} = 24'h4e4e4e;
						8'h1c : {VGA_R, VGA_G, VGA_B} = 24'h444444;
						8'h1d : {VGA_R, VGA_G, VGA_B} = 24'h393939;
						8'h1e : {VGA_R, VGA_G, VGA_B} = 24'h252525;
						8'h1f : {VGA_R, VGA_G, VGA_B} = 24'h161616;
						8'h20 : {VGA_R, VGA_G, VGA_B} = 24'h1f1f1f;
						8'h21 : {VGA_R, VGA_G, VGA_B} = 24'h777777;
						8'h22 : {VGA_R, VGA_G, VGA_B} = 24'h7a7a7a;
						8'h23 : {VGA_R, VGA_G, VGA_B} = 24'h7d7d7d;
						8'h24 : {VGA_R, VGA_G, VGA_B} = 24'h767676;
						8'h25 : {VGA_R, VGA_G, VGA_B} = 24'h656565;
						8'h26 : {VGA_R, VGA_G, VGA_B} = 24'h5a5a5a;
						8'h27 : {VGA_R, VGA_G, VGA_B} = 24'h363636;
						8'h28 : {VGA_R, VGA_G, VGA_B} = 24'h2a2a2a;
						8'h29 : {VGA_R, VGA_G, VGA_B} = 24'h212121;
						8'h2a : {VGA_R, VGA_G, VGA_B} = 24'h181818;
						8'h2b : {VGA_R, VGA_G, VGA_B} = 24'h111111;
						8'h2c : {VGA_R, VGA_G, VGA_B} = 24'h0d0d0d;
						8'h2d : {VGA_R, VGA_G, VGA_B} = 24'h707070;
						8'h2e : {VGA_R, VGA_G, VGA_B} = 24'h7c7c7c;
						8'h2f : {VGA_R, VGA_G, VGA_B} = 24'h828282;
						8'h30 : {VGA_R, VGA_G, VGA_B} = 24'h888888;
						8'h31 : {VGA_R, VGA_G, VGA_B} = 24'h8b8b8b;
						8'h32 : {VGA_R, VGA_G, VGA_B} = 24'h646464;
						8'h33 : {VGA_R, VGA_G, VGA_B} = 24'h494949;
						8'h34 : {VGA_R, VGA_G, VGA_B} = 24'h3c3c3c;
						8'h35 : {VGA_R, VGA_G, VGA_B} = 24'h303030;
						8'h36 : {VGA_R, VGA_G, VGA_B} = 24'h1b1b1b;
						8'h37 : {VGA_R, VGA_G, VGA_B} = 24'h131313;
						8'h38 : {VGA_R, VGA_G, VGA_B} = 24'h0c0c0c;
						8'h39 : {VGA_R, VGA_G, VGA_B} = 24'h090909;
						8'h3a : {VGA_R, VGA_G, VGA_B} = 24'h6d6d6d;
						8'h3b : {VGA_R, VGA_G, VGA_B} = 24'h868686;
						8'h3c : {VGA_R, VGA_G, VGA_B} = 24'h8f8f8f;
						8'h3d : {VGA_R, VGA_G, VGA_B} = 24'h959595;
						8'h3e : {VGA_R, VGA_G, VGA_B} = 24'h989898;
						8'h3f : {VGA_R, VGA_G, VGA_B} = 24'h949494;
						8'h40 : {VGA_R, VGA_G, VGA_B} = 24'h8d8d8d;
						8'h41 : {VGA_R, VGA_G, VGA_B} = 24'h858585;
						8'h42 : {VGA_R, VGA_G, VGA_B} = 24'h797979;
						8'h43 : {VGA_R, VGA_G, VGA_B} = 24'h6c6c6c;
						8'h44 : {VGA_R, VGA_G, VGA_B} = 24'h5f5f5f;
						8'h45 : {VGA_R, VGA_G, VGA_B} = 24'h505050;
						8'h46 : {VGA_R, VGA_G, VGA_B} = 24'h353535;
						8'h47 : {VGA_R, VGA_G, VGA_B} = 24'h292929;
						8'h48 : {VGA_R, VGA_G, VGA_B} = 24'h1e1e1e;
						8'h49 : {VGA_R, VGA_G, VGA_B} = 24'h151515;
						8'h4a : {VGA_R, VGA_G, VGA_B} = 24'h0e0e0e;
						8'h4b : {VGA_R, VGA_G, VGA_B} = 24'h080808;
						8'h4c : {VGA_R, VGA_G, VGA_B} = 24'h818181;
						8'h4d : {VGA_R, VGA_G, VGA_B} = 24'h9e9e9e;
						8'h4e : {VGA_R, VGA_G, VGA_B} = 24'ha1a1a1;
						8'h4f : {VGA_R, VGA_G, VGA_B} = 24'ha0a0a0;
						8'h50 : {VGA_R, VGA_G, VGA_B} = 24'h9c9c9c;
						8'h51 : {VGA_R, VGA_G, VGA_B} = 24'h8c8c8c;
						8'h52 : {VGA_R, VGA_G, VGA_B} = 24'h7f7f7f;
						8'h53 : {VGA_R, VGA_G, VGA_B} = 24'h737373;
						8'h54 : {VGA_R, VGA_G, VGA_B} = 24'h555555;
						8'h55 : {VGA_R, VGA_G, VGA_B} = 24'h464646;
						8'h56 : {VGA_R, VGA_G, VGA_B} = 24'h2b2b2b;
						8'h57 : {VGA_R, VGA_G, VGA_B} = 24'h171717;
						8'h58 : {VGA_R, VGA_G, VGA_B} = 24'h101010;
						8'h59 : {VGA_R, VGA_G, VGA_B} = 24'h0a0a0a;
						8'h5a : {VGA_R, VGA_G, VGA_B} = 24'h060606;
						8'h5b : {VGA_R, VGA_G, VGA_B} = 24'h5d5d5d;
						8'h5c : {VGA_R, VGA_G, VGA_B} = 24'ha4a4a4;
						8'h5d : {VGA_R, VGA_G, VGA_B} = 24'ha7a7a7;
						8'h5e : {VGA_R, VGA_G, VGA_B} = 24'ha6a6a6;
						8'h5f : {VGA_R, VGA_G, VGA_B} = 24'ha2a2a2;
						8'h60 : {VGA_R, VGA_G, VGA_B} = 24'h9b9b9b;
						8'h61 : {VGA_R, VGA_G, VGA_B} = 24'h909090;
						8'h62 : {VGA_R, VGA_G, VGA_B} = 24'h838383;
						8'h63 : {VGA_R, VGA_G, VGA_B} = 24'h484848;
						8'h64 : {VGA_R, VGA_G, VGA_B} = 24'h3a3a3a;
						8'h65 : {VGA_R, VGA_G, VGA_B} = 24'h2d2d2d;
						8'h66 : {VGA_R, VGA_G, VGA_B} = 24'h222222;
						8'h67 : {VGA_R, VGA_G, VGA_B} = 24'h050505;
						8'h68 : {VGA_R, VGA_G, VGA_B} = 24'h232323;
						8'h69 : {VGA_R, VGA_G, VGA_B} = 24'ha9a9a9;
						8'h6a : {VGA_R, VGA_G, VGA_B} = 24'ha8a8a8;
						8'h6b : {VGA_R, VGA_G, VGA_B} = 24'ha3a3a3;
						8'h6c : {VGA_R, VGA_G, VGA_B} = 24'h919191;
						8'h6d : {VGA_R, VGA_G, VGA_B} = 24'h848484;
						8'h6e : {VGA_R, VGA_G, VGA_B} = 24'h686868;
						8'h6f : {VGA_R, VGA_G, VGA_B} = 24'h020202;
						8'h70 : {VGA_R, VGA_G, VGA_B} = 24'h757575;
						8'h71 : {VGA_R, VGA_G, VGA_B} = 24'h9d9d9d;
						8'h72 : {VGA_R, VGA_G, VGA_B} = 24'h969696;
						8'h73 : {VGA_R, VGA_G, VGA_B} = 24'h717171;
						8'h74 : {VGA_R, VGA_G, VGA_B} = 24'h626262;
						8'h75 : {VGA_R, VGA_G, VGA_B} = 24'h0f0f0f;
						8'h76 : {VGA_R, VGA_G, VGA_B} = 24'h787878;
						8'h77 : {VGA_R, VGA_G, VGA_B} = 24'h404040;
						8'h78 : {VGA_R, VGA_G, VGA_B} = 24'h333333;
						8'h79 : {VGA_R, VGA_G, VGA_B} = 24'h1d1d1d;
						8'h7a : {VGA_R, VGA_G, VGA_B} = 24'h030303;
						8'h7b : {VGA_R, VGA_G, VGA_B} = 24'h7b7b7b;
						8'h7c : {VGA_R, VGA_G, VGA_B} = 24'h636363;
						8'h7d : {VGA_R, VGA_G, VGA_B} = 24'h2e2e2e;
						8'h7e : {VGA_R, VGA_G, VGA_B} = 24'h242424;
						8'h7f : {VGA_R, VGA_G, VGA_B} = 24'h1a1a1a;
						8'h80 : {VGA_R, VGA_G, VGA_B} = 24'h121212;
						8'h81 : {VGA_R, VGA_G, VGA_B} = 24'h040404;
						8'h82 : {VGA_R, VGA_G, VGA_B} = 24'h000000;
						8'h83 : {VGA_R, VGA_G, VGA_B} = 24'h3e3e3e;
						8'h84 : {VGA_R, VGA_G, VGA_B} = 24'h808080;
						8'h85 : {VGA_R, VGA_G, VGA_B} = 24'h595959;
						8'h86 : {VGA_R, VGA_G, VGA_B} = 24'h4d4d4d;
						8'h87 : {VGA_R, VGA_G, VGA_B} = 24'h010101;
						8'h88 : {VGA_R, VGA_G, VGA_B} = 24'h313131;
						8'h89 : {VGA_R, VGA_G, VGA_B} = 24'h262626;
						8'h8a : {VGA_R, VGA_G, VGA_B} = 24'h0b0b0b;
						8'h8b : {VGA_R, VGA_G, VGA_B} = 24'h070707;
						8'h8c : {VGA_R, VGA_G, VGA_B} = 24'h474747;
						8'h8d : {VGA_R, VGA_G, VGA_B} = 24'h191919;
						default:{VGA_R, VGA_G, VGA_B} = 24'hffffff;
					endcase
				end
				// Draw board
				else if(is_board_area) begin
					case(bg_palette)
						4'h0 : {VGA_R, VGA_G, VGA_B} = 24'h816951;
						4'h1 : {VGA_R, VGA_G, VGA_B} = 24'ha0866d;
						4'h2 : {VGA_R, VGA_G, VGA_B} = 24'hc9a081;
						4'h3 : {VGA_R, VGA_G, VGA_B} = 24'hc6ab91;
						4'h4 : {VGA_R, VGA_G, VGA_B} = 24'hd3b69a;
						4'h5 : {VGA_R, VGA_G, VGA_B} = 24'hd9bb9d;
						4'h6 : {VGA_R, VGA_G, VGA_B} = 24'hdcc0a5;
						4'h7 : {VGA_R, VGA_G, VGA_B} = 24'hd7b694;
						4'h8 : {VGA_R, VGA_G, VGA_B} = 24'h513f29;
						4'h9 : {VGA_R, VGA_G, VGA_B} = 24'hd4b191;
						4'ha : {VGA_R, VGA_G, VGA_B} = 24'hd1ac89;
						4'hb : {VGA_R, VGA_G, VGA_B} = 24'hd0a988;
						4'hc : {VGA_R, VGA_G, VGA_B} = 24'hd2ae8b;
						4'hd : {VGA_R, VGA_G, VGA_B} = 24'hcca383;
						4'he : {VGA_R, VGA_G, VGA_B} = 24'hcfa585;
						default:{VGA_R, VGA_G, VGA_B} = 24'hffffff;
					endcase
				end
				/************** Player UI Area *************/
				// Player UI Division line
				else if (division_line) begin
					{VGA_R, VGA_G, VGA_B} = 24'h000000;  // Black color
				end
				/*else if(is_player2_area)begin
					{VGA_R, VGA_G, VGA_B} = 24'hE8E8E3; //菜单背景颜色
					// display P2 or AI
					if((msg_display_ingame_players[2] || msg_display_ingame_players[3]) && font_val[font_pix_idx]) begin			
						{VGA_R, VGA_G, VGA_B} = 24'h000000; //默认字体黑色
					end
					else
						{VGA_R, VGA_G, VGA_B} = 24'hffffff;
				end */
				// Draw profile image
				else if(is_p1_profile_area)
					/* Panda palette */
					case(p1_profile_palette)
						4'h0 : {VGA_R, VGA_G, VGA_B} = 24'h111111;
						4'h1 : {VGA_R, VGA_G, VGA_B} = 24'h49484e;
						4'h2 : {VGA_R, VGA_G, VGA_B} = 24'hfefbf5;
						4'h3 : {VGA_R, VGA_G, VGA_B} = 24'h616160;
						4'h4 : {VGA_R, VGA_G, VGA_B} = 24'hfdede4;
						4'h5 : {VGA_R, VGA_G, VGA_B} = 24'h15340e;
						4'h6 : {VGA_R, VGA_G, VGA_B} = 24'hadadb0;
						4'h7 : {VGA_R, VGA_G, VGA_B} = 24'hdacbbe;
						4'h8 : {VGA_R, VGA_G, VGA_B} = 24'h838588;
						4'h9 : {VGA_R, VGA_G, VGA_B} = 24'h00833b;
						4'ha : {VGA_R, VGA_G, VGA_B} = 24'h004719;
						default:{VGA_R, VGA_G, VGA_B} = 24'hffffff;
					endcase
				else if(is_p2_profile_area) begin
					if(p2_profile_cat)
					/* Cat palette */
					case(p2_profile_palette)
						4'h0 : {VGA_R, VGA_G, VGA_B} = 24'hded6c6;
						4'h1 : {VGA_R, VGA_G, VGA_B} = 24'hd8cdb9;
						4'h2 : {VGA_R, VGA_G, VGA_B} = 24'he2bd85;
						4'h3 : {VGA_R, VGA_G, VGA_B} = 24'hd2a25a;
						4'h4 : {VGA_R, VGA_G, VGA_B} = 24'hd69d4a;
						4'h5 : {VGA_R, VGA_G, VGA_B} = 24'hd59833;
						4'h6 : {VGA_R, VGA_G, VGA_B} = 24'hcb9d50;
						4'h7 : {VGA_R, VGA_G, VGA_B} = 24'hbd9a75;
						4'h8 : {VGA_R, VGA_G, VGA_B} = 24'hd69d3f;
						4'h9 : {VGA_R, VGA_G, VGA_B} = 24'hc3904d;
						4'ha : {VGA_R, VGA_G, VGA_B} = 24'hc08748;
						4'hb : {VGA_R, VGA_G, VGA_B} = 24'hb87f36;
						4'hc : {VGA_R, VGA_G, VGA_B} = 24'hd86353;
						4'hd : {VGA_R, VGA_G, VGA_B} = 24'h985321;
						4'he : {VGA_R, VGA_G, VGA_B} = 24'h58402e;
						4'hf : {VGA_R, VGA_G, VGA_B} = 24'h1d1715;
					endcase
					else
					/* Dog palette */
					case(p2_profile_palette)
						4'h0 : {VGA_R, VGA_G, VGA_B} = 24'he1d1bd;
						4'h1 : {VGA_R, VGA_G, VGA_B} = 24'hdfb771;
						4'h2 : {VGA_R, VGA_G, VGA_B} = 24'hd7ad64;
						4'h3 : {VGA_R, VGA_G, VGA_B} = 24'hddbd83;
						4'h4 : {VGA_R, VGA_G, VGA_B} = 24'hcc9b53;
						4'h5 : {VGA_R, VGA_G, VGA_B} = 24'hc28b3e;
						4'h6 : {VGA_R, VGA_G, VGA_B} = 24'hb59770;
						4'h7 : {VGA_R, VGA_G, VGA_B} = 24'ha97230;
						4'h8 : {VGA_R, VGA_G, VGA_B} = 24'ha5835b;
						4'h9 : {VGA_R, VGA_G, VGA_B} = 24'h967551;
						4'ha : {VGA_R, VGA_G, VGA_B} = 24'h8d6c49;
						4'hb : {VGA_R, VGA_G, VGA_B} = 24'h4c86b4;
						4'hc : {VGA_R, VGA_G, VGA_B} = 24'h935a1c;
						4'hd : {VGA_R, VGA_G, VGA_B} = 24'h715944;
						4'he : {VGA_R, VGA_G, VGA_B} = 24'h67462a;
						4'hf : {VGA_R, VGA_G, VGA_B} = 24'h191412;
					endcase
				end
				// draw in-game options
				else if(is_option_area) begin
					{VGA_R, VGA_G, VGA_B} = 24'h00DDAA; //菜单背景颜色
					// Draw ingame_options message
					if(msg_display_ingame_options && font_val[font_pix_idx]) begin
						// Font selected			
						if(cur_msg_selected)
							{VGA_R, VGA_G, VGA_B} = 24'h00ffff;
						else
							{VGA_R, VGA_G, VGA_B} = 24'h000000;
					end
				end
				// Player UI Message & Background
				else if(is_player1_area || is_player2_area) begin
					{VGA_R, VGA_G, VGA_B} = 24'h96968C; //菜单背景颜色
					// Draw ingame_players message
					if(msg_display_ingame_players && font_val[font_pix_idx]) begin			
						{VGA_R, VGA_G, VGA_B} = 24'h000000;
					end
				end
			end
		     end
		end
endmodule

module vga_counters(
input  logic 		clk50, reset,
output logic [10:0]	hcount,  // hcount[10:1] is pixel columnvga_piece_painter
output logic [9:0]	vcount,  // vcount[9:0] is pixel row
output logic 		VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);

	/*
	* 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
	* 
	* HCOUNT 1599 0             1279       1599 0
	*             _______________              ________
	* ___________|    Video      |____________|  Video
	* 
	* 
	* |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
	*       _______________________      _____________
	* |____|       VGA_HS          |____|
	*/
	// Parameters for hcount
	parameter 	HACTIVE      = 11'd 1280,
				HFRONT_PORCH = 11'd 32,
				HSYNC        = 11'd 192,
				HBACK_PORCH  = 11'd 96,   
				HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
								HBACK_PORCH; // 1600

	// Parameters for vcount
	parameter 	VACTIVE      = 10'd 480,
				VFRONT_PORCH = 10'd 10,
				VSYNC        = 10'd 2,
				VBACK_PORCH  = 10'd 33,
				VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
								VBACK_PORCH; // 525

	logic endOfLine;

	always_ff @(posedge clk50 or posedge reset)
		if (reset)          hcount <= 0;
		else if (endOfLine) hcount <= 0;
		else  	         	hcount <= hcount + 11'd 1;

	assign endOfLine = hcount == HTOTAL - 1;

	logic endOfField;

	always_ff @(posedge clk50 or posedge reset)
		if (reset) 			vcount <= 0;
		else if (endOfLine)
			if (endOfField) vcount <= 0;
			else 			vcount <= vcount + 10'd 1;

	assign endOfField = vcount == VTOTAL - 1;

	// Horizontal sync: from 0x520 to 0x5DF (0x57F)
	// 101 0010 0000 to 101 1101 1111
	assign VGA_HS = !( (hcount[10:8] == 3'b101) &
					!(hcount[7:5] == 3'b111));
	assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

	assign VGA_SYNC_n = 1'b0; // For putting sync on the green signal; unused

	// Horizontal active: 0 to 1279     Vertical active: 0 to 479
	// 101 0000 0000  1280	       01 1110 0000  480
	// 110 0011 1111  1599	       10 0000 1100  524
	assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
						 !( vcount[9] | (vcount[8:5] == 4'b1111) );

	/* VGA_CLK is 25 MHz
	*             __    __    __
	* clk50    __|  |__|  |__|
	*        
	*             _____       __
	* hcount[0]__|     |_____|
	*/
	assign VGA_CLK = hcount[0]; // 25 MHz clock: rising edge sensitive

endmodule
