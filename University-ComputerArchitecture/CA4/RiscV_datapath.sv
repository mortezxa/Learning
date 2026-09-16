module RiscVDP(
    input clk,
    input rst,
    input wire Reg_write,
    input wire alu_src,
    input wire [2:0] alu_control,
    input wire memWrite,
    input wire [1:0] Result_src,
    input wire [2:0] Imm_src,
    input wire sel_jalr,
    input wire jmp,
    input wire [1:0] branch,
    input wire [31:0] order [0:1023], 
    input wire [1:0] ForwardAE,
    input wire [1:0] ForwardBE,
    input wire stallD,
    input wire stallF,
    input wire FlushD,
    input wire FlushE,
    output logic zero,
    output logic [1:0] ResultSrcE,
    output logic [4:0] Rs1d,
    output logic [4:0] Rs2d,
    output logic [4:0] Rs1E,
    output logic [4:0] Rs2E,
    output logic [4:0] RdE,
    output logic [4:0] RdM,
    output logic RegwriteM,
    output logic [4:0] RdW,
    output logic RegwriteW,
    output logic RegWriteE,
    output logic pcmuxsel,
    output logic [31:0] inst_port,
    output logic [31:0] allregisters [0:31],
    output logic [31:0] allmem [0:19]
);
wire [31:0] instD,wdata,data1,data2,
    second_alu_input,imm_ex,alu_to_mem,
    read_data_mem,imm_out,ResultW,
    jalr_mux_to_adder,pc_4_adder,
    jal_adder_to_pc,pc_mux_to_pc,
    PC,PCPluse4F,inst;
wire [31:0] RD1E,RD2E,Rd1,Rd2;
wire [31:0] WriteDataM,ALUResultE,ALUResultM,ReadDataW,ReadDataM
            ,PCD,PCE,A1input,ExtimmE,ALUResultW;
wire [31:0] PCPluse4E,PCPluse4D,PCPluse4W,PCPluse4M;
wire sel_jalrE,MemwriteM,MemwriteE,JumpE,ALUSrcE;//RegWriteE
wire [1:0] BranchE,ResultSrcW,ResultSrcM;//ResultSrcE
wire [2:0] ALUcontrolE;

struct packed {
    logic [31:0] Inst;
    logic [31:0] PC;
    logic [31:0] PCPluse4;
} FtoD;
assign FtoD.PC = PC;
assign FtoD.PCPluse4 = PCPluse4F;
assign FtoD.Inst = inst;
struct packed {
    //controller part
    logic sel_jalr;
    logic Regwrite;
    logic [1:0]ResultSrc;
    logic Memwrite;
    logic Jump;
    logic [1:0] Branch;
    logic [2:0] ALUcontrol;
    logic ALUSrc;
    //Data path part
    logic [31:0] RD1;
    logic [31:0] RD2;
    logic [31:0] A1input;
    logic [31:0] PC;
    logic [31:0]Extimm;
    logic [31:0]PCPluse4;
    logic [4:0]Rs1;
    logic [4:0]Rs2;
    logic [4:0]Rd ;
} DtoE;

