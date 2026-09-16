struct packed {
    logic [31:0] Inst;
    logic [9:0] PC;
    logic [9:0] PCPluse4;
} FtoD;
struct packed {
    //controller part
    logic sel_jalr;
    logic Regwrite ;
    logic [1:0]ResultSrc;
    logic Memwrite;
    logic Jump;
    logic [1:0] Branch;
    logic [2:0] ALUcontrol;
    logic ALUSrc;
    //Data path part
    logic [31:0] RD1;
    logic [31:0] RD2;
    logic [9:0] PC;
    logic [31:0]Extimm;
    logic [9:0]PCPluse4;
    logic [4:0]Rs1;
    logic [4:0]Rs2;
    logic [4:0]Rd;
} DtoE;
struct packed {
    //controller part
    logic Regwrite ;
    logic [1:0]ResultSrc;
    logic Memwrite;
    //Data path part
    logic [31:0] ALUResult;
    logic [9:0] WriteData;
    logic [9:0] PCPluse4;
    logic [4:0] Rd;
} EtoM;
struct packed {
    //controller part
    logic Regwrite ;
    logic [1:0]ResultSrc;
    //Data path part
    logic [31:0] ALUResult;
    logic [9:0] ReadData;
    logic [9:0] PCPluse4;
    logic [4:0] Rd;
} MtoW;

bigregister #(.n(52)) FtoDreg(
    .clk(clk),
    .regin(FtoD),
    .regout({InstrD,PCD,PCPluse4D})
);
bigregister #(.n(9'd142)) DtoEreg(
    .clk(clk),
    .regin(DtoE),
    .regout({sel_jalrE,RegwriteE,ResultSrcE,MemwriteE,JumpE,BranchE,ALUcontrolE,ALUSrcE
    RD1E,RD2E,PCE,ExtimmE,PCPluse4E,Rs1E,Rs2E,Rd})
);
bigregister #(.n(9'd61)) EtoMreg(
    .clk(clk),
    .regin(EtoM),
    .regout({RegwriteM,ResultSrcM,MemwriteM,ALUResultM,WriteDataM,PCPluse4M,RdM})
);
bigregister #(.n(9'd60)) MtoWreg(
    .ckl(clk),
    .regin(MtoW),
    .regout({RegwriteW,ResultSrcW,ALUResultW,ReadDataW,PCPluse4W,RdW})
);

