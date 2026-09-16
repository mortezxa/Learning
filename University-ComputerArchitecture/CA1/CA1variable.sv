`define idle_state 4'b0000
`define init_state 4'b0001
`define load_state 4'b0010
`define sh_Vr1_state 4'b0011
`define sh_Vr2_state 4'b0100
`define sh_rstp_state 4'b0101
`define sh_temp_state 4'b0110
`define sum1_state 4'b0111
`define sum2_state 4'b1000
`define sum_i_state 4'b1001
`define sum3_state 4'b1010
`define comp_state 4'b1011
`define prov1_state 4'b1100
`define prove2_state 4'b1101
`define waiting_state 4'b1110
`define sum_i2_state 4'b1111


module ALU (input signed[11:0] A,B,input[1:0]s,output signed[11:0] w);
assign w= (s==2'b00)?A+B:
	  (s==2'b01)?A-B:
	  (s==2'b10)?({A[11],A[11:1]}):
	  (s==2'b11)?((A > B) ? 12'd1 : 12'd0):12'd0;
endmodule

module register #(parameter n=11) (input clk,rst,loaden,inputen,input[n:0]load,regin,output logic[n:0]regout);
always @(posedge clk,posedge rst)begin
if(rst)	regout<={(n+1){1'b0}};
else if(loaden)	regout<=load;
else if(inputen)regout<=regin;
else regout<=regout;
	end
endmodule

module MUX4_to_1(input[11:0]a,b,c,d,input[1:0]s,output[11:0] w);
assign w=(s==2'b00)?a:
	 (s==2'b01)?b:
	 (s==2'b10)?c:
	 (s==2'b11)?d:12'd0;
endmodule
	

module MUX2_to_1(input[11:0]a,b,input s,output[11:0] w);
assign w=(s==2'b00)?a:
	 (s==2'b01)?b:12'd0;
endmodule


module DMUX1_to_4(input[11:0]a,input[1:0]s,output logic[11:0]w0,w1,w2);
 always@(*)begin
	case (s)
   	 2'b00: w0 = a;
   	 2'b01: w1 = a;
   	 2'b10: w2 = a;
	endcase
end
endmodule 

module counter(input clk,rst,iz0,counten,output logic[2:0] number,output logic Co);
always@(posedge clk,posedge rst)begin
	if(rst){Co,number}<= 4'd0;
	if(iz0){Co,number}<= 4'd0;
	else if(counten) number<=number+1;
	Co <= &number;
end
endmodule

module shiftreg(input clk,rst,loaden,shen,input [7:0]pre_w,input[7:0]load,output logic[7:0] post_w,output w);
always@(posedge clk,posedge rst)begin
	if(rst) post_w<=8'd0;
	else if(loaden) post_w<=load;
	else if(shen) post_w<={1'd0,pre_w[7:1]};
end
assign w = post_w[0];
endmodule


module ROM(input logic clk,input logic loadenable,input logic[2:0]addr,output logic[11:0]data);
    logic [11:0] mem [7:0];
    always_ff @(posedge clk) begin
	if (loadenable) 
            $readmemb("Weights.mif", mem);  
 	if (!loadenable) 
            data<=mem[addr];  
    end

    always_ff @(posedge clk) begin
        if (!loadenable) begin
            data<=mem[addr];  
        end
    end
endmodule



module LIF_Neuron_DP (sel1,sel2,sel3,sel4,ld_shreg,shen,counten,ini_rstp,iz0_temp,iz0_count,ld_ROM,rst,clk,input_spikes,
start,ld_th,Vth,ld_rst,Vrest,spike_out,valid,co,aluO,VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen);
	input clk,rst;
	input start,ld_th,ld_rst;
	input [7:0]input_spikes;
	input [11:0]Vth,Vrest;
	input ld_shreg,shen,counten,ini_rstp,iz0_temp,iz0_count,ld_ROM;
	input VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen;
	input [1:0]sel1,sel2,sel3,sel4;	
	
	output valid;
	output spike_out,co;
	output signed [11:0] aluO;
	
	wire signed [11:0]A,B;
	wire [11:0]s40,s41,s42;
	wire [11:0]s30,s32;//s31=s20
	wire [11:0]s20,s21,s22,s23;
	wire [11:0]s50,s51;
	wire [2:0]cout;
	wire sel5;
	wire [11:0]ROMO;
	wire [7:0]shregout;
	
	shiftreg shrg(.clk(clk),.rst(rst),.loaden(ld_shreg),.shen(shen),.pre_w(shregout),.load(input_spikes),.post_w(shregout),.w(sel5));
	counter cnt(.clk(clk),.rst(rst),.iz0(1'd0),.counten(counten),.number(cout),.Co(co));
	ALU alu1(.A(A),.B(B),.s(sel1),.w(aluO));
	ROM rom1(.clk(clk),.loadenable(ld_ROM),.addr(cout),.data(ROMO));

	register V_rest(.clk(clk),.rst(rst),.inputen(VrestInputen),.loaden(ld_rst),.load(Vrest),.regout(s22)),
	a_V_rest(.clk(clk),.rst(rst),.inputen(aVrestInputen),.regin(s42),.regout(s20)),
	V_th(.clk(clk),.rst(rst),.inputen(VthInputen),.loaden(ld_th),.load(Vth),.regout(s32)),
	rstp(.clk(clk),.rst(rst),.inputen(rstpInputen),.loaden(ini_rstp),.load(Vrest),.regin(s41),.regout(s21)),
	temp(.clk(clk),.rst(rst),.inputen(tempInputen),.loaden(iz0_temp),.load(12'd0),.regin(s40),.regout(s30));
	
	MUX4_to_1 s2(.a(s20),.b(s21),.c(s22),.d(s23),.s(sel2),.w(A));
	MUX4_to_1 s3(.a(s30),.b(s20),.c(s32),.s(sel3),.w(B));
	MUX2_to_1 s5(.a(12'd0),.b(ROMO),.s(sel5),.w(s23));
	DMUX1_to_4 s4(.a(aluO),.s(sel4),.w0(s40),.w1(s41),.w2(s42));

endmodule

module controller(clk,rst,co,aluO,start,s,ld_rom,shen,ld_shreg,ini_rstp,
iz0_temp,iz0_count,counten,valid,VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen,sel2,sel1,sel3,sel4);
	
	input clk,co,start,rst;
	input signed [11:0]aluO;
	output logic s,ld_rom,shen,ld_shreg,ini_rstp,iz0_temp,iz0_count,counten,valid;
	output logic VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen;
	output logic[1:0] sel2,sel1,sel3,sel4;

	logic [3:0]ps= 4'd0,ns;
	always @(posedge clk,posedge rst)begin
		if(rst)
			ps<=`idle_state;
		else 
			ps <= ns;
	end
	always @(ps,start,co,aluO)begin
		case(ps)
			`idle_state:ns=start?`init_state:`idle_state;
			`init_state:ns=start?`init_state:`load_state;
			`load_state:ns=`sh_Vr1_state;
			`sh_Vr1_state:ns=`sh_Vr2_state;
			`sh_Vr2_state:ns=`sh_rstp_state;
			`sh_rstp_state:ns=`sh_temp_state;
			`sh_temp_state:ns=`sum1_state;
			`sum1_state:ns=`sum2_state;
			`sum2_state:ns=`sum_i_state;
			`sum_i_state:ns=co?`sum3_state:`sum_i2_state;
			`sum_i2_state:ns=`sum_i_state;
			`sum3_state:ns=`comp_state;
			`comp_state:ns=aluO?`prove2_state:`prov1_state;
			`prov1_state:ns=start?`waiting_state:`prov1_state;
			`prove2_state:ns=start?`waiting_state:`prove2_state;
			`waiting_state:ns=start?`waiting_state:`load_state;
		endcase
	end
	
	always @(ps)begin
		{ld_rom,shen,ld_shreg,ini_rstp,iz0_temp,iz0_count,counten,valid,sel2,sel1,sel3,sel4,VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen}=22'd0;
		case (ps)
			`idle_state:;
			`init_state:ini_rstp=1'b1;
			`load_state:{ld_shreg,ld_rom,iz0_count,iz0_temp}=4'b1111;
			`sh_Vr1_state:{sel1,sel2,sel4,aVrestInputen}=7'b1010101;
			`sh_Vr2_state:{sel1,sel2,sel4,aVrestInputen}=7'b1000101;
			`sh_rstp_state:{sel1,sel2,sel4,rstpInputen}=7'b1001011;
			`sh_temp_state:{sel1,sel2,sel4,tempInputen}=7'b1001001;
			`sum1_state:{sel1,sel2,sel3,sel4,rstpInputen}=9'b000100011;
			`sum2_state:{sel1,sel2,sel3,sel4,iz0_temp,rstpInputen}=10'b0001010111;
			`sum_i_state:{sel1,sel2,sel3,sel4,tempInputen}=9'b001100001;
			`sum_i2_state:{counten,shen}=2'b11;
			`sum3_state:{sel1,sel2,sel3,sel4,rstpInputen}=9'b000100011;
			`comp_state:{sel1,sel2,sel3,iz0_temp,iz0_count}=8'b11011011;
			`prov1_state:{s,valid}=2'b01;
			`prove2_state:{s,valid,ini_rstp}=3'b111;
			`waiting_state:valid=1'b0;
			default ps=`idle_state;
		endcase	
	end
	
endmodule
	
module LIF_Neuron_vari(clk,rst,input_spikes,start,Vth_en,Vth,Vrest_en,Vrest,spike_out,valid);
	input clk,rst;
	input start,Vth_en,Vrest_en;
	input [7:0]input_spikes;
	input signed [11:0]Vth,Vrest;
	output valid;
	output spike_out;
	
	wire co;
	wire signed [11:0]aluO;
	wire [1:0]sel1,sel2,sel3,sel4;
	wire ld_shreg,shen,counten,ini_rstp,iz0_temp,iz0_count,ld_ROM;
	wire VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen;

	LIF_Neuron_DP u1(sel1,sel2,sel3,sel4,ld_shreg,shen,counten,ini_rstp,iz0_temp,iz0_count,ld_ROM,rst,clk,input_spikes,
start,Vth_en,Vth,Vrest_en,Vrest,spike_out,valid,co,aluO,VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen);
	controller u2(clk,rst,co,aluO,start,spike_out,ld_ROM,shen,ld_shreg,ini_rstp,
iz0_temp,iz0_count,counten,valid,VrestInputen,aVrestInputen,VthInputen,tempInputen,rstpInputen,sel2,sel1,sel3,sel4);	

endmodule	