`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_HREF_NEG,
	VGA_VSYNC_NEG,
	RESULT, 
	//CAM_HREF_NEG
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;
input       VGA_HREF_NEG;

output[2:0] RESULT;

reg RESULT;
reg [15:0] countBLUE;
reg [15:0] countRED;
reg [15:0] countNULL;
reg [15:0] R_CNT_THRESHOLD = 16'd2000;
reg [15:0] B_CNT_THRESHOLD = 16'd4000;
reg lastsync = 1'b0;

always @(posedge CLK) begin
	if(VGA_PIXEL_X>((`SCREEN_WIDTH/2)-50)&& VGA_PIXEL_X<((`SCREEN_WIDTH/2)+50) || VGA_PIXEL_Y>((`SCREEN_HEIGHT/2)-50)&& VGA_PIXEL_Y<((`SCREEN_HEIGHT/2)+50)) begin
		if(VGA_HREF_NEG) begin
			if(PIXEL_IN[3:0] == 4'b1111) begin
				countBLUE = countBLUE + 16'd1;
			end
			else if(PIXEL_IN[7:4] >=4'b0011) begin
				countRED = countRED +16'd1; 
			end
			else begin
				countNULL = countNULL + 16'd1;
			end
		end
		if(VGA_VSYNC_NEG == 1'b1 && lastsync == 1'b0) begin
			if(countRED > R_CNT_THRESHOLD) begin
				RESULT = 3'b010;
			end
			else if(countBLUE > B_CNT_THRESHOLD) begin 
				RESULT = 3'b100;
			end
			else begin
				RESULT = 3'b111;
			end
		end
		if(VGA_VSYNC_NEG == 1'b0 && lastsync == 1'b1) begin
			countBLUE = 16'b0;
			countRED  = 16'b0;
			countNULL = 16'b0;
		end
		lastsync = VGA_VSYNC_NEG;
	end
end

endmodule
