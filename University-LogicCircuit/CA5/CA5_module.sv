module CA5_module5 (
    input  logic clk, rst, rcIn, start_pulse, abort_pulse,
    output logic txOut, txValid, txAbort
);

    parameter [1:0] IDLE = 2'b00, TRANSMIT = 2'b01, ABORT = 2'b10;

    logic [1:0] state_curr, state_next;
    logic [7:0] counter;
    logic counter_done, load_counter, enable_counter;

    always_ff @(posedge clk or posedge rst) begin
        if (rst)
            state_curr <= IDLE;
        else
            state_curr <= state_next;
    end

    always_comb begin
        state_next = state_curr;
        load_counter = 1'b0;
        enable_counter = 1'b0;

        txOut = 1'b0;
        txValid = 1'b0;
        txAbort = 1'b0;

        case (state_curr)
            IDLE: begin
                txOut = 1'b1;
                if (start_pulse) begin
                    state_next = TRANSMIT;
                    load_counter = 1'b1;
                end
            end

            TRANSMIT: begin
                enable_counter = 1'b1;
                txOut = rcIn;
                txValid = 1'b1;
                if (abort_pulse)
                    state_next = ABORT;
                else if (counter_done)
                    state_next = IDLE;
            end

            ABORT: begin
                txOut = 1'b1;
                txAbort = 1'b1;
                state_next = IDLE;
            end
        endcase
    end

    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            counter <= 8'd0;
            counter_done <= 1'b0;
        end
        else if (load_counter) begin
            counter <= 8'd159;
            counter_done <= 1'b0;
        end
        else if (enable_counter) begin
            {counter_done, counter} <= counter + 1'b1;
        end
        else begin
            counter_done <= 1'b0;
        end
    end

endmodule
