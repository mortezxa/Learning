`timescale 1ns/1ns
module MyNORTB ();
reg aa = 0;
reg bb = 1;
wire ww;
MyNOR CUT1(aa , bb , ww);
initial begin
#20 bb = 0;
#20 bb = 1;
#20 bb = 0;
#20 aa = 1;
#20 aa = 0;
#20 aa = 1;
bb=1;
#40 $stop;
end
endmodule