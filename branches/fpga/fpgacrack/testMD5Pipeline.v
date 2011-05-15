`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   16:14:11 05/14/2011
// Design Name:   MD5Pipeline
// Module Name:   /home/azonenberg/native/programming/cracker/branches/fpga/fpgacrack/testMD5Pipeline.v
// Project Name:  fpgacrack
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: MD5Pipeline
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module testMD5Pipeline;

	// Inputs
	reg clk;
	reg [127:0] guess;
	reg [3:0] guesslen;

	// Outputs
	wire [31:0] hashA;
	wire [31:0] hashB;
	wire [31:0] hashC;
	wire [31:0] hashD;
	
	reg ready;

	// Instantiate the Unit Under Test (UUT)
	MD5Pipeline uut (
		.clk(clk), 
		.guess(guess), 
		.guesslen(guesslen), 
		.hashA(hashA), 
		.hashB(hashB), 
		.hashC(hashC), 
		.hashD(hashD)
	);

	initial begin
		// Initialize Inputs
		clk = 0;
		guess = 0;
		guesslen = 0;
		ready = 0;

		// Wait 100 ns for global reset to finish
		#100;
        
		guess = 128'h61626364000000000000000000000000;
		guesslen = 3;
		clk = 1;
		#10;
		clk = 0;
		  
		ready = 1;

	end
	
	always begin
		if(ready)
			clk = 0;
		#10;
		if(ready)
			clk = 1;
		#10;
	end
      
endmodule

