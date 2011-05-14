`timescale 1ns / 1ps
/**
	@file MD5Pipeline.v
	@author Andrew D. Zonenberg
	@brief A single MD5 hash pipeline
	
	@param clk			Clock
	@param guesslen	Guess length (used for padding)
	@param guess		Input guess value to be hashed
	@param hashA		A value of digest
	@param hashB		B value of digest
	@param hashC		C value of digest
	@param hashD		D value of digest
 */
module MD5Pipeline(clk, guess, guesslen, hashA, hashB, hashC, hashD, buf14
    );
	 
	input wire clk;
	input wire[127:0] guess;
	input wire[3:0] guesslen;		//up to 16
	output reg[31:0] hashA;
	output reg[31:0] hashB;
	output reg[31:0] hashC;
	output reg[31:0] hashD;
	
	reg[31:0] buf0;
	reg[31:0] buf1;
	reg[31:0] buf2;
	reg[31:0] buf3;
	//buf4...13 are always 0
	output reg[31:0] buf14;
	//buf15 is always 0
	
	//Buffer the first stage inputs
	always @(posedge clk) begin
		
		//Copy the input (adjusting endianness as needed) and append padding bits
		
		//FIRST WORD
		if(guesslen >= 4)
			buf0 <= {guess[103:96], guess[111:104], guess[119:112], guess[127:120]};
		else if(guesslen == 3)
			buf0 <= {8'h80, guess[111:104], guess[119:112], guess[127:120]};
		else if(guesslen == 2)
			buf0 <= {8'h00, 8'h80, guess[119:112], guess[127:120]};
		else if(guesslen == 1)
			buf0 <= {8'h0, 8'h0, 8'h80, guess[127:120]};
		else
			buf0 <= {8'h00, 8'h0, 8'h0, 8'h80};
		
		//SECOND WORD
		if(guesslen >= 8)
			buf1 <= {guess[71:64], guess[79:72], guess[87:80], guess[95:88]};
		else if(guesslen == 7)
			buf1 <= {8'h80, guess[79:72], guess[87:80], guess[95:88]};
		else if(guesslen == 6)
			buf1 <= {8'h00, 8'h80, guess[87:80], guess[95:88]};
		else if(guesslen == 5)
			buf1 <= {8'h00, 8'h0, 8'h80, guess[95:88]};
		else if(guesslen == 4)
			buf1 <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf1 <= 32'h0;
			
		//THIRD WORD
		if(guesslen >= 12)
			buf2 <= {guess[39:32], guess[47:40], guess[55:48], guess[63:56]};
		else if(guesslen == 11)
			buf2 <= {8'h80, guess[47:40], guess[55:48], guess[63:56]};
		else if(guesslen == 10)
			buf2 <= {8'h00, 8'h80, guess[55:48], guess[63:56]};
		else if(guesslen == 9)
			buf2 <= {8'h00, 8'h0, 8'h80, guess[63:56]};
		else if(guesslen == 8)
			buf2 <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf2 <= 32'h0;
			
		//FOURTH WORD
		//no >=16 allowed
		if(guesslen == 15)
			buf3 <= {8'h80, guess[15:8], guess[23:16], guess[31:24]};
		else if(guesslen == 14)
			buf3 <= {8'h00, 8'h80, guess[23:16], guess[31:24]};
		else if(guesslen == 13)
			buf3 <= {8'h00, 8'h0, 8'h80, guess[31:24]};
		else if(guesslen == 12)
			buf3 <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf3 <= 32'h0;
	
		
		//Set up length padding (converting from bytes to bits)
		buf14 <= {guesslen, 3'd0};
		
		//DEBUG!!!
		hashA <= buf0;
		hashB <= buf1;
		hashC <= buf2;
		hashD <= buf3;
		
	end
	

endmodule