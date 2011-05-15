`timescale 1ns / 1ps
/**
	@file md5cracker.v
	@author Andrew D. Zonenberg
	@brief Top-level module for PoC cracker
	
 */
module md5cracker(/*targetA, targetB, targetC, targetD,*/ clk, hit, done, reset, charset, guesslen,
	//debug stuff
	hashA, hashB, hashC, hashD
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
	output wire[31:0] hashA;
	output wire[31:0] hashB;
	output wire[31:0] hashC;
	output wire[31:0] hashD;
	wire[127:0] guess;
	//synthesis attribute s of hashA is yes;
	//synthesis attribute s of hashB is yes;
	//synthesis attribute s of hashC is yes;
	//synthesis attribute s of hashD is yes;
	//synthesis attribute s of guess is yes;
	
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

endmodule
