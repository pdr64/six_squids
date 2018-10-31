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
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'b00000000;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
reg [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 


///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

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
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : 8'b000_000_11),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(clk_25_pll),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////

always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		WRITE_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>=0 &&VGA_PIXEL_X<=22 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b11111111;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>22 &&VGA_PIXEL_X<=44&&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b00011111;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>44 &&VGA_PIXEL_X<=66 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b11111100;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>66 &&VGA_PIXEL_X<=88 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b00011100;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>88 &&VGA_PIXEL_X<=110 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b11100011;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>110 &&VGA_PIXEL_X<=132 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b00101111;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>132 &&VGA_PIXEL_X<=154 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b11100101;
				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>154 &&VGA_PIXEL_X<=176 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b00000000;
				W_EN = 1'b1;
		end
		else begin
				W_EN = 1'b0;
		end
end

always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end

/*
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X%5==0 || VGA_PIXEL_Y%5==0)begin
				VGA_READ_MEM_EN = 1'b1;
		end
		else begin
				VGA_READ_MEM_EN = 1'b0;
		end
end
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X%3==0 || VGA_PIXEL_Y%3                                 ==0)begin
				VGA_READ_MEM_EN = 1'b1;
		end
		else begin
				VGA_READ_MEM_EN = 1'b0;
		end
end
*/
/*
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		WRITE_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if( ( VGA_PIXEL_X == 88 ) || ( VGA_PIXEL_Y == 72 ) ) begin
				W_EN = 1'b1;
		end
		
		else begin
				W_EN = 1'b0;
		end
end


always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if( ( VGA_PIXEL_X == 88 ) || ( VGA_PIXEL_Y == 72 ) ) begin
				VGA_READ_MEM_EN = 1'b0;
		end
		
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end
*/

//	GPIO_0_D pixel_data_RGB332 == ????
/*
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end
*/	
	
endmodule 
