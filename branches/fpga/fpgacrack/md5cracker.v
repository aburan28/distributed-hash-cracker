`timescale 1ns / 1ps
/**
	@file md5cracker.v
	@author Andrew D. Zonenberg
	@brief Top-level module for PoC cracker
	
 */
module md5cracker(/*targetA, targetB, targetC, targetD,*/ clk, hit, done, reset, charset, guesslen
	//debug stuff
	/*hashA, hashB, hashC, hashD*/
	);

	/*
	input wire [31:0] targetA;
	input wire [31:0] targetB;
	input wire [31:0] targetC;
	input wire [31:0] targetD;
	*/
   input wire clk;
   output wire hit;
	output wire done;
	
	//Debug
	wire[31:0] hashA;
	wire[31:0] hashB;
	wire[31:0] hashC;
	wire[31:0] hashD;
	wire[127:0] guess;
	
	//Inputs for guess generation
	input wire reset;
	input wire[2:0] charset;
	input wire[3:0] guesslen;

	//The guess generator
	GuessGenerator generator (
		 .clk(clk),
		 .charset(charset), 
		 .guesslen(guesslen), 
		 .reset(reset), 
		 .guess(guess),
		 .done(done)
		 );
		 
	//Create the hash pipeline
	MD5Pipeline pipe (
		 .clk(clk), 
		 .guess(guess), 
		 .guesslen(guesslen), 
		 .hashA(hashA), 
		 .hashB(hashB), 
		 .hashC(hashC), 
		 .hashD(hashD)
		 );
		 
	assign hit = (hashA == 32'h98500190) && (hashB == 32'hb04fd23c) && (hashC == 32'h7d3f96d6) && (hashD == 32'h727fe128);

endmodule
