`timescale 1ns/1ns
module CA5_whole5 (input clk, rst, rcIn, output logic txOut, txValid, txAbort);

    parameter [1:0] IDLE = 2'b00, TRANSMIT = 2'b01, ABORT = 2'b10;

    logic [1:0] curr_state, next_state;
    logic [7:0] counter;
    logic cnt_overflow, load_cnt, count_en;

    logic start_event, abort_event;

    CA4_module u_abort (.clk(clk), .rst(rst), .rcIn(rcIn), .abort_pulse(abort_event));
    CA4_module u_start  (.clk(clk), .rst(rst), .rcIn(~rcIn), .abort_pulse(start_event));

    always_ff @(posedge clk or posedge rst) begin
        if (rst)
            curr_state <= IDLE;
        else
            curr_state <= next_state;
    end

    always_comb begin
        next_state = curr_state;
        load_cnt = 1'b0;
        count_en = 1'b0;

        txOut = 1'b0;
        txValid = 1'b0;
        txAbort = 1'b0;

        case (curr_state)
            IDLE: begin
                txOut = 1'b1;
                if (start_event) begin
                    next_state = TRANSMIT;
                    load_cnt = 1'b1;
                end
            end

            TRANSMIT: begin
                count_en = 1'b1;
                txOut = rcIn;
                txValid = 1'b1;
                if (abort_event)
                    next_state = ABORT;
                else if (cnt_overflow)
                    next_state = IDLE;
            end

            ABORT: begin
                txOut = 1'b1;
                txAbort = 1'b1;
                next_state = IDLE;
            end
        endcase
    end

    always_ff @(posedge clk or posedge rst) begin
        if (rst) begin
            counter <= 8'd0;
            cnt_overflow <= 1'b0;
        end
        else if (load_cnt) begin
            counter <= 8'd159;
            cnt_overflow <= 1'b0;
        end
        else if (count_en) begin
            {cnt_overflow, counter} <= counter + 1'b1;
        end
        else begin
            cnt_overflow <= 1'b0;
        end
    end

endmodule

