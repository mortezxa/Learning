`timescale 1ns/1ns
module MyOAITB ();
reg aa = 1;
reg bb = 0;
reg cc = 1;
wire ww;
MyOAI CUT2(aa , bb , cc , ww);
initial begin
#30 aa = 0;
#30 aa = 1;
#30 cc = 0;
#30 bb = 1;
#30 aa = 0;
#30 cc = 1;
#40;
end
endmodule