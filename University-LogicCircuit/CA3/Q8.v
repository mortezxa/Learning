`timescale 1ns/1ns
module Q8(input [7:0] B, input [1:0] m, input Ci, input clk, input rst, output [7:0] Count, output Co);
    wire [7:0] W;
    Q1_aa z1(Count , B , m , Ci , W, Co);
    Q7_Register z2(W , clk , rst , Count);
endmodule
