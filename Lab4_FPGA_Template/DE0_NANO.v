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
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'b00000000;


///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
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

assign GPIO_0_D[5]   = VGA_VSYNC_NEG;
assign GPIO_0_D[7]   = VGA_HSYNC_NEG;


assign CAM_HREF_NEG  = GPIO_1_D[22]; //arbitrarily chosen for href input, v
assign CAM_VSYNC_NEG = GPIO_1_D[23];
assign CAM_PCLK      = GPIO_1_D[20];

assign VGA_RESET     = ~KEY[0];

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
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
wire [7:0] data;
assign GPIO_0_D[32]  = clk_24_pll;

assign data[7] = GPIO_1_D[32];
assign data[6] = GPIO_1_D[33];
assign data[5] = GPIO_1_D[30];
assign data[4] = GPIO_1_D[31];
assign data[3] = GPIO_1_D[28];
assign data[2] = GPIO_1_D[29];
assign data[1] = GPIO_1_D[26];
assign data[0] = GPIO_1_D[27];

/*
always @ (posedge CAM_PCLK) begin
		WRITE_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>=0 &&VGA_PIXEL_X<=22 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			pixel_data_RGB332 = 8'b11111111;
			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
			end
		else if(VGA_PIXEL_X>22 &&VGA_PIXEL_X<=44&&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b00011111;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>44 &&VGA_PIXEL_X<=66 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b11111100;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>66 &&VGA_PIXEL_X<=88 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b00011100;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>88 &&VGA_PIXEL_X<=110 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b11100011;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>110 &&VGA_PIXEL_X<=132 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b00101111;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>132 &&VGA_PIXEL_X<=154 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b11100101;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else if(VGA_PIXEL_X>154 &&VGA_PIXEL_X<=176 &&VGA_PIXEL_Y<(`SCREEN_HEIGHT-1))begin
			//pixel_data_RGB332 = 8'b00000000;
//			pixel_data_RGB332[7:4] = {GPIO_1_D[32], GPIO_1_D[30], GPIO_1_D[25], GPIO_1_D[29]};
//			pixel_data_RGB332[3:0] = {GPIO_1_D[29], GPIO_1_D[31], GPIO_1_D[28], GPIO_1_D[33]};
//				W_EN = 1'b1;
		end
		else begin
//				W_EN = 1'b0;
		end
end
*/

///////////////////////////////////////////////
//         Reading Camera input              //
///////////////////////////////////////////////

//reg reset   = 1'b0;

/*
//new image 
always @(negedge CAM_VSYNC_NEG) begin 
	reset = 1'b1;
end 
//new row
always @(posedge CAM_HREF_NEG)begin 
	if (reset == 1'b1) begin
		reset = 1'b0;
		//switch pixels or something? 
	end
	
	if (newByte == 1'b0) begin
		pixel_data_RGB332[7:4] = data[7:4];
		W_EN = 1'b1;
	
	end
		
	if (newByte == 1'b1) begin
		pixel_data_RGB332[3:0] = data[4:1];
		W_EN = 1'b1;
	end
	//store info in m9k block
		
end
*/

//for each byte (1 and 2)
//always @(posedge CAM_PCLK)begin 
/*
	always @(posedge CAM_VSYNC_NEG) begin
		X_ADDR = 0;
		Y_ADDR = 0;
		always @(posedge CAM_HREF_NEG) begin
			always @(posedge CAM_PCLK)begin
				if (newByte == 1'b0) begin	
					pixel_data_RGB332[7:4] = data[7:4];
					W_EN = 1'b1;
				end
				if (newByte == 1'b1) begin
					pixel_data_RGB332[3:0] = data[4:1];
					W_EN = 1'b1;
				end
				W_EN = 1'b0;
				X_ADDR = X_ADDR + 1;
				newByte = ~newByte;
			end
		Y_ADDR = Y_ADDR + 1;
		end
	end
*/
reg newByte = 1'b0;
assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);
//assign data[7:0] = RED;
reg prevHref;
always @(posedge CAM_PCLK) begin
	W_EN = 1'b0;
	if (CAM_VSYNC_NEG) begin
		X_ADDR = 0;
		Y_ADDR = 0;
	end
	if (~CAM_HREF_NEG && prevHref) begin
		Y_ADDR = Y_ADDR + 1;
		X_ADDR = 0;
	end
	if (newByte == 1'b0 && CAM_HREF_NEG ) begin	
		//W_EN = 1'b1;
		pixel_data_RGB332[7:2] = {data[7], data[6], data[5], data[2], data[1], data[0]};
		newByte = 1'b1;
	end
	else if (newByte == 1'b1 && CAM_HREF_NEG) begin
		pixel_data_RGB332[1:0] = data[4:3];
		X_ADDR = X_ADDR + 1;
		W_EN = 1'b1;
		newByte = 1'b0;
	end

	
	prevHref = CAM_HREF_NEG;
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
		if(VGA_PIXEL_X%3==0 || VGA_PIXEL_Y%3==0)begin
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
