`timescale 1ns/1ns
module Q4 (input D , clk , rst ,  output Q , Qi);
	wire D_inv , s , r ;
	not #6 i1(D_inv , D);
	not #6 i2(rst_inv , rst);
        nor #12 i3(s , rst , D_inv);
        nand #8 i4(r , D , rst_inv); 
  	Q3 Dlatch(s , r , clk , Q , Qi);
endmodule
