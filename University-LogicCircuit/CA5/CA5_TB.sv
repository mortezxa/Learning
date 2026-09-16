`timescale 1ns/1ns

module CA5_TB;

    logic clk_tb, rst_tb, rcIn_tb;
    logic txOut_tb, txValid_tb, txAbort_tb;
    logic txOut_ref, txValid_ref, txAbort_ref;


    CA5_whole5 dut_inst (
        .clk(clk_tb),
        .rst(rst_tb),
        .rcIn(rcIn_tb),
        .txOut(txOut_tb),
        .txValid(txValid_tb),
        .txAbort(txAbort_tb)
    );


    CA5_whole ref_inst (
        .clk(clk_tb),
        .rst(rst_tb),
        .rcIn(rcIn_tb),
        .txOut(txOut_ref),
        .txValid(txValid_ref),
        .txAbort(txAbort_ref)
    );


    always #5 clk_tb = ~clk_tb;


    task automatic transmit_frame(input [7:0] data_frame);
        integer k;
        begin
            for (k = 7; k >= 0; k = k - 1) begin
                rcIn_tb = data_frame[k];

            end
        end
    endtask

    initial begin

        clk_tb = 1'b0;
        rst_tb = 1'b1;
        rcIn_tb = 1'b1;

        #20;
        rst_tb = 1'b0;
        #20;

        $display(">>> Transmitting START frame: 01111110");
        transmit_frame(8'b01111110);

        #200;

        $display(">>> Transmitting ABORT frame: 10000001");
        transmit_frame(8'b10000001);

        #200;

        $display(">>> Simulation completed.");
        $stop;
    end

endmodule

