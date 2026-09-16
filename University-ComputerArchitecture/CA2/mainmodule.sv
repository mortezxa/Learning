module RiscV_top(
    input  wire clk,
    input  wire rst,
    input  wire [31:0] order [0:1023],
    output wire [31:0] allreg [0:31],
    output wire [31:0] allmem [0:19]
);

    wire        Reg_write;
    wire        alu_src;
    wire [2:0]  alu_func;
    wire        memWrite;
    wire        Result_src;
    wire        sel_r;
    wire [2:0]  Imm_src;
    wire        sel_jalr;
    wire        jmp;
    wire [1:0]  branch;

    wire        zero;
    wire [31:0] inst;

    RiscVDP datapath(
        .clk(clk),
        .rst(rst),
        .Reg_write(Reg_write),
        .alu_src(alu_src),
        .alu_func(alu_func),
        .memWrite(memWrite),
        .Result_src(Result_src),
        .sel_r(sel_r),
        .Imm_src(Imm_src),
        .sel_jalr(sel_jalr),
        .jmp(jmp),
        .branch(branch),
        .order(order),
        .zero(zero),
        .inst_port(inst),
        .allregisters(allreg),
        .allmem(allmem)
    );


    RicsV_controller controller(
        .inst(inst),
        .zero(zero),
        .rst(rst),
        .alu_src(alu_src),
        .Reg_write(Reg_write),
        .Imm_src(Imm_src),
        .mem_write(memWrite),
        .Result_src(Result_src),
        .alu_func(alu_func),
        .brench(branch),
        .sel_r(sel_r),
        .jmp(jmp),
        .sel_jalr(sel_jalr)
    );

endmodule
