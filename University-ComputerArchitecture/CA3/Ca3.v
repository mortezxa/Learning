module mux_2to1 #(
    parameter WIDTH = 16
)(
    input sel,
    input [WIDTH-1:0] In1,
    input [WIDTH-1:0] In2,
    output [WIDTH-1:0] out
);

    assign out = sel ? In2 : In1;

endmodule


module mux_4to1 #(
    parameter WIDTH = 16
)(
    input [1:0] sel,
    input [WIDTH-1:0] In1,
    input [WIDTH-1:0] In2,
    input [WIDTH-1:0] In3,
    input [WIDTH-1:0] In4,
    output [WIDTH-1:0] out
);

    assign out =
        (sel == 2'b00) ? In1 :
        (sel == 2'b01) ? In2 :
        (sel == 2'b10) ? In3 :
        (sel == 2'b11) ? In4 :
        {WIDTH{1'b0}};

endmodule


module ALU #(
    parameter WIDTH = 16
)(
    input [2:0] s,
    input signed [WIDTH-1:0] In1,
    input signed [WIDTH-1:0] In2,
    output signed [WIDTH-1:0] Out,
    output zero
);

    assign Out =
        (s == 3'b000) ? In1 + In2 :
        (s == 3'b001) ? In1 - In2 :
        (s == 3'b010) ? In1 & In2 :
        (s == 3'b011) ? In1 | In2 :
        (s == 3'b100) ? ~In2 :
        (s == 3'b101) ? In1 :
        (s == 3'b110) ? In2 :
        {WIDTH{1'b0}};

    assign zero = (Out == 0);

endmodule

module memory #(parameter ADDR_WIDTH = 12, DATA_WIDTH = 16, SIZE = (1 << ADDR_WIDTH)
)
(
    input rst,clk,
    input writeEn,
    input [ADDR_WIDTH-1:0] addr,
    input [DATA_WIDTH-1:0] writeData,
    output [DATA_WIDTH-1:0] readData
);

    reg [DATA_WIDTH - 1:0] memory [0:SIZE-1];
    integer i;
    always @(posedge clk or posedge rst) begin
        if (rst) begin
            for (i = 0; i < SIZE; i = i + 1)
                memory[i] <= {DATA_WIDTH{1'b0}};
        end
        else if (writeEn)
            memory[addr] <= writeData;
    end


    assign readData = memory[addr];

endmodule


module imm_gen #(
    parameter WIDTH = 16
)(
    input  wire [WIDTH-5:0] instr,
    input  wire [1:0]  Imm_src,
    output reg  [WIDTH-1:0] imm_out
);

    always @(instr, Imm_src) begin
        case (Imm_src)

            2'b00: // A_type
                imm_out = {4'b0000, instr[11:0]};

            2'b01: // B_type
                imm_out = {7'b0000000, instr[8:0]};

            2'b10: // C_type
                imm_out = {7'b0000000, instr[8:0]};

            2'b11: // D_type
                imm_out = {4'b0000, instr[11:0]};

            default:
                imm_out = 16'd0;

        endcase
    end

endmodule

module main_reg #(
    parameter WIDTH = 16
)(
    input clk, rst, Reg_write,
    input wire [2:0] read_reg1, read_reg2, write_reg,
    input wire [WIDTH-1:0] write_data,
    output wire [WIDTH-1:0] read_data1, read_data2
);

    reg [WIDTH-1:0] mainregfile [0:7];
    integer i;

    assign read_data1 = mainregfile[read_reg1];

    assign read_data2 = mainregfile[read_reg2];

    always @(posedge clk) begin
        if (rst) begin
            for(i = 0; i < 8; i = i + 1)
                mainregfile[i] <= 16'd0;
        end
        else if (Reg_write) begin
            mainregfile[write_reg] <= write_data;
        end
    end
endmodule


module Register #(parameter N = 16) 
(
    input clk, rst,
    input [N-1:0] d,
    output reg [N-1:0] q
);
    always @(posedge clk or posedge rst) begin
        if (rst)
            q <= {N{1'b0}};
        else
            q <= d;
    end
endmodule


module RegisterEn #(parameter N = 16) 
(
    input clk, rst,
    input en,
    input [N-1:0] d,
    output reg [N-1:0] q
);
    always @(posedge clk or posedge rst) begin
        if (rst)
            q <= {N{1'b0}};
        else if (en)
            q <= d;
    end
    
endmodule

module Rick_Mul_Cy(
    input wire rst, clk,
    input wire IR_write, aluop_write, a2_src, a3_src, Adr_src,
    input wire pc_write, mem_write, Reg_write,
    input wire [1:0] alu_srcA, alu_srcB, Result_src, Imm_src,
    input wire [2:0] alu_func,
    output wire zero,
    output wire [15:0] inst,
    output wire [15:0] IR_outt
);

    wire [15:0] pc_out, adr_out, inst_out, MDR_out, imm_out, data_out1, data_out2,
    R0_out,R1_out, Amux2, Amux_out, Bmux_out, ALU_out, ALureg_out, result_out, IR_out;
    wire [2:0] data2, data3, data3b, controller3b;
    wire [11:0] IR_out12b;

    assign IR_out12b = IR_out[11:0];
    assign data3b = IR_out[11:9];
    assign IR_outt = IR_out;
    
    mux_2to1 Adr_mux(
        .sel(Adr_src),
        .In1(pc_out),
        .In2(result_out),
        .out(adr_out)
    );

    
    // 3-bit register index selector for read port 2
    mux_2to1 #(.WIDTH(3)) data2_mux(
        .sel(a2_src),
        .In1(3'b000),
        .In2(data3b),
        .out(data2)
    );

    
    // 3-bit register index selector for write port
    mux_2to1 #(.WIDTH(3)) data3_mux(
        .sel(a3_src),
        .In1(3'b000),
        .In2(data3b),
        .out(data3)
    );


    mux_4to1 A_mux(
        .sel(alu_srcA),
        .In1(pc_out),
        .In2(Amux2),
        .In3(R0_out),
        .In4(16'd0),
        .out(Amux_out)
    );


    mux_4to1 B_mux(
        .sel(alu_srcB),
        .In1(R1_out),
        .In2(inst_out),
        .In3(16'd2),
        .In4(MDR_out),
        .out(Bmux_out)
    );


    mux_4to1 Result_mux(
        .sel(Result_src),
        .In1(ALureg_out),
        .In2(ALU_out),
        .In3(MDR_out),
        .In4(16'd0),
        .out(result_out)
    );


    ALU alu(
        .s(alu_func),
        .In1(Bmux_out),
        .In2(Amux_out),
        .Out(ALU_out),
        .zero(zero)
    );


    memory Mem(
        .rst(rst),
        .clk(clk),
        .writeEn(mem_write),
        .addr(adr_out[12:1]),
        .writeData(R1_out),
        .readData(inst)
    );


    imm_gen gen_ex(
        .instr(IR_out12b),
        .Imm_src(Imm_src),
        .imm_out(inst_out)
    );


    main_reg reg_file(
        .clk(clk),
        .rst(rst),
        .Reg_write(Reg_write),
        .read_reg1(3'b000),
        .read_reg2(data2),
        .write_reg(data3),
        .write_data(result_out),
        .read_data1(data_out1),
        .read_data2(data_out2)
    );


    RegisterEn pc_reg(
        .clk(clk),
        .rst(rst),
        .en(pc_write),
        .d(result_out),
        .q(pc_out)
    );


    Register pc_save_reg(
        .clk(clk),
        .rst(rst),
        .d(pc_out),
        .q(Amux2)
    );


    RegisterEn IR_reg(
        .clk(clk),
        .rst(rst),
        .en(IR_write),
        .d(inst),
        .q(IR_out)
    );


    Register MDR_reg(
        .clk(clk),
        .rst(rst),
        .d(inst),
        .q(MDR_out)
    );


    Register regA(
        .clk(clk),
        .rst(rst),
        .d(data_out1),
        .q(R0_out)
    );


    Register regB(
        .clk(clk),
        .rst(rst),
        .d(data_out2),
        .q(R1_out)
    );


    RegisterEn ALU_reg(
        .clk(clk),
        .rst(rst),
        .en(aluop_write),
        .d(ALU_out),
        .q(ALureg_out)
    );

endmodule


`define START_state 5'b00000
`define LOAD_state_1 5'b00001
`define LOAD_state_2 5'b00010
`define LOAD_state_3 5'b10010
`define STORE_state_1 5'b00011
`define STORE_state_2 5'b00100
`define JUMP_state 5'b00110
`define BRANCHZ_state_1 5'b00111
`define BRANCHZ_state_2 5'b01000
`define BRANCHZ_state_3 5'b01001
`define MOVE_TO_state 5'b01010
`define C_TYPE_state_1 5'b01011
`define C_TYPE_state_2 5'b01100
`define C_TYPE_state_3 5'b10000
`define D_TYPE_state_1 5'b01101
`define D_TYPE_state_2 5'b01110
`define D_TYPE_state_3 5'b10001
`define RST_state 5'b01111


module RicsV_controller(
    input  wire rst, clk,
    input  wire zero,
    input  wire [15:0] inst,
    input  wire [15:0] IR_out,
    output reg [2:0] alu_func,
    output reg [1:0] alu_srcA,
    output reg [1:0] alu_srcB,
    output reg [1:0] Result_src,
    output reg [1:0] Imm_src,
    output reg Reg_write,
    output reg mem_write,
    output reg pc_write,
    output reg Adr_src,
    output reg a2_src,
    output reg a3_src,
    output reg aluop_write,
    output reg IR_write,
    output wire [4:0] state_out
);

    reg [4:0] ps= 5'b01111, ns;
    wire [3:0] opcode;
    wire [8:0] func;

    assign opcode = inst[15:12];
    assign func = IR_out[8:0];
    assign state_out = ps;

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            ps <= `RST_state;
        end
        else begin
            ps <= ns;
        end
    end


    always @(*)begin

		case(ps)			
            `START_state : begin
                ns =
                    (opcode == 4'b0000) ? `LOAD_state_1 :
                    (opcode == 4'b0001) ? `STORE_state_1 :
                    (opcode == 4'b0010) ? `JUMP_state :
                    (opcode == 4'b0100) ? `BRANCHZ_state_1 :
                    (opcode == 4'b1000) ? `C_TYPE_state_1 : `D_TYPE_state_1;
            end
            `LOAD_state_1 : ns = `LOAD_state_2;
            `LOAD_state_2 : ns = `LOAD_state_3;
            `LOAD_state_3 : ns = `START_state;
            `STORE_state_1 : ns = `STORE_state_2;
            `STORE_state_2 : ns = `START_state;
            `JUMP_state : ns = `START_state;
            `BRANCHZ_state_1 : ns = `BRANCHZ_state_2;
            `BRANCHZ_state_2 : ns = `BRANCHZ_state_3;
            `BRANCHZ_state_3 : ns = `START_state;
            `C_TYPE_state_1 : ns = `C_TYPE_state_2;
            `C_TYPE_state_2 : ns = `C_TYPE_state_3;
            `C_TYPE_state_3 : ns = `START_state;
            `D_TYPE_state_1 : ns = `D_TYPE_state_2;
            `D_TYPE_state_2 : ns = `D_TYPE_state_3;
            `D_TYPE_state_3 : ns = `START_state;
            `RST_state : ns = (rst) ? `RST_state : `START_state;
            
		endcase
	end
    always @(posedge clk, zero)begin
        {alu_func, alu_srcA, alu_srcB, Result_src, Imm_src, Reg_write, mem_write, pc_write, Adr_src, a2_src, a3_src, IR_write, aluop_write} = 19'd0;
		
        case (ps)
			`START_state : begin
                // Fetch: PC -> memory, IR load, PC = PC + 2
                {Adr_src, mem_write, IR_write, alu_srcA, alu_srcB, alu_func, Result_src, pc_write} = 13'b0_0_1_00_10_000_01_1;
            end
            `LOAD_state_1 : begin
                {a3_src, alu_srcB, alu_func, Result_src, Adr_src, mem_write, Imm_src} = 12'b0_01_101_01_1_0_00;
            end
            `LOAD_state_2 : begin
                {Result_src, Reg_write} = 3'b10_1;
            end
            `STORE_state_1 : begin
                {a2_src, Imm_src} = 3'b0_00;
            end
            `STORE_state_2 : begin
                {alu_srcB, alu_func, Result_src, Adr_src, mem_write} = 9'b01_101_01_1_1;
            end
            `JUMP_state : begin
                {Result_src, pc_write, Imm_src} = 5'b10_1_00;
            end
            `BRANCHZ_state_1 : begin
                {a2_src, alu_srcB, alu_func, aluop_write, Imm_src} = 9'b1_01_101_1_01;
            end
            `BRANCHZ_state_2 : begin
                {alu_srcA, alu_srcB} = 4'b10_00;
            end
            `BRANCHZ_state_3 : begin
                Result_src = 2'b00;
                Adr_src = (zero) ? 1'b1 : 1'b0;
            end
            `C_TYPE_state_1 : begin
                {a2_src, Imm_src} = 3'b1_10;
            end
            `C_TYPE_state_2 : begin
                if (func == 9'b000000001)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b1_10_00_110_01_1;
                else if(func == 9'b000000010)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_101_01_1;
                else if(func == 9'b000000100)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_000_01_1;
                else if(func == 9'b000001000)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_001_01_1;
                else if(func == 9'b000010000)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_010_01_1;
                else if(func == 9'b000100000)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_011_01_1;
                else if(func == 9'b001000000)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_100_01_1;
                else if(func == 9'b010000000)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_00_000_01_0;
            end
            `D_TYPE_state_1 : begin
                {alu_srcB, alu_func, Result_src, Adr_src, Imm_src} = 10'b01_101_01_1_11;
            end
            `D_TYPE_state_2 : begin
                if(opcode == 4'b1100)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_11_000_01_1;
                else if(opcode == 4'b1101)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_11_001_01_1;
                else if(opcode == 4'b1110)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_11_010_01_1;
                else if(opcode == 4'b1111)
                    {a3_src, alu_srcA, alu_srcB, alu_func, Result_src, Reg_write} = 11'b0_10_11_011_01_1;
            end
		endcase
    end
endmodule

module RiscV_top(
    input rst, clk,
    output wire [4:0] state_out
);
    wire IR_write, aluop_write, a2_src, a3_src, Adr_src, pc_write, mem_write, Reg_write, zero;
    wire [1:0] alu_srcA, alu_srcB, Result_src, Imm_src;
    wire [2:0] alu_func;
    wire [15:0] inst;
    wire [15:0] IR_out;
    
    Rick_Mul_Cy datapath(
        .rst(rst),
        .clk(clk),
        .IR_write(IR_write),
        .aluop_write(aluop_write),
        .a2_src(a2_src),
        .a3_src(a3_src),
        .Adr_src(Adr_src),
        .pc_write(pc_write),
        .mem_write(mem_write),
        .Reg_write(Reg_write),
        .alu_srcA(alu_srcA),
        .alu_srcB(alu_srcB),
        .Result_src(Result_src),
        .Imm_src(Imm_src),
        .alu_func(alu_func),
        .zero(zero),
        .inst(inst),
        .IR_outt(IR_out)
    );

    RicsV_controller controller(
        .rst(rst),
        .clk(clk),
        .zero(zero),
        .inst(inst),
        .IR_out(IR_out),
        .alu_func(alu_func),
        .alu_srcA(alu_srcA),
        .alu_srcB(alu_srcB),
        .Result_src(Result_src),
        .Imm_src(Imm_src),
        .Reg_write(Reg_write),
        .mem_write(mem_write),
        .pc_write(pc_write),
        .Adr_src(Adr_src),
        .a2_src(a2_src),
        .a3_src(a3_src),
        .aluop_write(aluop_write),
        .IR_write(IR_write),
        .state_out(state_out)
    );

endmodule


`timescale 1ns/1ns

module RiscVTB;
    reg clk = 0;
    reg rst = 1;
    wire [4:0] state_out;
    reg [15:0] rtemp;
    integer i;

    RiscV_top dut (
        .rst(rst),
        .clk(clk),
        .state_out(state_out)
    );

    always #5 clk = ~clk;

    initial begin
        // $display("time\ts");
        // $monitor("%0t\t%b  %b  %b  %b  %b", $time,
        // state_out, dut.datapath.Mem.memory[1034], dut.datapath.reg_file.mainregfile[0], dut.datapath.reg_file.mainregfile[1], dut.datapath.IR_out);

    rst = 1;
    #10;
    rst = 0;

        // preload memory
        for (i = 0; i < (1 << 12); i = i + 1)
            dut.datapath.Mem.memory[i] = 16'd0;

        dut.datapath.Mem.memory[0] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[1] = 16'b0000_1000_0000_0000;
        dut.datapath.Mem.memory[2] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[3] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[4] = 16'b0000_100000000010;
        dut.datapath.Mem.memory[5] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[6] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[7] = 16'b0000_100000000100;
        dut.datapath.Mem.memory[8] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[9] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[10] = 16'b0000_100000000110;
        dut.datapath.Mem.memory[11] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[12] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[13] = 16'b0000_100000001000;
        dut.datapath.Mem.memory[14] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[15] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[16] = 16'b0000_100000001010;
        dut.datapath.Mem.memory[17] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[18] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[19] = 16'b0000_100000001100;
        dut.datapath.Mem.memory[20] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[21] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[22] = 16'b0000_100000001110;
        dut.datapath.Mem.memory[23] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[24] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[25] = 16'b0000_100000010000;
        dut.datapath.Mem.memory[26] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[27] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[28] = 16'b0000_100000010010;
        dut.datapath.Mem.memory[29] = 16'b1000_001_000000100;
        
        dut.datapath.Mem.memory[30] = 16'b1000_001_000000001;
        dut.datapath.Mem.memory[31] = 16'b0001_100000010100;



        dut.datapath.Mem.memory[1024] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1025] = 16'b0000_0000_1000_0001;
        dut.datapath.Mem.memory[1026] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1027] = 16'b0000_0100_0000_0001;
        dut.datapath.Mem.memory[1028] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1029] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1030] = 16'b0000_0000_0000_0011;
        dut.datapath.Mem.memory[1031] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1032] = 16'b0000_0000_0000_0001;
        dut.datapath.Mem.memory[1033] = 16'b1111_1111_1111_1111;
        
        
        
        #2000;
        $stop;
        // $finish;
    end

endmodule

