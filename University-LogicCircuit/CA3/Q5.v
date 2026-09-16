`timescale 1ns/1ns
module Q5 (input [7:0] Pi, input clk, rst, output [7:0] Po );

    genvar i;
    generate
        for (i = 0; i < 8; i = i + 1) begin : gen_ff
            Q4 ff_inst (.D(Pi[i]), .clk(clk), .rst(rst), .Q(Po[i]));
        end
    endgenerate

endmodule

