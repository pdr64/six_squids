`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	//VGA_HREF_NEG,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;
//input [22]  GPIO_1_D;
//wire        CAM_HREF_NEG;

//assign CAM_HREF_NEG  = GPIO_1_D[22];

output [2:0] RESULT;

reg RESULT;
reg [15:0] countBLUE;
reg [15:0] countRED;
reg [15:0] countNULL;
reg [15:0] R_CNT_THRESHOLD = 16'd20000;
reg [15:0] B_CNT_THRESHOLD = 16'd20000;
reg lastsync = 1'b0;

always @(posedge CLK) begin
	if(CAM_HREF_NEG) begin
		if(PIXEL_IN == 8'b0) begin
			countBLUE = countBLUE + 16'd1;
		end
		else if(PIXEL_IN[7:5] > 3'b010) begin
			countRED = countRED +16'b1; 
		end
		else begin
			countNULL = countNULL + 16'd1;
		end
	end
	if(VGA_VSYNC_NEG == 1'b1 && lastsync == 1'b0) begin
		if(countBLUE > B_CNT_THRESHOLD) begin
			RESULT = 3'b111;
		end
		else if(countRED > R_CNT_THRESHOLD) begin 
			RESULT = 3'b110;
		end
		else begin
			RESULT = 3'b000;
		end
	end
	if(VGA_VSYNC_NEG == 1'b0 && lastsync == 1'b1) begin
		countBLUE = 16'b0;
		countRED  = 16'b0;
		countNULL = 16'b0;
	end
	lastsync = VGA_VSYNC_NEG;
end

endmodule
