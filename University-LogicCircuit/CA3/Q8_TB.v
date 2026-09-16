`timescale 1ns/1ns
module Q8_TB();
	reg [7:0] B;
	reg [1:0] m;
	reg Ci , clk , rst;
	wire [7:0] Count;
	wire Co;
	Q8 CUT51(B , m , Ci , clk , rst , Count , Co);
	initial begin 
	B = 8'b0;
	m = 2'b11;
	clk = 1;
	Ci = 1;
	rst = 0;
	#20 clk = 0 ;
	#20 m = 2'b01;
	#60 ;
	repeat (60) #50 clk = ~clk;
	#60 rst = 1;
	#30 clk = 1;
	#30 rst = 0;
	repeat (50) #50 clk = ~clk;
	#25 Ci = 0;
	#60 m = 2'b10;
	repeat (30) #50 clk = ~clk;
	#20 clk = 0;
	#60 B = 8'b11111111;
	#20 m = 2'b11;
	#10 clk = 1;
	#50 clk = 0;
	#5 Ci = 1;
	#20 m = 2'b01;
	#10 clk = 1;
	#50 clk = 0;
	#60 $stop;  
	end
endmodule
