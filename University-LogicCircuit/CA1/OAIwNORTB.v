`timescale 1ns/1ns
module OAIwNORTB();
 reg aa = 0;
 reg bb = 0;
 reg cc = 0;
 wire ww;
 OAIwNOR CUT3(aa , bb , cc , ww);
 initial begin
 #50 cc = 1;
 #50 aa = 1;
 #50 bb = 1;
 #50 cc = 0;
 #50 cc = 1;
 #50 bb = 0;
 #50 aa = 0;
 #50 $stop;
 end
endmodule

