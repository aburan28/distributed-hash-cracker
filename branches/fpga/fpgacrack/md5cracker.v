`timescale 1ns / 1ps
/**
	@file md5cracker.v
	@author Andrew D. Zonenberg
	@brief Top-level module for PoC cracker
	
 */
module md5cracker(targetA, targetB, targetC, targetD, clk, hit, done, guess);

	input wire [31:0] targetA;
	input wire [31:0] targetB;
	input wire [31:0] targetC;
	input wire [31:0] targetD;
   input wire clk;
   output wire hit;
	output wire done;
	
	output wire[127:0] guess;
	
	//Inputs for guess generation
	reg reset;
	reg[2:0] charset;
	reg[4:0] guesslen;
	
	//Reset on startup
	initial begin
		reset <= 1'd1;
		charset <= 3'd0;
		guesslen <= 5'd2;	//2 characters
	end
	
	always @(posedge clk) begin
		
		//Done with reset
		if(reset == 1'd1) begin
			reset <= 1'd0;
		end
		
		//Normal stuff
		else begin
		end
		
	end
	 
	// Instantiate the module
	GuessGenerator generator (
		 .clk(clk),
		 .charset(charset), 
		 .guesslen(guesslen), 
		 .reset(reset), 
		 .guess(guess),
		 .done(done)
		 );


endmodule
