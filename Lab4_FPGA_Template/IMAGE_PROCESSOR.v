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
	RESULT 
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


output reg[2:0] RESULT;

reg [9:0] countBLUE;
reg [9:0] countRED;
reg [9:0] countNULL;
reg [9:0] R_CNT_THRESHOLD = 10'd60;
reg [9:0] B_CNT_THRESHOLD = 10'd60;
reg lastsync = 1'b0;

localparam RED   = 8'b111_000_00;

localparam BLUE  = 8'b000_000_11;

//assign RESULT = 3'b111;
always @(posedge CLK) begin
//  RESULT = 3'b110;
	if(VGA_PIXEL_X>((`SCREEN_WIDTH/2)-35)&& VGA_PIXEL_X<((`SCREEN_WIDTH/2)+35) && VGA_PIXEL_Y<((`SCREEN_HEIGHT/2)+25)&& VGA_PIXEL_Y>((`SCREEN_HEIGHT/2)-25)) begin
//   if(!(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))) begin
		//if(VGA_HREF_NEG) begin
		if(PIXEL_IN == BLUE) begin
			countBLUE = countBLUE + 10'd1;
		end
		else if(PIXEL_IN == RED) begin
			countRED = countRED +10'd1; 
			end
		else begin
			countNULL = countNULL + 10'd1;
		end
		
	end
	if(VGA_VSYNC_NEG == 1'b1 && lastsync == 1'b0) begin
//			if(countRED >countBLUE) begin
//				RESULT = 3'b010;
//			end
//			else if(countBLUE >countRED) begin 
//				RESULT = 3'b100;
//			end
		if(countRED >= R_CNT_THRESHOLD) begin
			RESULT = 3'b010;
		end
		else if(countBLUE >= B_CNT_THRESHOLD) begin 
			RESULT = 3'b100;
		end
		else begin
			RESULT = 3'b111;
		end
	end
	else if(VGA_VSYNC_NEG == 1'b0 && lastsync == 1'b1) begin
		countBLUE = 10'b0;
		countRED  = 10'b0;
		countNULL = 10'b0;
	end
	lastsync = VGA_VSYNC_NEG;
end

endmodule

