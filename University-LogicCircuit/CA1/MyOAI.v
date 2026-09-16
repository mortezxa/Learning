`timescale 1ns/1ns
module MyOAI ( input a , b , c  , output w);
  supply1 Vdd ;
  supply0 Gnd;
  wire j;
  wire y;
  pmos#(5,6,7)T1(j , Vdd , a);
  pmos#(5,6,7)T2(w , j , b);
  pmos#(5,6,7)T3(w , Vdd , c);
  nmos#(3,4,5)T4(y , Gnd , b);
  nmos#(3,4,5)T5(y , Gnd , a);
  nmos#(3,4,5)T6(w , y , c);
endmodule
