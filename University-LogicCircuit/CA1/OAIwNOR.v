`timescale 1ns/1ns
module OAIwNOR(input a , b , c ,output w);
wire j;
wire y;
wire s;
MyNOR NOR1(a,b,j);
MyNOR NOR2(c,c,y);
MyNOR NOR3(j,y,s); 
MyNOR NOR4(s,s,w);
endmodule
