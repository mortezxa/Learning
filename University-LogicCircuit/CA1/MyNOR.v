`timescale 1ns/1ns
module MyNOR(input a , b , output w);
  supply1 Vdd;
  supply0 Gnd;
  wire y ;
  nmos#(3,4,5)T1(w,Gnd,a);
  nmos#(3,4,5)T2(w,Gnd,b);
  pmos#(5,6,7)T3(w,y,b);
  pmos#(5,6,7)T4(y,Vdd,a);
endmodule
