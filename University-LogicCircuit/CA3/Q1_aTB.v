`timescale 1ns/1ns
module Q1_aTB();
	reg [7:0] A, B;
	reg [1:0] m;
	reg Ci;
	wire [7:0] W;
	wire Co;
	Q1_aa CUT44(A , B , m , Ci , W , Co);
	initial begin 
	A = 8'b10111000;
	B = 8'b11111011;
	Ci = 1;
	m = 2'b00;
	#20 m = 2'b01;
	#20 m = 2'b11;
	Ci = 0;
	#20 m = 2'b10;
	#20 A = 8'b11111111;
	#20 m = 2'b00;
	#20 m = 2'b01;
	Ci = 1;
	#10 repeat (20) begin
	#20 m = $random;
	A = $random;
	Ci = ~Ci;
	end
	#40 $stop ; 
	end
endmodule
