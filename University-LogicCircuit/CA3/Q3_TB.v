`timescale 1ns/1ns
module Q3_TB();
	reg RR = 0 , SS = 0 , C = 1 ;
	wire QQ , QQi;
	Q3 CUT(SS , RR , C , QQ , QQi);
	initial begin
		#32 SS = 1;
		#60 C = 0 ; 
		#32 SS = 0; 
		#32 RR = 1;
		#32 C = 1 ;
		#32 RR = 0;
		#32 RR = 1; 
		#32 RR = 0; 
		#32 SS = 1;
		#32 RR = 1;
		#32 RR = 0;
		#50 $stop;
	end
endmodule 