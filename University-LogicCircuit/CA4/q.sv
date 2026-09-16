`timescale 1ns/1ns 
module CA4_module (input clk, rst, j, output logic w);

  parameter [1:0] S0 = 2'b00, S1 = 2'b01, S2 = 2'b10, S3 = 2'b11;

  logic [1:0] current_state, next_state;
  logic [2:0] counter;
  logic count_enable, load_counter;
  logic carry_out;

  always @(posedge clk, posedge rst) begin
    if (rst)
      current_state <= S0;
    else
      current_state <= next_state;
  end

  always @(current_state, j, counter, carry_out) begin
    next_state = current_state;
    load_counter = 0;
    count_enable = 0;
    w = 0;

    case (current_state)
      S0: begin
        if (j) next_state = S1;
        else next_state = S0;
      end

      S1: begin
        load_counter = 1;
        if (~j) next_state = S2;
        else next_state = S1;
      end

      S2: begin
        if (~j) begin
          if (~carry_out) begin
            count_enable = 1;
            next_state = S2;
          end else begin
            next_state = S0;
          end
        end else begin
          if (carry_out)
            next_state = S3;
          else
            next_state = S1;
        end
      end

      S3: begin
        w = 1'b1;
        if (j) next_state = S1;
        else next_state = S0;
      end
    endcase
  end

  always @(posedge clk, posedge rst) begin
    if (rst) begin
      counter <= 0;
      carry_out <= 0;
    end
    else if (load_counter) begin
      counter <= 3'b011;
      carry_out <= 0;
    end
    else if (count_enable) begin
      {carry_out, counter} <= counter + 1;
    end
    else carry_out <= 0;
  end
endmodule
