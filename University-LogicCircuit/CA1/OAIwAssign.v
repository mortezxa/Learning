`timescale 1ns/1ns
module OAIwAssign (input a , b , c , output w);
  assign #(10 , 14) w = ~((a|b)&c);
endmodule
