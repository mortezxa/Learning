module  RicsV_controller(
    input wire rst,
    input wire [31:0] inst,
    input wire zero,
    output reg alu_src,
    output reg Reg_write,
    output reg [2:0] Imm_src,
    output reg mem_write,
    output reg Result_src,
    output reg [2:0] alu_func,
    output reg [1:0] brench,
    output reg sel_r,
    output reg jmp,
    output reg sel_jalr
);
always@(posedge rst)begin
    alu_src <= 1'd0;
    Reg_write <=1'd0;
    Imm_src <= 3'd0;
    mem_write <= 1'd0;
    Result_src <= 1'd0;
    alu_func <= 3'd0;
    brench <= 2'd0;
    sel_r <= 1'b0;
    jmp <= 1'b0;
    sel_jalr <= 1'b0;
    end
wire [6:0]opcode,func3,func7;
reg [1:0] aluop;
assign opcode = inst[6:0];
assign func3 = inst[14:12];
assign func7 = inst[31:25];
always @(inst)begin
case(opcode)
7'b0110011:begin //R_type add, sub, and, or, slt
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b10000001000000;
    case(func3)
    3'b000:begin
    if(func7==7'b0000000)
        alu_func = 3'b000;
    else if(func7==7'b0100000)
        alu_func = 3'b001;
    end
    3'b111:
        alu_func = 3'b010;
    3'b110:
        alu_func = 3'b011;
    3'b010:
        alu_func = 3'b100;
    endcase
    end
7'b0010011:begin //I_type addi, xori, ori, slti
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b11000000100000;
    case(func3)
    3'b000:
        alu_func = 3'b000;
    3'b100:
        alu_func = 3'b110;
    3'b110:
        alu_func = 3'b011;
    3'b010:
        alu_func = 3'b100;
    endcase
    end
7'b0000011: //lw
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b11000010000000;
7'b1100111: //jalr
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b10011000000100;
7'b0100011: //s_type sw
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b01001100000000;
7'b1100011: begin //b_type
    if(func3 == 3'b000) // beq
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b00010000011001;
    else if(func3==3'b001) //bne
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b00010000010001;
    end
7'b0110111: //lui
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b11100001100000;
7'b1101111: //jal
    {Reg_write,alu_src,Imm_src,mem_write,Result_src,aluop,brench,sel_r,jmp,sel_jalr}=14'b10011000000110;
endcase
if(aluop==2'b00)
    alu_func = 3'b000;
else if(aluop==2'b11)
    alu_func = 3'b101;

end
endmodule