module Hazard(
    input wire rst,
    input wire [4:0] Rs1D,
    input wire [4:0] Rs2D,
    input wire [4:0] Rs1E,
    input wire [4:0] Rs2E,
    input wire [4:0] RdE,
    input wire pcmuxsel,
    input wire ResultSrcE0,
    input wire [4:0] RdM,
    input wire RegwriteM,
    input wire [4:0] RdW,
    input wire RegwriteW,
    output logic stallF,
    output logic stallD,
    output logic FlushD,
    output logic FlushE,
    output logic [1:0] ForwardAE,
    output logic [1:0] ForwardBE
);
    wire lwstall ;
    assign lwstall= (((Rs1D==RdE)|(Rs2D==RdE))&ResultSrcE0)?1:0;
    always@(posedge rst)begin
        stallF <= 1'd0;
        stallD <=1'd0;
        FlushD <= 1'd0;
        FlushE <= 1'd0;
        ForwardAE <= 2'd0;
        ForwardBE <= 2'd0;
    end
    
    always@(*) begin
        ForwardAE = ((Rs1E == RdM) & RegwriteM & (Rs1E != 5'b0)) ? 2'b10:
        ((Rs1E == RdW) & RegwriteW & (Rs1E != 5'b0))?2'b01:2'b00;

        ForwardBE =((Rs2E == RdM) & RegwriteM & (Rs2E != 5'b0)) ? 2'b10:
        ((Rs2E == RdW) & RegwriteW & (Rs2E != 5'b0))?2'b01:2'b00;
       
        stallF = lwstall;
        stallD = lwstall;
        FlushE = (lwstall | pcmuxsel);
        FlushD = pcmuxsel;
    end
endmodule