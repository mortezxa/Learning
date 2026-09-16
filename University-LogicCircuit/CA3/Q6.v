`timescale 1ns/1ns
module Q6(input [7:0] B, input [1:0] m, input Ci, input clk, input rst, output [7:0] Count, output Co);
    wire [7:0] W;
    Q1_aa m1(Count , B , m , Ci , W, Co);
    Q5 m2(W , clk , rst , Count);
endmodule