assign DtoE.sel_jalr = sel_jalr;
assign DtoE.Regwrite= Reg_write;
assign DtoE.ResultSrc= Result_src;
assign DtoE.Memwrite = memWrite;
assign DtoE.Jump = jmp;
assign DtoE.Branch = branch;
assign DtoE.ALUcontrol = alu_control;
assign DtoE.ALUSrc = alu_src;
assign DtoE.A1input = {{27'd0},instD[19:15]};
assign DtoE.PC = PCD;
assign DtoE.PCPluse4 = PCPluse4D;
assign DtoE.Rs1 = instD[19:15];
assign DtoE.Rs2 = instD[24:20];
assign DtoE.Rd = instD[11:7];
assign Rs1d = instD[19:15];
assign Rs2d = instD[24:20];

struct packed {
    //controller part
    logic Regwrite;
    logic [1:0]ResultSrc ;
    logic Memwrite;
    //Data path part
    logic [31:0] ALUResult;
    logic [31:0] WriteData;
    logic [31:0] PCPluse4;
    logic [4:0] Rd;
} EtoM;

assign EtoM.Rd = RdE;

struct packed {
    //controller part
    logic Regwrite;
    logic [1:0]ResultSrc;
    //Data path part
    logic [31:0] ALUResult;
    logic [31:0] ReadData;
    logic [31:0] PCPluse4;
    logic [4:0] Rd;
} MtoW;

assign  MtoW.Regwrite = RegwriteM;
assign inst_port = instD;
assign MtoW.Rd = RdM;

bigregister #(.n(96)) FtoDreg(
    .clk(clk),
    .rst(rst),
    .regin(FtoD),
    .writen(~stallD),
    .flush(FlushD),
    .regout({instD,PCD,PCPluse4D})
);
bigregister #(.n(219)) DtoEreg(
    .clk(clk),
    .rst(rst),
    .regin(DtoE),
    .writen(1'b1),
    .flush(FlushE),
    .regout({sel_jalrE,RegWriteE,ResultSrcE,MemwriteE,JumpE,BranchE,ALUcontrolE,ALUSrcE,
    RD1E,RD2E,A1input,PCE,ExtimmE,PCPluse4E,Rs1E,Rs2E,RdE})
);
assign EtoM.Regwrite=RegWriteE;
assign EtoM.Memwrite=MemwriteE;
assign EtoM.PCPluse4 = PCPluse4E;
assign EtoM.ResultSrc = ResultSrcE;
//assign ResultSrcE = ResultSrcE;
bigregister #(.n(105)) EtoMreg(
    .clk(clk),
    .rst(rst),
    .regin(EtoM),
    .writen(1'b1),
    .flush(1'b0),
    .regout({RegwriteM,ResultSrcM,MemwriteM,ALUResultM,WriteDataM,PCPluse4M,RdM})
);
assign MtoW.ResultSrc = ResultSrcM;
assign MtoW.PCPluse4 = PCPluse4M;
bigregister #(.n(104)) MtoWreg(
    .clk(clk),
    .rst(rst),
    .regin(MtoW),
    .writen(1'b1),
    .flush(1'b0),
    .regout({RegwriteW,ResultSrcW,ALUResultW,ReadDataW,PCPluse4W,RdW})
);

main_reg Mreg1(
    .clk(~clk),
    .Reg_write(RegwriteW),
    .read_reg1(instD[19:15]),
    .read_reg2(instD[24:20]),
    .write_reg(RdW),
    .write_data(ResultW),
    .read_data1(Rd1),
    .read_data2(Rd2),
    .allregisters(allregisters)
);
assign DtoE.RD1= Rd1;
assign DtoE.RD2 = Rd2;
imm_gen imm_extend(
    .instr(instD),
    .Imm_sr(Imm_src),
    .imm_out(imm_ex)
);
assign DtoE.Extimm = imm_ex;
inst_mem adr_inst(
    .clk(clk),
    .address(PC),
    .orders(order),
    .inst(inst)
);

data_memory Data_mem(
    .clk(clk),
    .rst(rst),
    .memWrite(MemwriteM),
    .address(ALUResultM),
    .writeData(WriteDataM),
    .readData(ReadDataM),
    .allmem(allmem)
);
assign MtoW.ReadData = ReadDataM;
assign MtoW.ALUResult = ALUResultM;
mux_3to1 mux_a_alu(
    .a(RD1E),
    .b(ResultW),
    .c(ALUResultM),
    .sel(ForwardAE),
    .out(data1)
);

mux_3to1 mux_b_alu(
    .a(RD2E),
    .b(ResultW),
    .c(ALUResultM),
    .sel(ForwardBE),
    .out(data2)
);
assign EtoM.WriteData= data2;
ALU alu(
    .a(data1),
    .b(second_alu_input),
    .s(ALUcontrolE),
    .zero(zero),
    .w(ALUResultE)
);
assign EtoM.ALUResult = ALUResultE;
mux_2to1 mux_befor_alu(
    .a(data2),
    .b(ExtimmE),
    .sel(ALUSrcE),
    .out(second_alu_input)
);

mux_2to1 result_mex(
    .a(ALUResultW),
    .b(ReadDataW),
    .sel(ResultSrcW[0]),
    .out(wdata)
);

mux_2to1 r_mex(
    .a(wdata),
    .b(PCPluse4W),
    .sel(ResultSrcW[1]),
    .out(ResultW)
);

mux_2to1 jalr_mux(
    .a(A1input),
    .b(PCE),
    .sel(sel_jalrE),
    .out(jalr_mux_to_adder)
);
wire sel_pc;
assign sel_pc = JumpE|((&BranchE)&zero)|((^BranchE)&(~zero));
assign pcmuxsel = sel_pc;
mux_2to1 pc_mux(
    .a(PCPluse4F),
    .b(jal_adder_to_pc),
    .sel(sel_pc),
    .out(pc_mux_to_pc)
);

register pcreg(
    .clk(clk),
    .rst(rst),
    .en(~stallF),
    .regin(pc_mux_to_pc),
    .regout(PC)
);

adder pc_adder(
    .a(32'd4),
    .b(PC),
    .y(PCPluse4F)
);

adder jalr_adder(
    .a(ExtimmE),
    .b(jalr_mux_to_adder),
    .y(jal_adder_to_pc)
);
endmodule