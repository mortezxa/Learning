module RiscVDP(
    input clk,
    input rst,
    input wire Reg_write,
    input wire alu_src,
    input wire [2:0] alu_func,
    input wire memWrite,
    input wire Result_src,
    input wire sel_r,
    input wire [2:0] Imm_src,
    input wire sel_jalr,
    input wire jmp,
    input wire [1:0] branch,
    input wire [31:0] order [0:1023], 
    output wire zero,
    output wire [31:0] inst_port,
    output wire [31:0] allregisters [0:31],
    output wire [31:0] allmem [0:19]
);
wire [31:0] pc,inst,wdata,data1,data2,
    second_alu_input,imm_ex,alu_to_mem,
    read_data_mem,imm_out,result_to_r_mux,
    jalr_mux_to_adder,pc_4_adder,
    jal_adder_to_pc,pc_mux_to_pc;
assign inst_port = inst;
main_reg Mreg1(
    .clk(clk),
    .Reg_write(Reg_write),
    .read_reg1(inst[19:15]),
    .read_reg2(inst[24:20]),
    .write_reg(inst[11:7]),
    .write_data(wdata),
    .read_data1(data1),
    .read_data2(data2),
    .allregisters(allregisters)
);

imm_gen imm_extend(
    .instr(inst),
    .Imm_sr(Imm_src),
    .imm_out(imm_out)
);

inst_mem adr_inst(
    .clk(clk),
    .address(pc),
    .orders(order),
    .inst(inst)
);

data_memory Data_mem(
    .clk(clk),
    .memWrite(memWrite),
    .address(alu_to_mem),
    .writeData(data2),
    .readData(read_data_mem),
    .allmem(allmem)
);

ALU alu(
    .a(data1),
    .b(second_alu_input),
    .s(alu_func),
    .zero(zero),
    .w(alu_to_mem)
);

mux_2to1 mux_befor_alu(
    .a(data2),
    .b(imm_out),
    .sel(alu_src),
    .out(second_alu_input)
);

mux_2to1 result_mex(
    .a(alu_to_mem),
    .b(read_data_mem),
    .sel(Result_src),
    .out(result_to_r_mux)
);

mux_2to1 r_mex(
    .a(result_to_r_mux),
    .b(pc_4_adder),
    .sel(sel_r),
    .out(wdata)
);

mux_2to1 jalr_mux(
    .a({{27'd0},inst[19:15]}),
    .b(pc),
    .sel(sel_jalr),
    .out(jalr_mux_to_adder)
);
wire sel_pc;
assign sel_pc = jmp|((&branch)&zero)|((^branch)&(~zero));
mux_2to1 pc_mux(
    .a(pc_4_adder),
    .b(jal_adder_to_pc),
    .sel(sel_pc),
    .out(pc_mux_to_pc)
);

register pcreg(
    .clk(clk),
    .rst(rst),
    .regin(pc_mux_to_pc),
    .regout(pc)
);

adder pc_adder(
    .a(32'd4),
    .b(pc),
    .y(pc_4_adder)
);

adder jalr_adder(
    .a(imm_out),
    .b(jalr_mux_to_adder),
    .y(jal_adder_to_pc)
);
endmodule