`timescale 1ns / 1ps
/**
	@file md5cracker.v
	@author Andrew D. Zonenberg
	@brief Top-level module for PoC cracker
	
 */
module md5cracker(targetA, targetB, targetC, targetD, clk, hit);

	 input wire [31:0] targetA;
	 input wire [31:0] targetB;
	 input wire [31:0] targetC;
	 input wire [31:0] targetD;
    input wire clk;
    output wire hit;	

endmodule
