`timescale 1ns/1ns
module Q16(input [7:0] B_0to7, B_8to15 ,input [1:0] m ,input Ci ,input clk , rst, output [15:0] Count , output Co);
    wire Carry_out;
    Q8 j1(B_0to7 , m , Ci , clk , rst , Count[7:0] ,Carry_out);
    Q8 j2(B_8to15 , m , Carry_out , clk , rst , Count[15:8] , Co);
endmodule