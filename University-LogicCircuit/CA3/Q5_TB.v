`timescale 1ns/1ns
module Q5_TB();
	reg [7:0] Parallel_in ; 
	reg clk = 1 , rst = 1;
	wire [7:0] Parallel_out;
	Q5 CUT2(Parallel_in , clk , rst , Parallel_out);
	initial begin 
	Parallel_in = 8'b0;
	#100 rst = 0;
	#100 Parallel_in = 8'b10110101;
	#100 Parallel_in = 8'b00101111;
	#100 rst = 1;
	#100 Parallel_in = 8'b11111111;
        #50 rst = 0;
	#100 clk = 0;
	#100 Parallel_in = 8'b0;
	#100 $stop;
	end
endmodule
