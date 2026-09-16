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
    wire [1:0] ForwardAE;
    wire [1:0] ForwardBE,ResultSrcE;//ResultSrcE
    wire RegwriteM,RegwriteW,RegwriteE,pcmuxsel;//ResultSrcE0
    wire [4:0] Rs1d,Rs2d,Rs1E,Rs2E,RdE,RdM,RdW;
    wire stallF,stallD,FlushD,FlushE;
    RiscVDP datapath(
        .clk(clk),
        .rst(rst),
        .Reg_write(Reg_write),
        .alu_src(alu_src),
        .alu_control(alu_func),
        .memWrite(memWrite),
        .Result_src({sel_r,Result_src}),
        .Imm_src(Imm_src),
        .sel_jalr(sel_jalr),
        .jmp(jmp),
        .branch(branch),
        .order(order),
        .ForwardAE(ForwardAE),
        .ForwardBE(ForwardBE),
        .stallD(stallD),
        .stallF(stallF),
        .FlushD(FlushD),
        .FlushE(FlushE),
        .zero(zero),
        .ResultSrcE(ResultSrcE),
        .Rs1d(Rs1d),
        .Rs2d(Rs2d),
        .Rs1E(Rs1E),
        .Rs2E(Rs2E),
        .RdE(RdE),
        .RdM(RdM),
        .RegwriteM(RegwriteM),
        .RdW(RdW),
        .RegwriteW(RegwriteW),
        .RegWriteE(RegwriteE),
        .pcmuxsel(pcmuxsel),
        .inst_port(inst),
        .allregisters(allreg),
        .allmem(allmem)
    );

    Hazard HazardUnit(
        .rst(rst),
        .Rs1D(Rs1d),
        .Rs2D(Rs2d),
        .Rs1E(Rs1E),
        .Rs2E(Rs2E),
        .RdE(RdE),
        .pcmuxsel(pcmuxsel),
        .ResultSrcE0(ResultSrcE[0]),
        .RdM(RdM),
        .RegwriteM(RegwriteM),
        .RdW(RdW),
        .RegwriteW(RegwriteW),
        .stallF(stallF),
        .stallD(stallD),
        .FlushD(FlushD),
        .FlushE(FlushE),
        .ForwardAE(ForwardAE),
        .ForwardBE(ForwardBE)
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
