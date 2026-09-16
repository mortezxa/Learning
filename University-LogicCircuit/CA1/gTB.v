`timescale 1ns/1ns
module gTB();
reg aa0 = 0;
reg aa1 = 0;
reg bb0 = 0;
reg bb1 = 0;
wire gg;
g_Fun CUT10(aa0,aa1,bb0,bb1,gg);
initial begin
#90 aa1 = 1;
#20 aa0 = 1;
#50 bb1 = 1;
#50 aa1 = 1;
#20 bb0 = 0;
#50 repeat (5) #40 aa1=~aa1 ;
#50 $stop;
end
endmodule
