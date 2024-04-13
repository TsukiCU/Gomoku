/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 */

module vga_ball(input logic        clk,
	        input logic 	   reset,
		input logic [7:0]  writedata,
		input logic 	   write,
		input 		   chipselect,
		input logic [3:0]  address,

		output logic [7:0] VGA_R, VGA_G, VGA_B,
		output logic 	   VGA_CLK, VGA_HS, VGA_VS,
		                   VGA_BLANK_n,
		output logic 	   VGA_SYNC_n);

   logic [10:0]	   hcount;
   logic [9:0]     vcount;

   // Index - x,y,info, x is vertial, y is horizontal
   // 0 - draw selected border
   // 1,2 - piece info
   //       00 - no piece
   //       10 - draw white piece
   //       01 - draw black piece
   logic [14:0][14:0][2:0]	board; 
   logic[4:0]	piece_v;
   logic[4:0]	piece_h;
   logic[2:0]	piece_info;
   logic[7:0]	piece_rgb;
   logic[3:0] 	piece_x;
   logic[3:0] 	piece_y;
   logic		is_selected;
   logic		is_piece_area;
   logic		is_board_area;
   logic [7:0] 	background_r, background_g, background_b;
   logic [7:0]	operand;
   logic [6:0][7:0] params;

   logic [17:0] bg_addr;
   logic [7:0] bg_rgb;
   logic [3:0] bg_palette;
   logic [9:0]	piece_addr;
   logic [7:0] black_rgb;
   logic [7:0] white_rgb;

   assign piece_info = board[piece_x][piece_y];
   assign is_board_area = (hcount[10:1]>=10'd70) && (hcount[10:1]<10'd570);
   assign is_piece_area = (hcount[10:1]>=10'd103) && (hcount[10:1]<=10'd532) && (vcount[9:0]>=10'd25) && (vcount[9:0]<=10'd460);
   assign is_selected_area  = (piece_h <= 5'd1) || (piece_h >= 5'd27) || (piece_v <= 5'd1) || (piece_v >= 5'd27) && is_piece_area;
   assign piece_addr = piece_v*29+piece_h;
   assign bg_addr = is_board_area?((vcount[9:0]*17'd500+hcount[10:1]-17'd70)>>1):0;
   assign bg_palette = hcount[1]?bg_rgb[7:4]:bg_rgb[3:0];
   vga_counters counters(.clk50(clk), .*);
   // vga_piece_painter painter(.*);
   soc_system_background background(.address(bg_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(bg_rgb));
   soc_system_white_piece white(.address(piece_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(white_rgb));
   soc_system_black_piece black(.address(piece_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(black_rgb));

   // Set board info
   initial begin
		for(int i=0;i<15;i++) begin
			for(int j=0;j<15;j++) begin
				board[i][j][0] <= (i==7)&&(j==7);
				board[i][j][2:1] <= 2'b0;
			end
		end
		// test
		board[7][7][2]=1'b1;
		board[7][8][1]=1'b1;
		board[7][6][1]=1'b1;
		board[6][7][2]=1'b1;
		board[8][7][1]=1'b1;
   end

   // Set piece counter
   always_ff @(posedge clk) begin
		// piece left 70+33
		if(hcount[10:1]<=10'd103) begin
			piece_h <= 5'd0;
			piece_y <= 4'd0;
		end
		// piece right 570-32
		else if(hcount[10:1]>=10'd532) begin
			piece_h <= 5'd28;
			piece_y <= 4'd15;
		end
		else if(piece_h==5'd28) begin
			piece_y <= piece_y + 4'd1;
			piece_h <= 0;
		end
		else if(hcount[0]==0) begin
			piece_h <= piece_h + 5'd1;
		end
		if(hcount[10:0]==11'd0)
			// piece top 0+25
			if(vcount[9:0]<=10'd25) begin
				piece_v <= 5'd0;
				piece_x <= 4'd0;
			end
			// piece bottom 480-20
			else if(hcount[9:0]>=10'd460) begin
				piece_v <= 5'd28;
				piece_x <= 4'd15;
			end
			else if(piece_v==5'd28) begin
				piece_x <= piece_x + 4'd1;
				piece_v <= 0;
			end
			else begin
				piece_v <= piece_v + 5'd1;
			end
		//piece_addr <= piece_v*29+piece_h;
   end

   always_ff @(posedge clk)
     if (reset) begin
		for(int i=0;i<15;i++) begin
			for(int j=0;j<15;j++) begin
				board[i][j][0] <= (i==7)&&(j==7);
				board[i][j][2:1] <= 2'b0;
			end
		end
		background_r = 8'hF0;
		background_g = 8'hC0;
		background_b = 8'h90;
		// test
		board[7][7][2]=1'b1;
		board[7][8][1]=1'b1;
		board[7][6][1]=1'b1;
		board[6][7][2]=1'b1;
		board[8][7][1]=1'b1;
     end else if (chipselect && write) begin
      case (address)
        4'h0 : operand <= writedata;
        // 4'h1 : circle_g <= writedata;
        // 4'h2 : circle_b <= writedata;
        // 4'h3 : circle_x[15:8] <= writedata;
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
	 else begin
	//   case (operand)
	//   	8'h1 : begin // Take move
	// 		board[params[0]][params[1]][7:6] <= params[2][1:0];
	// 	end
	// 	8'hff: begin // reset
	// 		for(int i=0;i<15;i++) begin
	// 			for(int j=0;j<15;j++) begin
	// 				board[i][j][0] <= (i==7)&&(j==7);
	// 				board[i][j][2:1] <= 2'b0;
	// 			end
	// 		end
	// 	end
	//   endcase
	 end

   /* Graphics*/
   always_comb begin
      {VGA_R, VGA_G, VGA_B} = 24'h0;
      if (VGA_BLANK_n)
	    // Draw selected border
		if(is_selected_area && piece_info[0]) begin
			{VGA_R, VGA_G, VGA_B} <= 24'h000000;
		end
		// Draw White Piece, but don't draw transparent
		else if(piece_info[1]==1'b1 && white_rgb!=8'h00 && is_piece_area) begin
			case(white_rgb)
				8'h00 : {VGA_R, VGA_G, VGA_B} = 24'h000000;
				8'h01 : {VGA_R, VGA_G, VGA_B} = 24'he5e5e3;
				8'h02 : {VGA_R, VGA_G, VGA_B} = 24'he3e3e1;
				8'h03 : {VGA_R, VGA_G, VGA_B} = 24'hcdcdcd;
				8'h04 : {VGA_R, VGA_G, VGA_B} = 24'hecedeb;
				8'h05 : {VGA_R, VGA_G, VGA_B} = 24'hf7f7f7;
				8'h06 : {VGA_R, VGA_G, VGA_B} = 24'hc0c0c0;
				8'h07 : {VGA_R, VGA_G, VGA_B} = 24'hdddcdc;
				8'h08 : {VGA_R, VGA_G, VGA_B} = 24'he4e5e2;
				8'h09 : {VGA_R, VGA_G, VGA_B} = 24'hf2f2f1;
				8'h0a : {VGA_R, VGA_G, VGA_B} = 24'hfafafa;
				8'h0b : {VGA_R, VGA_G, VGA_B} = 24'hfafbfa;
				8'h0c : {VGA_R, VGA_G, VGA_B} = 24'hf6f6f5;
				8'h0d : {VGA_R, VGA_G, VGA_B} = 24'hecedec;
				8'h0e : {VGA_R, VGA_G, VGA_B} = 24'he3e3e0;
				8'h0f : {VGA_R, VGA_G, VGA_B} = 24'he2e2df;
				8'h10 : {VGA_R, VGA_G, VGA_B} = 24'hb6b6b6;
				8'h11 : {VGA_R, VGA_G, VGA_B} = 24'he2e3e0;
				8'h12 : {VGA_R, VGA_G, VGA_B} = 24'hececeb;
				8'h13 : {VGA_R, VGA_G, VGA_B} = 24'hf5f6f5;
				8'h14 : {VGA_R, VGA_G, VGA_B} = 24'hfafaf9;
				8'h15 : {VGA_R, VGA_G, VGA_B} = 24'hf8f8f8;
				8'h16 : {VGA_R, VGA_G, VGA_B} = 24'hf9f8f8;
				8'h17 : {VGA_R, VGA_G, VGA_B} = 24'hf9f9f9;
				8'h18 : {VGA_R, VGA_G, VGA_B} = 24'hf8f9f8;
				8'h19 : {VGA_R, VGA_G, VGA_B} = 24'hf5f5f5;
				8'h1a : {VGA_R, VGA_G, VGA_B} = 24'hededec;
				8'h1b : {VGA_R, VGA_G, VGA_B} = 24'he2e3df;
				8'h1c : {VGA_R, VGA_G, VGA_B} = 24'hdfe0dd;
				8'h1d : {VGA_R, VGA_G, VGA_B} = 24'haeaeae;
				8'h1e : {VGA_R, VGA_G, VGA_B} = 24'hd0d0d0;
				8'h1f : {VGA_R, VGA_G, VGA_B} = 24'he5e4e0;
				8'h20 : {VGA_R, VGA_G, VGA_B} = 24'he4e5e1;
				8'h21 : {VGA_R, VGA_G, VGA_B} = 24'hf3f3f3;
				8'h22 : {VGA_R, VGA_G, VGA_B} = 24'hfefefe;
				8'h23 : {VGA_R, VGA_G, VGA_B} = 24'hffffff;
				8'h24 : {VGA_R, VGA_G, VGA_B} = 24'hf4f3f3;
				8'h25 : {VGA_R, VGA_G, VGA_B} = 24'hf4f4f3;
				8'h26 : {VGA_R, VGA_G, VGA_B} = 24'hf2f3f1;
				8'h27 : {VGA_R, VGA_G, VGA_B} = 24'hf0f0ee;
				8'h28 : {VGA_R, VGA_G, VGA_B} = 24'heeedec;
				8'h29 : {VGA_R, VGA_G, VGA_B} = 24'heaeae8;
				8'h2a : {VGA_R, VGA_G, VGA_B} = 24'he3e3df;
				8'h2b : {VGA_R, VGA_G, VGA_B} = 24'he3e2de;
				8'h2c : {VGA_R, VGA_G, VGA_B} = 24'he2e2de;
				8'h2d : {VGA_R, VGA_G, VGA_B} = 24'he4e4e0;
				8'h2e : {VGA_R, VGA_G, VGA_B} = 24'he9e9e7;
				8'h2f : {VGA_R, VGA_G, VGA_B} = 24'hfffefe;
				8'h30 : {VGA_R, VGA_G, VGA_B} = 24'hf6f5f5;
				8'h31 : {VGA_R, VGA_G, VGA_B} = 24'hefefed;
				8'h32 : {VGA_R, VGA_G, VGA_B} = 24'hebebea;
				8'h33 : {VGA_R, VGA_G, VGA_B} = 24'hebeae7;
				8'h34 : {VGA_R, VGA_G, VGA_B} = 24'he8e7e5;
				8'h35 : {VGA_R, VGA_G, VGA_B} = 24'he2e3de;
				8'h36 : {VGA_R, VGA_G, VGA_B} = 24'he1e1dd;
				8'h37 : {VGA_R, VGA_G, VGA_B} = 24'hb4b4b4;
				8'h38 : {VGA_R, VGA_G, VGA_B} = 24'hdfdfdc;
				8'h39 : {VGA_R, VGA_G, VGA_B} = 24'hf0f1ee;
				8'h3a : {VGA_R, VGA_G, VGA_B} = 24'hf8f7f7;
				8'h3b : {VGA_R, VGA_G, VGA_B} = 24'hf0f1ef;
				8'h3c : {VGA_R, VGA_G, VGA_B} = 24'hebeae6;
				8'h3d : {VGA_R, VGA_G, VGA_B} = 24'he6e6e1;
				8'h3e : {VGA_R, VGA_G, VGA_B} = 24'hdfdfdb;
				8'h3f : {VGA_R, VGA_G, VGA_B} = 24'he0e1dd;
				8'h40 : {VGA_R, VGA_G, VGA_B} = 24'he0e0dc;
				8'h41 : {VGA_R, VGA_G, VGA_B} = 24'hdedfdb;
				8'h42 : {VGA_R, VGA_G, VGA_B} = 24'hfbfbfa;
				8'h43 : {VGA_R, VGA_G, VGA_B} = 24'hfcfcfb;
				8'h44 : {VGA_R, VGA_G, VGA_B} = 24'hfdfdfd;
				8'h45 : {VGA_R, VGA_G, VGA_B} = 24'hfdfcfc;
				8'h46 : {VGA_R, VGA_G, VGA_B} = 24'hf9f9f8;
				8'h47 : {VGA_R, VGA_G, VGA_B} = 24'hf2f2f0;
				8'h48 : {VGA_R, VGA_G, VGA_B} = 24'he8e8e6;
				8'h49 : {VGA_R, VGA_G, VGA_B} = 24'hdfe0dc;
				8'h4a : {VGA_R, VGA_G, VGA_B} = 24'hdddeda;
				8'h4b : {VGA_R, VGA_G, VGA_B} = 24'hdededa;
				8'h4c : {VGA_R, VGA_G, VGA_B} = 24'hf4f5f4;
				8'h4d : {VGA_R, VGA_G, VGA_B} = 24'hfcfcfc;
				8'h4e : {VGA_R, VGA_G, VGA_B} = 24'hf9faf9;
				8'h4f : {VGA_R, VGA_G, VGA_B} = 24'hf1f1ef;
				8'h50 : {VGA_R, VGA_G, VGA_B} = 24'heaebe9;
				8'h51 : {VGA_R, VGA_G, VGA_B} = 24'hddddda;
				8'h52 : {VGA_R, VGA_G, VGA_B} = 24'he0e1dc;
				8'h53 : {VGA_R, VGA_G, VGA_B} = 24'hdadbd7;
				8'h54 : {VGA_R, VGA_G, VGA_B} = 24'hdeddda;
				8'h55 : {VGA_R, VGA_G, VGA_B} = 24'hf5f5f4;
				8'h56 : {VGA_R, VGA_G, VGA_B} = 24'hfdfdfc;
				8'h57 : {VGA_R, VGA_G, VGA_B} = 24'hefefec;
				8'h58 : {VGA_R, VGA_G, VGA_B} = 24'he7e8e4;
				8'h59 : {VGA_R, VGA_G, VGA_B} = 24'hdcdcd8;
				8'h5a : {VGA_R, VGA_G, VGA_B} = 24'hdcddda;
				8'h5b : {VGA_R, VGA_G, VGA_B} = 24'hc4c5c3;
				8'h5c : {VGA_R, VGA_G, VGA_B} = 24'hdadbd9;
				8'h5d : {VGA_R, VGA_G, VGA_B} = 24'hd9dad6;
				8'h5e : {VGA_R, VGA_G, VGA_B} = 24'he1e2dd;
				8'h5f : {VGA_R, VGA_G, VGA_B} = 24'hf6f7f5;
				8'h60 : {VGA_R, VGA_G, VGA_B} = 24'hf6f7f6;
				8'h61 : {VGA_R, VGA_G, VGA_B} = 24'hfcfbfb;
				8'h62 : {VGA_R, VGA_G, VGA_B} = 24'hfefffe;
				8'h63 : {VGA_R, VGA_G, VGA_B} = 24'hf8f7f6;
				8'h64 : {VGA_R, VGA_G, VGA_B} = 24'heaebe8;
				8'h65 : {VGA_R, VGA_G, VGA_B} = 24'hdcddd9;
				8'h66 : {VGA_R, VGA_G, VGA_B} = 24'hdadad7;
				8'h67 : {VGA_R, VGA_G, VGA_B} = 24'hdadbd6;
				8'h68 : {VGA_R, VGA_G, VGA_B} = 24'he9eae7;
				8'h69 : {VGA_R, VGA_G, VGA_B} = 24'hf3f3f1;
				8'h6a : {VGA_R, VGA_G, VGA_B} = 24'hf8f9f7;
				8'h6b : {VGA_R, VGA_G, VGA_B} = 24'hfefdfd;
				8'h6c : {VGA_R, VGA_G, VGA_B} = 24'hf3f4f3;
				8'h6d : {VGA_R, VGA_G, VGA_B} = 24'hecebeb;
				8'h6e : {VGA_R, VGA_G, VGA_B} = 24'he5e5e1;
				8'h6f : {VGA_R, VGA_G, VGA_B} = 24'hd8dad5;
				8'h70 : {VGA_R, VGA_G, VGA_B} = 24'hd9dad7;
				8'h71 : {VGA_R, VGA_G, VGA_B} = 24'hebebe9;
				8'h72 : {VGA_R, VGA_G, VGA_B} = 24'hfbfafa;
				8'h73 : {VGA_R, VGA_G, VGA_B} = 24'heeedeb;
				8'h74 : {VGA_R, VGA_G, VGA_B} = 24'hdad9d5;
				8'h75 : {VGA_R, VGA_G, VGA_B} = 24'hdbdbd7;
				8'h76 : {VGA_R, VGA_G, VGA_B} = 24'hecebe9;
				8'h77 : {VGA_R, VGA_G, VGA_B} = 24'hf5f4f3;
				8'h78 : {VGA_R, VGA_G, VGA_B} = 24'hfaf9f9;
				8'h79 : {VGA_R, VGA_G, VGA_B} = 24'heeeeeb;
				8'h7a : {VGA_R, VGA_G, VGA_B} = 24'hd9dbd8;
				8'h7b : {VGA_R, VGA_G, VGA_B} = 24'hf5f4f4;
				8'h7c : {VGA_R, VGA_G, VGA_B} = 24'hededeb;
				8'h7d : {VGA_R, VGA_G, VGA_B} = 24'hd9dad5;
				8'h7e : {VGA_R, VGA_G, VGA_B} = 24'hdbdbd9;
				8'h7f : {VGA_R, VGA_G, VGA_B} = 24'he1e1dc;
				8'h80 : {VGA_R, VGA_G, VGA_B} = 24'he9e9e6;
				8'h81 : {VGA_R, VGA_G, VGA_B} = 24'hf8f8f7;
				8'h82 : {VGA_R, VGA_G, VGA_B} = 24'hfcfdfc;
				8'h83 : {VGA_R, VGA_G, VGA_B} = 24'hfafaf8;
				8'h84 : {VGA_R, VGA_G, VGA_B} = 24'hf4f3f2;
				8'h85 : {VGA_R, VGA_G, VGA_B} = 24'he4e3e0;
				8'h86 : {VGA_R, VGA_G, VGA_B} = 24'he7e7e3;
				8'h87 : {VGA_R, VGA_G, VGA_B} = 24'hfdfefd;
				8'h88 : {VGA_R, VGA_G, VGA_B} = 24'he1e2df;
				8'h89 : {VGA_R, VGA_G, VGA_B} = 24'hdddedc;
				8'h8a : {VGA_R, VGA_G, VGA_B} = 24'hdededb;
				8'h8b : {VGA_R, VGA_G, VGA_B} = 24'he6e7e3;
				8'h8c : {VGA_R, VGA_G, VGA_B} = 24'hf4f4f2;
				8'h8d : {VGA_R, VGA_G, VGA_B} = 24'hfbfbfb;
				8'h8e : {VGA_R, VGA_G, VGA_B} = 24'he8e7e3;
				8'h8f : {VGA_R, VGA_G, VGA_B} = 24'he3e2df;
				8'h90 : {VGA_R, VGA_G, VGA_B} = 24'hdedfdc;
				8'h91 : {VGA_R, VGA_G, VGA_B} = 24'he8e8e5;
				8'h92 : {VGA_R, VGA_G, VGA_B} = 24'hedeeec;
				8'h93 : {VGA_R, VGA_G, VGA_B} = 24'hf3f3f2;
				8'h94 : {VGA_R, VGA_G, VGA_B} = 24'hf4f4f4;
				8'h95 : {VGA_R, VGA_G, VGA_B} = 24'he4e5e0;
				8'h96 : {VGA_R, VGA_G, VGA_B} = 24'hdadbda;
				8'h97 : {VGA_R, VGA_G, VGA_B} = 24'he3e4df;
				8'h98 : {VGA_R, VGA_G, VGA_B} = 24'he9e9e5;
				8'h99 : {VGA_R, VGA_G, VGA_B} = 24'hf6f6f6;
				8'h9a : {VGA_R, VGA_G, VGA_B} = 24'heff0ed;
				8'h9b : {VGA_R, VGA_G, VGA_B} = 24'hdadada;
				8'h9c : {VGA_R, VGA_G, VGA_B} = 24'he6e6e3;
				8'h9d : {VGA_R, VGA_G, VGA_B} = 24'he7e7e4;
				8'h9e : {VGA_R, VGA_G, VGA_B} = 24'hf2f3f2;
				8'h9f : {VGA_R, VGA_G, VGA_B} = 24'hf1f2f0;
				8'ha0 : {VGA_R, VGA_G, VGA_B} = 24'heeefec;
				8'ha1 : {VGA_R, VGA_G, VGA_B} = 24'hc4c4c4;
				8'ha2 : {VGA_R, VGA_G, VGA_B} = 24'hd6d6d6;
				8'ha3 : {VGA_R, VGA_G, VGA_B} = 24'he7e8e5;
				8'ha4 : {VGA_R, VGA_G, VGA_B} = 24'he6e7e2;
				8'ha5 : {VGA_R, VGA_G, VGA_B} = 24'hf0efed;
				8'ha6 : {VGA_R, VGA_G, VGA_B} = 24'hf0f0ef;
				8'ha7 : {VGA_R, VGA_G, VGA_B} = 24'hf2f1f0;
				8'ha8 : {VGA_R, VGA_G, VGA_B} = 24'hecece9;
				8'ha9 : {VGA_R, VGA_G, VGA_B} = 24'he5e6e1;
				8'haa : {VGA_R, VGA_G, VGA_B} = 24'he8e7e4;
				8'hab : {VGA_R, VGA_G, VGA_B} = 24'heaeae7;
				8'hac : {VGA_R, VGA_G, VGA_B} = 24'heff0ee;
				8'had : {VGA_R, VGA_G, VGA_B} = 24'hedeceb;
				8'hae : {VGA_R, VGA_G, VGA_B} = 24'hc1c1c1;
				8'haf : {VGA_R, VGA_G, VGA_B} = 24'he1e1e1;
				8'hb0 : {VGA_R, VGA_G, VGA_B} = 24'heaeae6;
				8'hb1 : {VGA_R, VGA_G, VGA_B} = 24'he9ebe8;
				8'hb2 : {VGA_R, VGA_G, VGA_B} = 24'hd9dad9;
				default:{VGA_R, VGA_G, VGA_B} = 24'h000000;
			endcase
		end
		// Draw White Piece
		else if(piece_info[2]==1'b1 && black_rgb!=8'h00 && is_piece_area) begin
			case(black_rgb)
				8'h00 : {VGA_R, VGA_G, VGA_B} = 24'h000000;
				8'h01 : {VGA_R, VGA_G, VGA_B} = 24'h646464;
				8'h02 : {VGA_R, VGA_G, VGA_B} = 24'h606161;
				8'h03 : {VGA_R, VGA_G, VGA_B} = 24'h414243;
				8'h04 : {VGA_R, VGA_G, VGA_B} = 24'h323334;
				8'h05 : {VGA_R, VGA_G, VGA_B} = 24'h333436;
				8'h06 : {VGA_R, VGA_G, VGA_B} = 24'h4e4f50;
				8'h07 : {VGA_R, VGA_G, VGA_B} = 24'h5e5f5f;
				8'h08 : {VGA_R, VGA_G, VGA_B} = 24'h444445;
				8'h09 : {VGA_R, VGA_G, VGA_B} = 24'h343637;
				8'h0a : {VGA_R, VGA_G, VGA_B} = 24'h262829;
				8'h0b : {VGA_R, VGA_G, VGA_B} = 24'h2d2e2f;
				8'h0c : {VGA_R, VGA_G, VGA_B} = 24'h2c2e2f;
				8'h0d : {VGA_R, VGA_G, VGA_B} = 24'h2a2b2d;
				8'h0e : {VGA_R, VGA_G, VGA_B} = 24'h282a2b;
				8'h0f : {VGA_R, VGA_G, VGA_B} = 24'h27282a;
				8'h10 : {VGA_R, VGA_G, VGA_B} = 24'h272728;
				8'h11 : {VGA_R, VGA_G, VGA_B} = 24'h2b2c2d;
				8'h12 : {VGA_R, VGA_G, VGA_B} = 24'h292b2d;
				8'h13 : {VGA_R, VGA_G, VGA_B} = 24'h454547;
				8'h14 : {VGA_R, VGA_G, VGA_B} = 24'h39393b;
				8'h15 : {VGA_R, VGA_G, VGA_B} = 24'h262728;
				8'h16 : {VGA_R, VGA_G, VGA_B} = 24'h2f3131;
				8'h17 : {VGA_R, VGA_G, VGA_B} = 24'h3d3f40;
				8'h18 : {VGA_R, VGA_G, VGA_B} = 24'h4a4b4c;
				8'h19 : {VGA_R, VGA_G, VGA_B} = 24'h4f5152;
				8'h1a : {VGA_R, VGA_G, VGA_B} = 24'h515254;
				8'h1b : {VGA_R, VGA_G, VGA_B} = 24'h4f5153;
				8'h1c : {VGA_R, VGA_G, VGA_B} = 24'h484a4c;
				8'h1d : {VGA_R, VGA_G, VGA_B} = 24'h3a3c3d;
				8'h1e : {VGA_R, VGA_G, VGA_B} = 24'h2e3031;
				8'h1f : {VGA_R, VGA_G, VGA_B} = 24'h262727;
				8'h20 : {VGA_R, VGA_G, VGA_B} = 24'h2b2c2c;
				8'h21 : {VGA_R, VGA_G, VGA_B} = 24'h2b2d2d;
				8'h22 : {VGA_R, VGA_G, VGA_B} = 24'h37393b;
				8'h23 : {VGA_R, VGA_G, VGA_B} = 24'h292b2c;
				8'h24 : {VGA_R, VGA_G, VGA_B} = 24'h292b2b;
				8'h25 : {VGA_R, VGA_G, VGA_B} = 24'h737375;
				8'h26 : {VGA_R, VGA_G, VGA_B} = 24'h818284;
				8'h27 : {VGA_R, VGA_G, VGA_B} = 24'h707274;
				8'h28 : {VGA_R, VGA_G, VGA_B} = 24'h646567;
				8'h29 : {VGA_R, VGA_G, VGA_B} = 24'h696a6d;
				8'h2a : {VGA_R, VGA_G, VGA_B} = 24'h6c6e70;
				8'h2b : {VGA_R, VGA_G, VGA_B} = 24'h676a6b;
				8'h2c : {VGA_R, VGA_G, VGA_B} = 24'h595c5c;
				8'h2d : {VGA_R, VGA_G, VGA_B} = 24'h414444;
				8'h2e : {VGA_R, VGA_G, VGA_B} = 24'h303031;
				8'h2f : {VGA_R, VGA_G, VGA_B} = 24'h252626;
				8'h30 : {VGA_R, VGA_G, VGA_B} = 24'h222324;
				8'h31 : {VGA_R, VGA_G, VGA_B} = 24'h1d1d1f;
				8'h32 : {VGA_R, VGA_G, VGA_B} = 24'h373939;
				8'h33 : {VGA_R, VGA_G, VGA_B} = 24'h292a2b;
				8'h34 : {VGA_R, VGA_G, VGA_B} = 24'h222323;
				8'h35 : {VGA_R, VGA_G, VGA_B} = 24'h5c5e5f;
				8'h36 : {VGA_R, VGA_G, VGA_B} = 24'ha6a7a7;
				8'h37 : {VGA_R, VGA_G, VGA_B} = 24'hc5c5c5;
				8'h38 : {VGA_R, VGA_G, VGA_B} = 24'h9fa0a1;
				8'h39 : {VGA_R, VGA_G, VGA_B} = 24'h5a5d5f;
				8'h3a : {VGA_R, VGA_G, VGA_B} = 24'h5d5f62;
				8'h3b : {VGA_R, VGA_G, VGA_B} = 24'h59595d;
				8'h3c : {VGA_R, VGA_G, VGA_B} = 24'h424547;
				8'h3d : {VGA_R, VGA_G, VGA_B} = 24'h424446;
				8'h3e : {VGA_R, VGA_G, VGA_B} = 24'h434546;
				8'h3f : {VGA_R, VGA_G, VGA_B} = 24'h3b3d3d;
				8'h40 : {VGA_R, VGA_G, VGA_B} = 24'h303132;
				8'h41 : {VGA_R, VGA_G, VGA_B} = 24'h2a2b2b;
				8'h42 : {VGA_R, VGA_G, VGA_B} = 24'h232425;
				8'h43 : {VGA_R, VGA_G, VGA_B} = 24'h202021;
				8'h44 : {VGA_R, VGA_G, VGA_B} = 24'h1b1d1f;
				8'h45 : {VGA_R, VGA_G, VGA_B} = 24'h414343;
				8'h46 : {VGA_R, VGA_G, VGA_B} = 24'h2a2b2c;
				8'h47 : {VGA_R, VGA_G, VGA_B} = 24'h1a1b1b;
				8'h48 : {VGA_R, VGA_G, VGA_B} = 24'h333434;
				8'h49 : {VGA_R, VGA_G, VGA_B} = 24'h838586;
				8'h4a : {VGA_R, VGA_G, VGA_B} = 24'hc9c8c8;
				8'h4b : {VGA_R, VGA_G, VGA_B} = 24'hb5b5b6;
				8'h4c : {VGA_R, VGA_G, VGA_B} = 24'h616465;
				8'h4d : {VGA_R, VGA_G, VGA_B} = 24'h333536;
				8'h4e : {VGA_R, VGA_G, VGA_B} = 24'h5e5f62;
				8'h4f : {VGA_R, VGA_G, VGA_B} = 24'h4f4f51;
				8'h50 : {VGA_R, VGA_G, VGA_B} = 24'h28292b;
				8'h51 : {VGA_R, VGA_G, VGA_B} = 24'h29292a;
				8'h52 : {VGA_R, VGA_G, VGA_B} = 24'h272a29;
				8'h53 : {VGA_R, VGA_G, VGA_B} = 24'h282a2a;
				8'h54 : {VGA_R, VGA_G, VGA_B} = 24'h222224;
				8'h55 : {VGA_R, VGA_G, VGA_B} = 24'h201f20;
				8'h56 : {VGA_R, VGA_G, VGA_B} = 24'h1d1f1f;
				8'h57 : {VGA_R, VGA_G, VGA_B} = 24'h212525;
				8'h58 : {VGA_R, VGA_G, VGA_B} = 24'h3b3c3d;
				8'h59 : {VGA_R, VGA_G, VGA_B} = 24'h1a1b1c;
				8'h5a : {VGA_R, VGA_G, VGA_B} = 24'h212223;
				8'h5b : {VGA_R, VGA_G, VGA_B} = 24'h2e2f2f;
				8'h5c : {VGA_R, VGA_G, VGA_B} = 24'h545656;
				8'h5d : {VGA_R, VGA_G, VGA_B} = 24'h777778;
				8'h5e : {VGA_R, VGA_G, VGA_B} = 24'h565759;
				8'h5f : {VGA_R, VGA_G, VGA_B} = 24'h27292a;
				8'h60 : {VGA_R, VGA_G, VGA_B} = 24'h2b2c2e;
				8'h61 : {VGA_R, VGA_G, VGA_B} = 24'h272829;
				8'h62 : {VGA_R, VGA_G, VGA_B} = 24'h202222;
				8'h63 : {VGA_R, VGA_G, VGA_B} = 24'h212222;
				8'h64 : {VGA_R, VGA_G, VGA_B} = 24'h1d1d1e;
				8'h65 : {VGA_R, VGA_G, VGA_B} = 24'h1d1e1d;
				8'h66 : {VGA_R, VGA_G, VGA_B} = 24'h212224;
				8'h67 : {VGA_R, VGA_G, VGA_B} = 24'h444545;
				8'h68 : {VGA_R, VGA_G, VGA_B} = 24'h1d1d1d;
				8'h69 : {VGA_R, VGA_G, VGA_B} = 24'h1c1d1e;
				8'h6a : {VGA_R, VGA_G, VGA_B} = 24'h191a1b;
				8'h6b : {VGA_R, VGA_G, VGA_B} = 24'h1b1c1d;
				8'h6c : {VGA_R, VGA_G, VGA_B} = 24'h1b1c1c;
				8'h6d : {VGA_R, VGA_G, VGA_B} = 24'h1c1e1e;
				8'h6e : {VGA_R, VGA_G, VGA_B} = 24'h1d1e1f;
				8'h6f : {VGA_R, VGA_G, VGA_B} = 24'h1d1e1e;
				8'h70 : {VGA_R, VGA_G, VGA_B} = 24'h191b1b;
				8'h71 : {VGA_R, VGA_G, VGA_B} = 24'h181a19;
				8'h72 : {VGA_R, VGA_G, VGA_B} = 24'h1c1c1c;
				8'h73 : {VGA_R, VGA_G, VGA_B} = 24'h181819;
				8'h74 : {VGA_R, VGA_G, VGA_B} = 24'h232323;
				8'h75 : {VGA_R, VGA_G, VGA_B} = 24'h353738;
				8'h76 : {VGA_R, VGA_G, VGA_B} = 24'h2c2c2d;
				8'h77 : {VGA_R, VGA_G, VGA_B} = 24'h3d3e3f;
				8'h78 : {VGA_R, VGA_G, VGA_B} = 24'h202122;
				8'h79 : {VGA_R, VGA_G, VGA_B} = 24'h131415;
				8'h7a : {VGA_R, VGA_G, VGA_B} = 24'h141516;
				8'h7b : {VGA_R, VGA_G, VGA_B} = 24'h151617;
				8'h7c : {VGA_R, VGA_G, VGA_B} = 24'h18191a;
				8'h7d : {VGA_R, VGA_G, VGA_B} = 24'h161718;
				8'h7e : {VGA_R, VGA_G, VGA_B} = 24'h161717;
				8'h7f : {VGA_R, VGA_G, VGA_B} = 24'h181918;
				8'h80 : {VGA_R, VGA_G, VGA_B} = 24'h171818;
				8'h81 : {VGA_R, VGA_G, VGA_B} = 24'h1c1d1d;
				8'h82 : {VGA_R, VGA_G, VGA_B} = 24'h676767;
				8'h83 : {VGA_R, VGA_G, VGA_B} = 24'h252627;
				8'h84 : {VGA_R, VGA_G, VGA_B} = 24'h141416;
				8'h85 : {VGA_R, VGA_G, VGA_B} = 24'h292929;
				8'h86 : {VGA_R, VGA_G, VGA_B} = 24'h323333;
				8'h87 : {VGA_R, VGA_G, VGA_B} = 24'h1b1b1d;
				8'h88 : {VGA_R, VGA_G, VGA_B} = 24'h151616;
				8'h89 : {VGA_R, VGA_G, VGA_B} = 24'h151516;
				8'h8a : {VGA_R, VGA_G, VGA_B} = 24'h141415;
				8'h8b : {VGA_R, VGA_G, VGA_B} = 24'h636363;
				8'h8c : {VGA_R, VGA_G, VGA_B} = 24'h424444;
				8'h8d : {VGA_R, VGA_G, VGA_B} = 24'h212122;
				8'h8e : {VGA_R, VGA_G, VGA_B} = 24'h161618;
				8'h8f : {VGA_R, VGA_G, VGA_B} = 24'h1b1b1c;
				8'h90 : {VGA_R, VGA_G, VGA_B} = 24'h171717;
				8'h91 : {VGA_R, VGA_G, VGA_B} = 24'h161617;
				8'h92 : {VGA_R, VGA_G, VGA_B} = 24'h474747;
				8'h93 : {VGA_R, VGA_G, VGA_B} = 24'h2e3030;
				8'h94 : {VGA_R, VGA_G, VGA_B} = 24'h1f1f20;
				8'h95 : {VGA_R, VGA_G, VGA_B} = 24'h141515;
				8'h96 : {VGA_R, VGA_G, VGA_B} = 24'h131414;
				8'h97 : {VGA_R, VGA_G, VGA_B} = 24'h313333;
				8'h98 : {VGA_R, VGA_G, VGA_B} = 24'h131514;
				8'h99 : {VGA_R, VGA_G, VGA_B} = 24'h313232;
				8'h9a : {VGA_R, VGA_G, VGA_B} = 24'h141514;
				8'h9b : {VGA_R, VGA_G, VGA_B} = 24'h2f3030;
				8'h9c : {VGA_R, VGA_G, VGA_B} = 24'h424343;
				8'h9d : {VGA_R, VGA_G, VGA_B} = 24'h141414;
				8'h9e : {VGA_R, VGA_G, VGA_B} = 24'h434343;
				8'h9f : {VGA_R, VGA_G, VGA_B} = 24'h646666;
				8'ha0 : {VGA_R, VGA_G, VGA_B} = 24'h131314;
				8'ha1 : {VGA_R, VGA_G, VGA_B} = 24'h38393a;
				8'ha2 : {VGA_R, VGA_G, VGA_B} = 24'h171819;
				8'ha3 : {VGA_R, VGA_G, VGA_B} = 24'h262628;
				8'ha4 : {VGA_R, VGA_G, VGA_B} = 24'h393839;
				8'ha5 : {VGA_R, VGA_G, VGA_B} = 24'h141517;
				8'ha6 : {VGA_R, VGA_G, VGA_B} = 24'h181919;
				8'ha7 : {VGA_R, VGA_G, VGA_B} = 24'h191a19;
				8'ha8 : {VGA_R, VGA_G, VGA_B} = 24'h191a1a;
				8'ha9 : {VGA_R, VGA_G, VGA_B} = 24'h464545;
				8'haa : {VGA_R, VGA_G, VGA_B} = 24'h1b1d1c;
				8'hab : {VGA_R, VGA_G, VGA_B} = 24'h2b2b2b;
				8'hac : {VGA_R, VGA_G, VGA_B} = 24'h313132;
				8'had : {VGA_R, VGA_G, VGA_B} = 24'h171919;
				8'hae : {VGA_R, VGA_G, VGA_B} = 24'h1c1d1f;
				8'haf : {VGA_R, VGA_G, VGA_B} = 24'h1d1f20;
				8'hb0 : {VGA_R, VGA_G, VGA_B} = 24'h1f2021;
				8'hb1 : {VGA_R, VGA_G, VGA_B} = 24'h222223;
				8'hb2 : {VGA_R, VGA_G, VGA_B} = 24'h1e1f20;
				8'hb3 : {VGA_R, VGA_G, VGA_B} = 24'h1e1f1f;
				8'hb4 : {VGA_R, VGA_G, VGA_B} = 24'h252526;
				8'hb5 : {VGA_R, VGA_G, VGA_B} = 24'h2b2d2e;
				8'hb6 : {VGA_R, VGA_G, VGA_B} = 24'h424242;
				8'hb7 : {VGA_R, VGA_G, VGA_B} = 24'h333334;
				8'hb8 : {VGA_R, VGA_G, VGA_B} = 24'h1a1a1c;
				8'hb9 : {VGA_R, VGA_G, VGA_B} = 24'h232426;
				8'hba : {VGA_R, VGA_G, VGA_B} = 24'h292c2b;
				8'hbb : {VGA_R, VGA_G, VGA_B} = 24'h2c2d2f;
				8'hbc : {VGA_R, VGA_G, VGA_B} = 24'h272929;
				8'hbd : {VGA_R, VGA_G, VGA_B} = 24'h262729;
				8'hbe : {VGA_R, VGA_G, VGA_B} = 24'h232225;
				8'hbf : {VGA_R, VGA_G, VGA_B} = 24'h2f2f2f;
				8'hc0 : {VGA_R, VGA_G, VGA_B} = 24'h404040;
				8'hc1 : {VGA_R, VGA_G, VGA_B} = 24'h3a3a3b;
				8'hc2 : {VGA_R, VGA_G, VGA_B} = 24'h292a2a;
				8'hc3 : {VGA_R, VGA_G, VGA_B} = 24'h212324;
				8'hc4 : {VGA_R, VGA_G, VGA_B} = 24'h252527;
				8'hc5 : {VGA_R, VGA_G, VGA_B} = 24'h2d2e30;
				8'hc6 : {VGA_R, VGA_G, VGA_B} = 24'h2d3031;
				8'hc7 : {VGA_R, VGA_G, VGA_B} = 24'h292a2c;
				8'hc8 : {VGA_R, VGA_G, VGA_B} = 24'h29292b;
				8'hc9 : {VGA_R, VGA_G, VGA_B} = 24'h343535;
				8'hca : {VGA_R, VGA_G, VGA_B} = 24'h373737;
				8'hcb : {VGA_R, VGA_G, VGA_B} = 24'h393b3b;
				8'hcc : {VGA_R, VGA_G, VGA_B} = 24'h383939;
				8'hcd : {VGA_R, VGA_G, VGA_B} = 24'h303131;
				8'hce : {VGA_R, VGA_G, VGA_B} = 24'h2d2d2e;
				8'hcf : {VGA_R, VGA_G, VGA_B} = 24'h242627;
				8'hd0 : {VGA_R, VGA_G, VGA_B} = 24'h2c2b2d;
				8'hd1 : {VGA_R, VGA_G, VGA_B} = 24'h38383a;
				8'hd2 : {VGA_R, VGA_G, VGA_B} = 24'h383a3a;
				8'hd3 : {VGA_R, VGA_G, VGA_B} = 24'h343434;
				8'hd4 : {VGA_R, VGA_G, VGA_B} = 24'h363737;
				8'hd5 : {VGA_R, VGA_G, VGA_B} = 24'h3f4040;
				8'hd6 : {VGA_R, VGA_G, VGA_B} = 24'h414141;
				default:{VGA_R, VGA_G, VGA_B} = 24'h000000;
			endcase
		end
		// Draw board
		else if(is_board_area)
			case(bg_palette)
				4'h0 : {VGA_R, VGA_G, VGA_B} = 24'he7c5a3;
				4'h1 : {VGA_R, VGA_G, VGA_B} = 24'he4c19d;
				4'h2 : {VGA_R, VGA_G, VGA_B} = 24'he2bd98;
				4'h3 : {VGA_R, VGA_G, VGA_B} = 24'hdbb791;
				4'h4 : {VGA_R, VGA_G, VGA_B} = 24'hb5885a;
				4'h5 : {VGA_R, VGA_G, VGA_B} = 24'hbb9165;
				4'h6 : {VGA_R, VGA_G, VGA_B} = 24'hd2ac84;
				4'h7 : {VGA_R, VGA_G, VGA_B} = 24'hc69d72;
				4'h8 : {VGA_R, VGA_G, VGA_B} = 24'hcba47a;
				4'h9 : {VGA_R, VGA_G, VGA_B} = 24'hb98e60;
				4'ha : {VGA_R, VGA_G, VGA_B} = 24'heed0ad;
				4'hb : {VGA_R, VGA_G, VGA_B} = 24'he2bea0;
				4'hc : {VGA_R, VGA_G, VGA_B} = 24'h000000;
				4'hd : {VGA_R, VGA_G, VGA_B} = 24'hdec09c;
				4'he : {VGA_R, VGA_G, VGA_B} = 24'hdebea0;
			endcase
		else
			{VGA_R, VGA_G, VGA_B} = 24'h000000;
   end
endmodule

module vga_counters(
 input logic 	     clk50, reset,
 output logic [10:0] hcount,  // hcount[10:1] is pixel column
 output logic [9:0]  vcount,  // vcount[9:0] is pixel row
 output logic 	     VGA_CLK, VGA_HS, VGA_VS, VGA_BLANK_n, VGA_SYNC_n);

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
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

   logic endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else  	         hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;
       
   logic endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

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

module vga_piece_painter(
	input logic			clk, reset,
	input logic[4:0]	piece_v,
	input logic[4:0]	piece_h,
	input logic[2:0]	piece_info,
	output logic[7:0]	piece_rgb,
	output logic		draw_piece);

	logic left_line, right_line, up_line, bottom_line, center, piece, selected;
	logic [9:0] dis2;
	logic [4:0] dis_v,dis_h;
	assign dis_v = (piece_v > 5'd14) ? (piece_v - 5'd14): (5'd14 - piece_v);
    assign dis_h = (piece_h > 5'd14) ? (piece_h - 5'd14): (5'd14 - piece_h);
    assign dis2 = $unsigned(dis_v)*$unsigned(dis_v) + 
                 $unsigned(dis_h)*$unsigned(dis_h);
	assign piece_addr = piece_v*5'd29+piece_h;

	always_ff @(posedge clk) begin
		// Draw Selected border
		if(selected && piece_info[2]) begin
			draw_piece <= 1'b1;
			piece_rgb <= 24'h000000;
		end
		// Draw Piece
		else if(piece && piece_info[7:6]>2'b0) begin
			if(piece_info[7]) piece_rgb <= 24'hffffff;
			else piece_rgb <= 24'h000000;
		end
		// Draw center dot
		else if(piece_info[4]&&center)
			piece_rgb <= 24'h000000;
		// left line
		else if(piece_info[0]&&left_line)
			piece_rgb <= 24'h000000;
		// right line
		else if(piece_info[1]&&right_line)
			piece_rgb <= 24'h000000;
		// up line
		else if(piece_info[2]&&up_line)
			piece_rgb <= 24'h000000;
		// bottom line
		else if(piece_info[3]&&bottom_line)
			piece_rgb <= 24'h000000;
		// background
		else
			piece_rgb <= 24'hF0C090;
	end

	assign left_line = (piece_h <= 5'd14) && (piece_v == 5'd14);
	assign right_line = (piece_h >= 5'd14) && (piece_v == 5'd14);
	assign up_line = (piece_v <= 5'd14) && (piece_h == 5'd14);
	assign bottom_line = (piece_v >= 5'd14) && (piece_h == 5'd14);
	assign center = (dis2 <= 10'd9);
	assign selected  = (piece_h <= 5'd1) || (piece_h >= 5'd27) || (piece_v <= 5'd1) || (piece_v >= 5'd27);
	/*assign selected  = ((piece_h <= 5'd1) || (piece_h >= 5'd27) && ((piece_v <= 5'd7) || (piece_v >= 5'd21))) ||
	                   ((piece_v <= 5'd1) || (piece_v >= 5'd27) && ((piece_h <= 5'd7) || (piece_h >= 5'd21)));*/
	/*
	assign left_line = (piece_h <= 5'd14) && (piece_v >= 5'd13) && (piece_v <= 5'd15);
	assign right_line = (piece_h >= 5'd14) && (piece_v >= 5'd13) && (piece_v <= 5'd15);
	assign up_line = (piece_v <= 5'd14) && (piece_h >= 5'd13) && (piece_h <= 5'd15);
	assign bottom_line = (piece_v >= 5'd14) && (piece_h >= 5'd13) && (piece_h <= 5'd15);
	assign center = (piece_h >= 5'd11) && (piece_h <= 5'd17) && (piece_v >= 5'd11) && (piece_v >= 5'd17);
	assign selected  = ((piece_h <= 5'd1) || (piece_h >= 5'd27) && ((piece_v <= 5'd7) || (piece_v >= 5'd21))) ||
	                   ((piece_v <= 5'd1) || (piece_v >= 5'd27) && ((piece_h <= 5'd7) || (piece_h >= 5'd21)));*/
	// Piece radius = 12
	assign piece = (dis2 <= 10'd144);
endmodule

module white_palette(
	input logic [7:0] rgb,
	output logic[23:0] color
);
endmodule
