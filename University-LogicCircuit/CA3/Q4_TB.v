`timescale 1ns/1ns
module Q4_TB ();
	reg rst = 1 , clk = 1 , DD = 1 ;
	wire QQ , QQi; 
	Q4 CUT1(DD , clk , rst , QQ , QQi);
	initial begin 
	#50 DD = 0;
	#50 rst = 0;
	#50 DD = 1 ;
	#50 clk = 0;
	#50 repeat (5) #40 DD = ~DD;
	#50 clk = 1 ;
	#70 DD = ~DD; 
	#70 DD = ~DD;
	#100 $stop ; 
	end
endmodule