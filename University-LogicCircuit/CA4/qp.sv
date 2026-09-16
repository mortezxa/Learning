// Copyright (C) 2020  Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions 
// and other software and tools, and any partner logic 
// functions, and any output files from any of the foregoing 
// (including device programming or simulation files), and any 
// associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License 
// Subscription Agreement, the Intel Quartus Prime License Agreement,
// the Intel FPGA IP License Agreement, or other applicable license
// agreement, including, without limitation, that your use is for
// the sole purpose of programming logic devices manufactured by
// Intel and sold by Intel or its authorized distributors.  Please
// refer to the applicable agreement for further details, at
// https://fpgasoftware.intel.com/eula.

// VENDOR "Altera"
// PROGRAM "Quartus Prime"
// VERSION "Version 20.1.0 Build 711 06/05/2020 SJ Lite Edition"

// DATE "06/10/2025 06:26:54"

// 
// Device: Altera EP4CGX15BF14A7 Package FBGA169
// 

// 
// This Verilog file should be used for ModelSim-Altera (SystemVerilog) only
// 

`timescale 1 ns/ 1 ps

module CA4_modulep (
	clk,
	rst,
	j,
	w);
input 	reg clk ;
input 	reg rst ;
input 	reg j ;
output 	logic w ;

// Design Ports Information
// w	=>  Location: PIN_N9,	 I/O Standard: 2.5 V,	 Current Strength: Default
// j	=>  Location: PIN_N8,	 I/O Standard: 2.5 V,	 Current Strength: Default
// clk	=>  Location: PIN_J7,	 I/O Standard: 2.5 V,	 Current Strength: Default
// rst	=>  Location: PIN_J6,	 I/O Standard: 2.5 V,	 Current Strength: Default


wire gnd;
wire vcc;
wire unknown;

assign gnd = 1'b0;
assign vcc = 1'b1;
assign unknown = 1'bx;

tri1 devclrn;
tri1 devpor;
tri1 devoe;
// synopsys translate_off
initial $sdf_annotate("CA4_module_v.sdo");
// synopsys translate_on

wire \w~output_o ;
wire \clk~input_o ;
wire \clk~inputclkctrl_outclk ;
wire \j~input_o ;
wire \counter~2_combout ;
wire \rst~input_o ;
wire \rst~inputclkctrl_outclk ;
wire \counter[2]~1_combout ;
wire \counter~0_combout ;
wire \counter~3_combout ;
wire \carry_out~0_combout ;
wire \carry_out~1_combout ;
wire \carry_out~q ;
wire \Selector1~0_combout ;
wire \current_state.S1~q ;
wire \Selector2~0_combout ;
wire \current_state.S2~q ;
wire \next_state.S3~0_combout ;
wire \current_state.S3~q ;
wire [2:0] counter;


hard_block auto_generated_inst(
	.devpor(devpor),
	.devclrn(devclrn),
	.devoe(devoe));

// Location: IOOBUF_X20_Y0_N2
cycloneiv_io_obuf \w~output (
	.i(\current_state.S3~q ),
	.oe(vcc),
	.seriesterminationcontrol(16'b0000000000000000),
	.devoe(devoe),
	.o(\w~output_o ),
	.obar());
// synopsys translate_off
defparam \w~output .bus_hold = "false";
defparam \w~output .open_drain_output = "false";
// synopsys translate_on

// Location: IOIBUF_X16_Y0_N15
cycloneiv_io_ibuf \clk~input (
	.i(clk),
	.ibar(gnd),
	.o(\clk~input_o ));
// synopsys translate_off
defparam \clk~input .bus_hold = "false";
defparam \clk~input .simulate_z_as = "z";
// synopsys translate_on

// Location: CLKCTRL_G17
cycloneiv_clkctrl \clk~inputclkctrl (
	.ena(vcc),
	.inclk({vcc,vcc,vcc,\clk~input_o }),
	.clkselect(2'b00),
	.devclrn(devclrn),
	.devpor(devpor),
	.outclk(\clk~inputclkctrl_outclk ));
// synopsys translate_off
defparam \clk~inputclkctrl .clock_type = "global clock";
defparam \clk~inputclkctrl .ena_register_mode = "none";
// synopsys translate_on

// Location: IOIBUF_X20_Y0_N8
cycloneiv_io_ibuf \j~input (
	.i(j),
	.ibar(gnd),
	.o(\j~input_o ));
// synopsys translate_off
defparam \j~input .bus_hold = "false";
defparam \j~input .simulate_z_as = "z";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N8
cycloneiv_lcell_comb \counter~2 (
// Equation(s):
// \counter~2_combout  = (\current_state.S1~q ) # (!counter[0])

	.dataa(gnd),
	.datab(\current_state.S1~q ),
	.datac(counter[0]),
	.datad(gnd),
	.cin(gnd),
	.combout(\counter~2_combout ),
	.cout());
// synopsys translate_off
defparam \counter~2 .lut_mask = 16'hCFCF;
defparam \counter~2 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: IOIBUF_X16_Y0_N22
cycloneiv_io_ibuf \rst~input (
	.i(rst),
	.ibar(gnd),
	.o(\rst~input_o ));
// synopsys translate_off
defparam \rst~input .bus_hold = "false";
defparam \rst~input .simulate_z_as = "z";
// synopsys translate_on

// Location: CLKCTRL_G19
cycloneiv_clkctrl \rst~inputclkctrl (
	.ena(vcc),
	.inclk({vcc,vcc,vcc,\rst~input_o }),
	.clkselect(2'b00),
	.devclrn(devclrn),
	.devpor(devpor),
	.outclk(\rst~inputclkctrl_outclk ));
// synopsys translate_off
defparam \rst~inputclkctrl .clock_type = "global clock";
defparam \rst~inputclkctrl .ena_register_mode = "none";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N16
cycloneiv_lcell_comb \counter[2]~1 (
// Equation(s):
// \counter[2]~1_combout  = \current_state.S1~q  $ (((!\j~input_o  & (\current_state.S2~q  & !\carry_out~q ))))

	.dataa(\j~input_o ),
	.datab(\current_state.S2~q ),
	.datac(\carry_out~q ),
	.datad(\current_state.S1~q ),
	.cin(gnd),
	.combout(\counter[2]~1_combout ),
	.cout());
// synopsys translate_off
defparam \counter[2]~1 .lut_mask = 16'hFB04;
defparam \counter[2]~1 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N9
dffeas \counter[0] (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\counter~2_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(\counter[2]~1_combout ),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(counter[0]),
	.prn(vcc));
// synopsys translate_off
defparam \counter[0] .is_wysiwyg = "true";
defparam \counter[0] .power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N30
cycloneiv_lcell_comb \counter~0 (
// Equation(s):
// \counter~0_combout  = (\current_state.S1~q ) # (counter[0] $ (counter[1]))

	.dataa(counter[0]),
	.datab(\current_state.S1~q ),
	.datac(counter[1]),
	.datad(gnd),
	.cin(gnd),
	.combout(\counter~0_combout ),
	.cout());
// synopsys translate_off
defparam \counter~0 .lut_mask = 16'hDEDE;
defparam \counter~0 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N31
dffeas \counter[1] (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\counter~0_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(\counter[2]~1_combout ),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(counter[1]),
	.prn(vcc));
// synopsys translate_off
defparam \counter[1] .is_wysiwyg = "true";
defparam \counter[1] .power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N22
cycloneiv_lcell_comb \counter~3 (
// Equation(s):
// \counter~3_combout  = (!\current_state.S1~q  & (counter[2] $ (((counter[1] & counter[0])))))

	.dataa(counter[1]),
	.datab(\current_state.S1~q ),
	.datac(counter[2]),
	.datad(counter[0]),
	.cin(gnd),
	.combout(\counter~3_combout ),
	.cout());
// synopsys translate_off
defparam \counter~3 .lut_mask = 16'h1230;
defparam \counter~3 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N23
dffeas \counter[2] (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\counter~3_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(\counter[2]~1_combout ),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(counter[2]),
	.prn(vcc));
// synopsys translate_off
defparam \counter[2] .is_wysiwyg = "true";
defparam \counter[2] .power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N24
cycloneiv_lcell_comb \carry_out~0 (
// Equation(s):
// \carry_out~0_combout  = (!\carry_out~q  & (!\j~input_o  & (counter[2] & \current_state.S2~q )))

	.dataa(\carry_out~q ),
	.datab(\j~input_o ),
	.datac(counter[2]),
	.datad(\current_state.S2~q ),
	.cin(gnd),
	.combout(\carry_out~0_combout ),
	.cout());
// synopsys translate_off
defparam \carry_out~0 .lut_mask = 16'h1000;
defparam \carry_out~0 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N26
cycloneiv_lcell_comb \carry_out~1 (
// Equation(s):
// \carry_out~1_combout  = (\carry_out~0_combout  & (counter[1] & counter[0]))

	.dataa(gnd),
	.datab(\carry_out~0_combout ),
	.datac(counter[1]),
	.datad(counter[0]),
	.cin(gnd),
	.combout(\carry_out~1_combout ),
	.cout());
// synopsys translate_off
defparam \carry_out~1 .lut_mask = 16'hC000;
defparam \carry_out~1 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N27
dffeas carry_out(
	.clk(\clk~inputclkctrl_outclk ),
	.d(\carry_out~1_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(vcc),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(\carry_out~q ),
	.prn(vcc));
// synopsys translate_off
defparam carry_out.is_wysiwyg = "true";
defparam carry_out.power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N6
cycloneiv_lcell_comb \Selector1~0 (
// Equation(s):
// \Selector1~0_combout  = (\j~input_o  & ((!\carry_out~q ) # (!\current_state.S2~q )))

	.dataa(\current_state.S2~q ),
	.datab(\j~input_o ),
	.datac(\carry_out~q ),
	.datad(gnd),
	.cin(gnd),
	.combout(\Selector1~0_combout ),
	.cout());
// synopsys translate_off
defparam \Selector1~0 .lut_mask = 16'h4C4C;
defparam \Selector1~0 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N7
dffeas \current_state.S1 (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\Selector1~0_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(vcc),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(\current_state.S1~q ),
	.prn(vcc));
// synopsys translate_off
defparam \current_state.S1 .is_wysiwyg = "true";
defparam \current_state.S1 .power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N4
cycloneiv_lcell_comb \Selector2~0 (
// Equation(s):
// \Selector2~0_combout  = (!\j~input_o  & ((\current_state.S1~q ) # ((\current_state.S2~q  & !\carry_out~q ))))

	.dataa(\j~input_o ),
	.datab(\current_state.S1~q ),
	.datac(\current_state.S2~q ),
	.datad(\carry_out~q ),
	.cin(gnd),
	.combout(\Selector2~0_combout ),
	.cout());
// synopsys translate_off
defparam \Selector2~0 .lut_mask = 16'h4454;
defparam \Selector2~0 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N5
dffeas \current_state.S2 (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\Selector2~0_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(vcc),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(\current_state.S2~q ),
	.prn(vcc));
// synopsys translate_off
defparam \current_state.S2 .is_wysiwyg = "true";
defparam \current_state.S2 .power_up = "low";
// synopsys translate_on

// Location: LCCOMB_X20_Y1_N28
cycloneiv_lcell_comb \next_state.S3~0 (
// Equation(s):
// \next_state.S3~0_combout  = (\current_state.S2~q  & (\j~input_o  & \carry_out~q ))

	.dataa(\current_state.S2~q ),
	.datab(\j~input_o ),
	.datac(\carry_out~q ),
	.datad(gnd),
	.cin(gnd),
	.combout(\next_state.S3~0_combout ),
	.cout());
// synopsys translate_off
defparam \next_state.S3~0 .lut_mask = 16'h8080;
defparam \next_state.S3~0 .sum_lutc_input = "datac";
// synopsys translate_on

// Location: FF_X20_Y1_N29
dffeas \current_state.S3 (
	.clk(\clk~inputclkctrl_outclk ),
	.d(\next_state.S3~0_combout ),
	.asdata(vcc),
	.clrn(!\rst~inputclkctrl_outclk ),
	.aload(gnd),
	.sclr(gnd),
	.sload(gnd),
	.ena(vcc),
	.devclrn(devclrn),
	.devpor(devpor),
	.q(\current_state.S3~q ),
	.prn(vcc));
// synopsys translate_off
defparam \current_state.S3 .is_wysiwyg = "true";
defparam \current_state.S3 .power_up = "low";
// synopsys translate_on

assign w = \w~output_o ;

endmodule

module hard_block (

	devpor,
	devclrn,
	devoe);

// Design Ports Information
// ~ALTERA_NCEO~	=>  Location: PIN_N5,	 I/O Standard: 2.5 V,	 Current Strength: 16mA
// ~ALTERA_DATA0~	=>  Location: PIN_A5,	 I/O Standard: 2.5 V,	 Current Strength: Default
// ~ALTERA_ASDO~	=>  Location: PIN_B5,	 I/O Standard: 2.5 V,	 Current Strength: Default
// ~ALTERA_NCSO~	=>  Location: PIN_C5,	 I/O Standard: 2.5 V,	 Current Strength: Default
// ~ALTERA_DCLK~	=>  Location: PIN_A4,	 I/O Standard: 2.5 V,	 Current Strength: Default

input 	devpor;
input 	devclrn;
input 	devoe;

wire gnd;
wire vcc;
wire unknown;

assign gnd = 1'b0;
assign vcc = 1'b1;
assign unknown = 1'bx;

wire \~ALTERA_DATA0~~padout ;
wire \~ALTERA_ASDO~~padout ;
wire \~ALTERA_NCSO~~padout ;
wire \~ALTERA_DATA0~~ibuf_o ;
wire \~ALTERA_ASDO~~ibuf_o ;
wire \~ALTERA_NCSO~~ibuf_o ;


endmodule
