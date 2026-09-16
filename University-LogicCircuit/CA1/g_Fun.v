`timescale 1ns/1ns
module g_Fun(input a0 ,a1 , b0 , b1 , output g);
wire j;
wire k;
OAIwAssign j1(a1 , ~b1 , ~b0 , j);
OAIwAssign k1(a1 , a1 , ~b1 , k);
OAIwAssign g1(~a0 , j , k , g);
endmodule
