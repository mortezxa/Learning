`timescale 1ns/1ns
module Q1_aa (input [7:0] A, B, input [1:0] m, input Ci, output reg [7:0] W, output Co );

always @(*) begin
    case (m)
        2'b00: W = A;
        2'b01: W = Ci ? A + 1 : A;
        2'b10: W = ~Ci ? A - 1 : A;
        2'b11: W = B;
        default: W = 8'b0;
    endcase
end

assign Co = (m == 2'b01 && Ci && A == 8'd255) ||
            (m == 2'b10 && ~Ci && A == 8'd0);

endmodule
