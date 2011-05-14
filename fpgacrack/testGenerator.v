`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   04:47:42 05/14/2011
// Design Name:   GuessGenerator
// Module Name:   /home/azonenberg/native/programming/cracker/branches/fpga/fpgacrack/testGenerator.v
// Project Name:  fpgacrack
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: GuessGenerator
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module testGenerator;

	// Inputs
	reg clk;
	reg [2:0] charset;
	reg [4:0] guesslen;
	reg reset;

	// Outputs
	wire [127:0] guess;
	wire done;

	// Instantiate the Unit Under Test (UUT)
	GuessGenerator uut (
		.clk(clk), 
		.charset(charset), 
		.guesslen(guesslen), 
		.reset(reset), 
		.guess(guess), 
		.done(done)
	);

	reg ready;

	initial begin
		// Initialize Inputs
		clk = 0;
		charset = 0;
		guesslen = 0;
		reset = 0;
		ready = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		//First clock, load all of the stuff
		reset = 1'd1;
		charset = 3'd0;	//lowercase letters
		guesslen = 5'd2;	//2 characters
		clk = 1;
		#5;
		clk = 0;
		guesslen = 5'd0;
		reset = 0;
		#5;
		  
		ready = 1;

	end
	
	always begin
		if(ready)
			clk = 0;
		#5;
		if(ready)
			clk = 1;
		#5;
	end
      
endmodule

