# File: sv_gen.py 
# Author: Yunzhou Li
# Time: Spring 2024
# Description: Generate SystemVerilog code block of message display
import math

# Parameters
#	- txt: message text
#	- h_start: message start hcount [0,640)
# 			   if h_start=-1, the message will be placed at the center.
#	- v_start: message start vcount [0,480)
#			   if v_start=-1, the message will be placed at the center.
#	- width_size_order: describes font width, the final font width is
# 						2^width_size_order*8
#	- height_size_order: describes font height, the final font height is
# 						2^height_size_order*5

# Add your message here
message_list = [{
	"txt":"GOMOKU",
	"h_start":128,
	"v_start":128,
	"width_size_order":3,
	"height_size_order":4
},{
	"txt":"START PVE",
	"h_start":-1,
	"v_start":250,
	"width_size_order":1,
	"height_size_order":2
},{
	"txt":"START PVP",
	"h_start":-1,
	"v_start":280,
	"width_size_order":1,
	"height_size_order":2
},{
	"txt":"CREATE LAN",
	"h_start":-1,
	"v_start":310,
	"width_size_order":1,
	"height_size_order":2
},{
	"txt":"JOIN LAN",
	"h_start":-1,
	"v_start":340,
	"width_size_order":1,
	"height_size_order":2
},
]
msg_cnt = len(message_list)

code = '''\
	/*******************************************************/
	/**** Message display block, generated by sv_gen.py ****/
	/*******************************************************/

	// Is message visible
	logic [%d:0] msg_visible;
	// Whether show message
	logic [%d:0] msg_display;
	// Selected message index, 0 means none is selected
	logic [%d:0] msg_selected;
	// Whether the current message is selected
	logic cur_msg_selected;
	// Font pixel index of a line, [0,8)
	logic [2:0] font_pix_idx;
	logic [7:0] font_addr;
	logic [7:0] font_val;
	soc_system_font font(.address(font_addr),.clk(clk),.clken(1),.reset_req(0),.readdata(font_val));

	always_ff @(posedge clk) begin\
'''%(msg_cnt,msg_cnt,math.ceil(math.log2(msg_cnt)))
for idx,msg in enumerate(message_list):
	txt=msg['txt']
	h_start=msg['h_start']
	v_start=msg['v_start']
	msg_idx=idx+1
	width_size_order=msg['width_size_order']
	height_size_order=msg['height_size_order']

	idx = []
	for c in txt.upper():
		i = ord(c)-ord('A')
		if ord(c) >= ord('0') and ord(c) <= ord('9'):
			i=ord(c)-ord('0')+26
		if c==' ':
			i=36
		elif c=='!':
			i=37
		elif c=='?':
			i=38
		elif c==',':
			i=39
		elif c=='.':
			i=40
		elif c=='+':
			i=41
		elif c=='-':
			i=42
		elif c=='*':
			i=43
		elif c=='/':
			i=44
		idx.append(i)

	seg_width = pow(2,width_size_order)
	seg_height = pow(2,height_size_order)
	str_len = len(txt)
	str_len_order = math.ceil(math.log2(str_len))

	if v_start == -1:
		v_start = (480-seg_height*5)/2
	if h_start == -1:
		h_start = (640-seg_width*8*str_len)/2

	code +='''
		/**
		* message: %s
		* index: %d
		* h_start: %d
		* v_start: %d
		* font_width: %d
		* font_height: %d
		*/
		if((hcount[10:1] >= 10'd%d) && (hcount[10:1] < 10'd%d) && (vcount >= 10'd%d) && (vcount < 10'd%d) && msg_visible[%d]) begin
			msg_display[%d] <= 1;
			cur_msg_selected <= (msg_selected==%d);
'''%(txt,msg_idx,h_start,v_start,8*seg_width,5*seg_height,h_start,h_start+8*seg_width*str_len,v_start,v_start+5*seg_height,msg_idx,msg_idx,msg_idx)

	code +="			case((hcount[10:1]-%d)>>%d)\n"%(h_start,width_size_order+3)

	for i in range(str_len):
		code +="				%d'd%d: font_addr <= 8'd%d+((vcount[9:0]-%d)>>%d);\n"\
			%(str_len_order,i,idx[i]*5,v_start,height_size_order)

	code +='''				default:;
			endcase
			font_pix_idx <= (hcount[10:1]-%d)>>%d;
		end
		else
			msg_display[%d] <= 0;
'''%(h_start,width_size_order,msg_idx)

code +='''	end

	/*******************************************************/
	/************** Message display block end **************/
	/*******************************************************/
 
	/*
	 * You may need to put the following code in the combinational logic.
		// Draw font
		if(msg_display && font_val[font_pix_idx]) begin
			// Font selected			
			if(cur_msg_selected)
				{VGA_R, VGA_G, VGA_B} = 24'h00ffff;
			else
				{VGA_R, VGA_G, VGA_B} = 24'h000000;
		end
	 */
'''

print(code)

with open("display.sv","w") as f:
    f.write(code)