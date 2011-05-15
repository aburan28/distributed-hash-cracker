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
module MD5Pipeline(clk, guess, guesslen, hashA, hashB, hashC, hashD
    );
	 
	input wire clk;
	input wire[127:0] guess;
	input wire[3:0] guesslen;		//up to 16
	output reg[31:0] hashA;
	output reg[31:0] hashB;
	output reg[31:0] hashC;
	output reg[31:0] hashD;
	
	//Buffering for buf0...3 and buf14
	reg[31:0] buf0[63:0];
	reg[31:0] buf1[63:0];
	reg[31:0] buf2[63:0];
	reg[31:0] buf3[63:0];
	//buf4...13 are always 0
	reg[31:0] buf14[63:0];
	//buf15 is always 0
	
	//Digest values
	wire[31:0] digA[63:0];
	wire[31:0] digB[63:0];
	wire[31:0] digC[63:0];
	wire[31:0] digD[63:0];
	
	//Initialize the first digest values
	assign digA[0] = 32'h67452301;
	assign digB[0] = 32'hefcdab89;
	assign digC[0] = 32'h98badcfe;
	assign digD[0] = 32'h10325476;
	
	//Init
	reg[8:0] k;
	initial begin
		//Clear all buf values to zero
		for(k=0; k<64; k = k+1) begin
			buf0[k] = 0;
			buf1[k] = 0;
			buf2[k] = 0;
			buf3[k] = 0;
			buf14[k] = 0;
		end				
	end
	
	//Buffer the first stage inputs
	always @(posedge clk) begin
		
		//Copy the input (adjusting endianness as needed) and append padding bits
		
		//FIRST WORD
		if(guesslen >= 4)
			buf0[0] <= {guess[103:96], guess[111:104], guess[119:112], guess[127:120]};
		else if(guesslen == 3)
			buf0[0] <= {8'h80, guess[111:104], guess[119:112], guess[127:120]};
		else if(guesslen == 2)
			buf0[0] <= {8'h00, 8'h80, guess[119:112], guess[127:120]};
		else if(guesslen == 1)
			buf0[0] <= {8'h0, 8'h0, 8'h80, guess[127:120]};
		else
			buf0[0] <= {8'h00, 8'h0, 8'h0, 8'h80};
		
		//SECOND WORD
		if(guesslen >= 8)
			buf1[0] <= {guess[71:64], guess[79:72], guess[87:80], guess[95:88]};
		else if(guesslen == 7)
			buf1[0] <= {8'h80, guess[79:72], guess[87:80], guess[95:88]};
		else if(guesslen == 6)
			buf1[0] <= {8'h00, 8'h80, guess[87:80], guess[95:88]};
		else if(guesslen == 5)
			buf1[0] <= {8'h00, 8'h0, 8'h80, guess[95:88]};
		else if(guesslen == 4)
			buf1[0] <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf1[0] <= 32'h0;
			
		//THIRD WORD
		if(guesslen >= 12)
			buf2[0] <= {guess[39:32], guess[47:40], guess[55:48], guess[63:56]};
		else if(guesslen == 11)
			buf2[0] <= {8'h80, guess[47:40], guess[55:48], guess[63:56]};
		else if(guesslen == 10)
			buf2[0] <= {8'h00, 8'h80, guess[55:48], guess[63:56]};
		else if(guesslen == 9)
			buf2[0] <= {8'h00, 8'h0, 8'h80, guess[63:56]};
		else if(guesslen == 8)
			buf2[0] <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf2[0] <= 32'h0;
			
		//FOURTH WORD
		//no >=16 allowed
		if(guesslen == 15)
			buf3[0] <= {8'h80, guess[15:8], guess[23:16], guess[31:24]};
		else if(guesslen == 14)
			buf3[0] <= {8'h00, 8'h80, guess[23:16], guess[31:24]};
		else if(guesslen == 13)
			buf3[0] <= {8'h00, 8'h0, 8'h80, guess[31:24]};
		else if(guesslen == 12)
			buf3[0] <= {8'h00, 8'h0, 8'h0, 8'h80};
		else
			buf3[0] <= 32'h0;
	
		//Set up length padding (converting from bytes to bits)
		buf14[0] <= {guesslen, 3'd0};
		
	end
	
	//Shift all buf values down the pipe
	reg[8:0] n;
	always @(posedge clk) begin
		for(k=1; k<64; k = k+1) begin
			buf0[k] <= buf0[k-1];
			buf1[k] <= buf1[k-1];
			buf2[k] <= buf2[k-1];
			buf3[k] <= buf3[k-1];
			buf14[k] <= buf14[k-1];
		end
	end
	
	//DEBUG: Copy the output
	always @(posedge clk) begin
		$display("digA[0] = %x, digA[1] = %x", digA[0], digA[1]);
		hashA <= digA[1];
		hashB <= digB[1];
		hashC <= digC[1];
		hashD <= digC[1];
	end
	
	//FIRST ROUND
	wire[31:0] outA1_0;
	MD5RoundF #(.s(5'd7)) round1_0(clk, digA[0], digB[0], digC[0], digD[0], buf0[0], 32'hd76aa478, digA[1]);
		assign digB[1] = digB[0];
		assign digC[1] = digC[0];
		assign digD[1] = digD[0];
	
	//TODO: Shift 

endmodule

/**
	@brief Does an F round
	
	@param clk		Clock
	@param a			Current A word
	@param b			Current B word
	@param c			Current C word
	@param d			Current D word
	@param k			Word #K from the message
	@param t			Input constant T
	
	@param s			Shift amount
 */
module MD5RoundF(clk, a, b, c, d, k, t, out);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] out;
	
	parameter s = 7;

	//Core round function
	wire[31:0] f;
	wire[31:0] inner;
	assign f = (b & c) | (~b & d);
	assign inner = a + f + k + t;
	
	wire[63:0] xrrotval = ({inner, inner} << s);
	wire[31:0] rrotval = xrrotval[63:32];
	
	//"Funnel shift" for left rotate
	always @(posedge clk) begin
		/*
		$display("  A = %x", a);
		$display("  X = %x", k);
		$display("  T = %x", t);
		$display("fval = %x", f);
		$display("rotval = %x", inner);
		$display("rrotval = %x", rrotval);
		*/
		out <= b + rrotval;
	end
	
endmodule
