`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY	
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED   = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE  = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [23:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'b00000000;


///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire[14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 


///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
wire        CAM_HREF_NEG;
wire        CAM_VSYNC_NEG;
wire        CAM_PCLK; 
reg			VGA_READ_MEM_EN;
///// I/O for Img Proc /////
wire [2:0] RESULT;

assign GPIO_0_D[5]   = VGA_VSYNC_NEG;
assign GPIO_0_D[7]   = VGA_HSYNC_NEG;


assign CAM_HREF_NEG  = GPIO_1_D[3]; //arbitrarily chosen for href input, v
assign CAM_VSYNC_NEG = GPIO_1_D[1];
assign CAM_PCLK      = GPIO_1_D[7];

assign VGA_RESET     = ~KEY[0];



/* WRITE ENABLE */
reg W_EN;

wire clk_24_pll;
wire clk_25_pll;
wire clk_50_pll;


lab4 lab4_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( clk_24_pll ),
	.c1 ( clk_25_pll ),
	.c2 ( clk_50_pll )
	);
///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(clk_50_pll),
	//.clk_R(CLK_25_PLL), // DO WE NEED TO READ SLOWER THAN WRITE??
	.clk_R(clk_25_pll),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(clk_25_pll),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(VGA_HSYNC_NEG),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////

IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(clk_25_pll),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_HREF_NEG(CAM_HSYNC_NEG),
	.VGA_VSYNC_NEG(CAM_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
wire [7:0] DATA;
assign GPIO_0_D[32]  = clk_24_pll;

assign DATA[7] = GPIO_1_D[23];
assign DATA[6] = GPIO_1_D[21];
assign DATA[5] = GPIO_1_D[19];
assign DATA[4] = GPIO_1_D[17];
assign DATA[3] = GPIO_1_D[15];
assign DATA[2] = GPIO_1_D[13];
assign DATA[1] = GPIO_1_D[11];
assign DATA[0] = GPIO_1_D[9];



assign GPIO_0_D[33]   = RESULT[1];   // this is b
assign GPIO_0_D[31]   = RESULT[0];   // this is c
assign GPIO_0_D[30]   = RESULT[2];    // this is a
//
//assign GPIO_0_D[33]   = 1'b1;
//assign GPIO_0_D[31]   = 1'b1;
//assign GPIO_0_D[30]   = 1'b1;

///////////////////////////////////////////////
//         Reading Camera input              //
///////////////////////////////////////////////



reg newByte = 1'b0;
assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);
//assign data[7:0] = RED;
reg prevHref;
reg prevVsync;
reg [15:0] TEMP;
always @(posedge CAM_PCLK) begin
	if (CAM_VSYNC_NEG & ~prevVsync) begin //new frame
		X_ADDR  = 10'b0;
		Y_ADDR  = 10'b0;
		newByte = 1'b0;
	end
	else if (~CAM_HREF_NEG & prevHref) begin //new row
		Y_ADDR  = Y_ADDR + 10'b1;
		X_ADDR  = 10'b0;
		newByte = 1'b0;
	end
	else begin
		Y_ADDR = Y_ADDR;
		if (CAM_HREF_NEG) begin
			if (newByte == 1'b0) begin
				TEMP[7:0]= DATA[7:0];
				W_EN      = 1'b0;
				X_ADDR    = X_ADDR;
				newByte   = 1'b1;
				
			end
			else begin
			    TEMP[15:8] = DATA[7:0];
			   if(TEMP[7:4] >= 4'b0010 && TEMP[11:8] > 4'b0010 && TEMP[3:0] > 4'b0010)begin
					pixel_data_RGB332 = 8'b11111111; end
				else if(TEMP[11:8]>=4'b0011)begin
					pixel_data_RGB332 = RED; end
//				else if(TEMP[7:4]>=4'b0111)begin
//					pixel_data_RGB332 = GREEN; end
				else if(TEMP[3:0]>=4'b0010)begin
					pixel_data_RGB332 = BLUE; end
				else begin
					pixel_data_RGB332 = 8'b00000000; end
//				if(TEMP[11:8]>TEMP[7:4] && TEMP[11:8]>TEMP[3:0])begin
//					pixel_data_RGB332 = RED; end
////				else if(TEMP[7:4]>TEMP[11:8] && TEMP[7:4]>TEMP[3:0])begin
////					pixel_data_RGB332 = GREEN; end
//				else if(TEMP[3:0]>TEMP[11:8] && TEMP[3:0]>TEMP[7:4])begin
//					pixel_data_RGB332 = BLUE; end
//				else if(TEMP[7:4]==TEMP[11:8] && TEMP[11:8] == TEMP[3:0] && TEMP[3:0]==4'b1111)begin
//					pixel_data_RGB332 = 8'b11111111; end
//				else begin
//					pixel_data_RGB332 = 8'b00000000; end
//				pixel_data_RGB332= {TEMP[11:8],TEMP[3:0]};

				X_ADDR = X_ADDR + 10'b1;
				W_EN = 1'b1;
				newByte = 1'b0;
			end
		end
		else begin
			X_ADDR = 10'b0;
		end
	end
	prevVsync = CAM_VSYNC_NEG;
	prevHref  = CAM_HREF_NEG;
end

//reading
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end



endmodule 
