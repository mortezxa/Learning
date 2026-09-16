module adder #(
    parameter WIDTH = 32
) (
    input  wire signed [WIDTH - 1:0] a,
    input  wire signed [WIDTH - 1:0] b,
    output wire signed [WIDTH - 1:0] y
);
  
    assign y = a + b;
endmodule

module mux_2to1 #(
    parameter WIDTH = 32
) (
    input [WIDTH-1:0] a,
    input [WIDTH-1:0] b,
    input sel,
    output [WIDTH-1:0] out
);
    assign out = sel ? b : a;
endmodule

module mux_3to1 #(
    parameter WIDTH = 32
) (
    input [WIDTH-1:0] a,
    input [WIDTH-1:0] b,
    input [WIDTH-1:0] c,
    input [1:0] sel,
    output [WIDTH-1:0] out
);
    assign out = (sel==2'b00) ? a:
    (sel==2'b01) ? b:
    (sel==2'b10) ? c:32'd0;
endmodule
    

module ALU  #(
    parameter WIDTH = 32
) (
    input signed[WIDTH-1:0] a,
    input signed[WIDTH-1:0] b,
    input[2:0] s,
    output zero,
    output signed[WIDTH-1:0] w
);
    assign w= (s==3'b000)?a+b :
	  (s==3'b001)?a-b:
	  (s==3'b010)?a&b:
	  (s==3'b011)?a|b:
	  (s==3'b100)?((a<b)?{{31'b0},1'b1}:32'b0):
	  (s==3'b101)?b:
      (s==3'b110)?a^b:w;
    assign zero = (a==b);
endmodule

module data_memory #(
    parameter WIDTH = 32
)(
    input  wire clk,
    input wire rst,
    input  wire memWrite,
    input  wire [WIDTH-1:0] address,
    input  wire [WIDTH-1:0] writeData,
    output wire [WIDTH-1:0] readData,
    output wire [WIDTH-1:0] allmem [0:19]
);
    logic [WIDTH-1:0] mem [0:1023];

    wire [7:0] word_addr;
    assign word_addr = address[9:2];

    assign readData = (!memWrite) ? mem[word_addr]:readData;
    assign allmem = mem[0:19];
    always @(posedge clk,posedge rst) begin
        if(rst)begin
            foreach(mem[i])
            mem[i] <= 32'b0;
        end
        else if (memWrite)
            mem[word_addr] <= writeData;
    end
endmodule

module inst_mem #(//correction order input
    parameter WIDTH = 32
)(
    input clk,
    input [WIDTH-1:0] orders [0:1023],
    input wire [WIDTH-1:0]address,
    output logic [WIDTH-1:0]inst 
);
    assign inst = orders[{address[11:2]}];

endmodule

module imm_gen(
    input  wire [31:0] instr,
    input wire [2:0] Imm_sr,
    output reg  [31:0] imm_out
);
always@(instr,Imm_sr)begin
        case(Imm_sr)

           3'b000: //I_type
               imm_out = {{20{instr[31]}}, instr[31:20]};
            3'b001:  // S_type
                imm_out = {{20{instr[31]}}, instr[31:25], instr[11:7]};
            3'b010: //B_type
                imm_out = {{19{instr[31]}},
                    instr[31],
                    instr[7],
                    instr[30:25],
                    instr[11:8],
                    1'b0};
            3'b011:  // jal
                imm_out = {{11{instr[31]}},
                    instr[31],
                    instr[19:12],
                    instr[20],
                    instr[30:21],
                    1'b0};
            3'b100:  // lui
                imm_out = {instr[31:12], 12'b0};
            default:
                imm_out = 32'b0;
        endcase
end
endmodule

module main_reg(
    input clk,
    input wire Reg_write,
    input wire [4:0] read_reg1,
    input wire [4:0] read_reg2,
    input wire [4:0] write_reg,
    input wire [31:0] write_data,
    output wire [31:0] read_data1,
    output wire [31:0] read_data2,
    output wire [31:0] allregisters [0:31]
);
logic [31:0] mainregfile [0:31];
assign read_data1 = (read_reg1 == 5'd0) ? 32'd0 : mainregfile[read_reg1];
assign read_data2 = (read_reg2 == 5'd0) ? 32'd0 : mainregfile[read_reg2];
assign allregisters = mainregfile;
always@(posedge clk)begin
    if(Reg_write && (write_reg!=5'd0))
     mainregfile[write_reg] <= write_data; 
end
initial begin
    integer i;
    for (i = 0; i < 32; i = i + 1)
        mainregfile[i] = 32'd0;
end
endmodule

module register (
    input clk,
    input rst,
    input en,
    input wire [31:0] regin,
    output logic [31:0] regout
);
    always@(posedge clk,posedge rst) begin
    if(rst)
        regout<=32'd0;
    else if(en)
        regout<=regin;
    end
endmodule

module bigregister #(parameter n=32)(
    input clk,
    input rst,
    input wire [n-1:0] regin,
    input wire writen,
    input flush,
    output logic [n-1:0] regout
);
    always@(posedge clk,posedge rst)begin
    if(rst)
        regout<= 'd0;
    else if(flush)
        regout<= 'd0;
    else if(writen)
        regout<=regin;
    end
endmodule
