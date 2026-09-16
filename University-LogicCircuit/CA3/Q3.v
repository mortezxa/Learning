`timescale 1ns/1ns
module Q3(input S , R , clk , output Q , Qi);
	wire j , k ;
	nand #8 n1(j , S , clk);
	nand #8 n2(k , R , clk);
	nand #8 n3(Q , j , Qi);
	nand #8 n4(Qi , k , Q);
endmodule
