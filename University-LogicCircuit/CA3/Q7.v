`timescale 1ns/1ns
module Q7_Dflipflop (input D , clk , rst , output  Q , Qi);
	wire clk_inv , Q_1 , Qi_1 ;
	not #6 k1(clk_inv , clk);
	Q4 k2(D , clk , rst , Q_1 , Qi_1);
	Q4 k3(Q_1 , clk_inv , rst , Q , Qi);
endmodule