module CA5_compelete (
    input  logic clk, rst, rcIn,
    output logic txOut, txValid, txAbort
);

    logic start_sig;
    logic abort_sig;

    CA4_module u_start_det (
        .clk(clk),
        .rst(rst),
        .rcIn(~rcIn),
        .abort_pulse(start_sig)
    );

    CA4_module u_abort_det (
        .clk(clk),
        .rst(rst),
        .rcIn(rcIn),
        .abort_pulse(abort_sig)
    );

    CA5_module5 tx_ctrl (
        .clk(clk),
        .rst(rst),
        .rcIn(rcIn),
        .start_pulse(start_sig),
        .abort_pulse(abort_sig),
        .txOut(txOut),
        .txValid(txValid),
        .txAbort(txAbort)
    );

endmodule


