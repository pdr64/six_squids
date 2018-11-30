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
reg [9:0] R_CNT_THRESHOLD = 10'd80;
reg [9:0] B_CNT_THRESHOLD = 10'd80;
reg lastsync = 1'b0;

reg [9:0] red1 = 10'b0;
reg [9:0] red2 = 10'b0;
reg [9:0] red3 = 10'b0;
reg [9:0] blue1 = 10'b0;
reg [9:0] blue2 = 10'b0;
reg [9:0] blue3 = 10'b0;
reg [9:0] firstRED;
reg [9:0] firstBLUE;
reg [9:0] lastRED;
reg [9:0] lastBLUE;

localparam RED   = 8'b111_000_00;
localparam BLUE  = 8'b000_000_11;

always @(posedge CLK) begin
	if(VGA_PIXEL_X>((`SCREEN_WIDTH/2)-25)&& VGA_PIXEL_X<((`SCREEN_WIDTH/2)+25) && VGA_PIXEL_Y<((`SCREEN_HEIGHT/2)+30)&& VGA_PIXEL_Y>((`SCREEN_HEIGHT/2)-30)) begin
		
		if(PIXEL_IN == BLUE) begin
			countBLUE = countBLUE + 10'd1;
			lastBLUE = VGA_PIXEL_Y;
			if(countBLUE==10'd1) firstBLUE= VGA_PIXEL_Y;
				
		end
		else if(PIXEL_IN == RED) begin
			countRED = countRED +10'd1;
			lastRED = VGA_PIXEL_Y;
			if(countRED==10'd1) firstRED= VGA_PIXEL_Y;
		end	
	
		if(VGA_PIXEL_Y==firstRED+((lastRED-firstRED)*(1/3)) || VGA_PIXEL_Y ==((lastBLUE-firstBLUE)*(1/3)))begin
			
			blue1 = countBLUE;
			red1=countRED;
		end
		else if((VGA_PIXEL_Y==firstRED+((lastRED-firstRED)*(2/3))) || VGA_PIXEL_Y ==(firstBLUE+lastBLUE*(2/3)))begin
		
			blue2 =  countBLUE;
			red2=countRED;
		end
		else if((VGA_PIXEL_Y==firstRED+(lastRED-firstRED)) || VGA_PIXEL_Y==firstBLUE +(lastBLUE-firstBLUE))begin
		
			blue3 = countBLUE;
			red3=countRED;
		
		end
	
	end
	if(VGA_VSYNC_NEG == 1'b1 && lastsync == 1'b0) begin
		//if it is a red diamond (001), if it is red square (011), if it is red triangle (010)
		//if it is a blue diamond (100), if it is blue square (110), if it is blue triangle (101)
		if(countRED >= R_CNT_THRESHOLD) begin
			
			if(red1<(red2-red1)&& (red2-red1)<(red3-red2)) begin  
				RESULT = 3'b010; end// triangle
			else if(red1<(red2-red1) && (red3-red2)<(red2-red1)) begin
				RESULT= 3'b001; end  // diamond
			else if((red1+red2)-(red2+red3)<= 10'd5 ||(red3+red2)-(red1+red2)<= 10'd5) RESULT = 3'b011;
			//else RESULT = 3'b111;                                                
		end
		else if(countBLUE >= B_CNT_THRESHOLD) begin 
			
			if(blue1<(blue2-blue1)&& (blue2-blue1)<(blue3-blue2)) RESULT = 3'b101; // triangle
			else if(blue1<(blue2-blue1) && (blue3-blue2)<(blue2-blue1)) RESULT= 3'b100;   // diamond
			else if((blue1+blue2)-(blue3+blue2)<= 10'd5 ||(blue3+blue2)-(blue1+blue2)<= 10'd5) RESULT = 3'b110;
			//else RESULT = 3'b111; 																		
		end

		else begin
			RESULT = 3'b111;																				//null color
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
