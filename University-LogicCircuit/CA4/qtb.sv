`timescale 1ns/1ns

module CA4_module_tb();

  logic clk, rst, j;
  logic w1, w2;
  
  CA4_modulep uut2 (
    .clk(clk),
    .rst(rst),
    .j(j),
    .w(w2)
  );

  CA4_module uut1 (
    .clk(clk),
    .rst(rst),
    .j(j),
    .w(w1)
  );

  initial begin
    clk = 0;
    forever #10 clk = ~clk;
  end

  initial begin
    rst = 1;
    j = 0;
    #25;
    
    rst = 0;
    #20;

    send(1);
    send(0);
    repeat(3) send(0);
    send(1);
    send(1);
    send(0);

    send(1);
    send(0);
    send(0);
    send(1);

    #100;
    $finish;
  end

  task send(input logic bitval);
    begin
      j = bitval;
      #40;
    end
  endtask

endmodule
