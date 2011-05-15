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
		$display("END OF ROUND 1:  %x, %x, %x, %x", digA[16], digB[16], digC[16], digD[16]);
		$display("END OF ROUND 2:  %x, %x, %x, %x", digA[32], digB[32], digC[32], digD[32]);
		
		hashA <= digA[32];
		hashB <= digB[32];
		hashC <= digC[32];
		hashD <= digD[32];
	end
	
	//ROUND 1
	MD5RoundF #(.s(5'd7), .p(0)) round1_0(clk, digA[0], digB[0], digC[0], digD[0], buf0[0], 32'hd76aa478, digA[1]);
		assign digB[1] = digB[0];
		assign digC[1] = digC[0];
		assign digD[1] = digD[0];
	MD5RoundF #(.s(5'd12), .p(0)) round1_1(clk, digD[1], digA[1], digB[1], digC[1], buf1[1], 32'he8c7b756, digD[2]);
		assign digB[2] = digB[1];
		assign digC[2] = digC[1];
		assign digA[2] = digA[1]; 
	MD5RoundF #(.s(5'd17), .p(0)) round1_2(clk, digC[2], digD[2], digA[2], digB[2], buf2[2], 32'h242070db, digC[3]);
		assign digD[3] = digD[2];
		assign digA[3] = digA[2]; 
		assign digB[3] = digB[2];
	MD5RoundF #(.s(5'd22), .p(0)) round1_3(clk, digB[3], digC[3], digD[3], digA[3], buf3[3], 32'hc1bdceee, digB[4]);
		assign digD[4] = digD[3];
		assign digA[4] = digA[3]; 
		assign digC[4] = digC[3];
	MD5RoundF #(.s(5'd7), .p(0)) round1_4(clk, digA[4], digB[4], digC[4], digD[4], 32'h0, 32'hf57c0faf, digA[5]);
		assign digB[5] = digB[4];
		assign digC[5] = digC[4];
		assign digD[5] = digD[4];
	MD5RoundF #(.s(5'd12), .p(0)) round1_5(clk, digD[5], digA[5], digB[5], digC[5], 32'h0, 32'h4787c62a, digD[6]);
		assign digB[6] = digB[5];
		assign digC[6] = digC[5];
		assign digA[6] = digA[5]; 
	MD5RoundF #(.s(5'd17), .p(0)) round1_6(clk, digC[6], digD[6], digA[6], digB[6], 32'h0, 32'ha8304613, digC[7]);
		assign digD[7] = digD[6];
		assign digA[7] = digA[6]; 
		assign digB[7] = digB[6];
	MD5RoundF #(.s(5'd22), .p(0)) round1_7(clk, digB[7], digC[7], digD[7], digA[7], 32'h0, 32'hfd469501, digB[8]);
		assign digD[8] = digD[7];
		assign digA[8] = digA[7]; 
		assign digC[8] = digC[7];
	MD5RoundF #(.s(5'd7), .p(0)) round1_8(clk, digA[8], digB[8], digC[8], digD[8], 32'h0, 32'h698098d8, digA[9]);
		assign digB[9] = digB[8];
		assign digC[9] = digC[8];
		assign digD[9] = digD[8];
	MD5RoundF #(.s(5'd12), .p(0)) round1_9(clk, digD[9], digA[9], digB[9], digC[9], 32'h0, 32'h8b44f7af, digD[10]);
		assign digB[10] = digB[9];
		assign digC[10] = digC[9];
		assign digA[10] = digA[9]; 
	MD5RoundF #(.s(5'd17), .p(0)) round1_10(clk, digC[10], digD[10], digA[10], digB[10], 32'h0, 32'hffff5bb1, digC[11]);
		assign digD[11] = digD[10];
		assign digA[11] = digA[10]; 
		assign digB[11] = digB[10];
	MD5RoundF #(.s(5'd22), .p(0)) round1_11(clk, digB[11], digC[11], digD[11], digA[11], 32'h0, 32'h895cd7be, digB[12]);
		assign digD[12] = digD[11];
		assign digA[12] = digA[11]; 
		assign digC[12] = digC[11];
	MD5RoundF #(.s(5'd7), .p(0)) round1_12(clk, digA[12], digB[12], digC[12], digD[12], 32'h0, 32'h6b901122, digA[13]);
		assign digB[13] = digB[12];
		assign digC[13] = digC[12];
		assign digD[13] = digD[12];
	MD5RoundF #(.s(5'd12), .p(0)) round1_13(clk, digD[13], digA[13], digB[13], digC[13], 32'h0, 32'hfd987193, digD[14]);
		assign digB[14] = digB[13];
		assign digC[14] = digC[13];
		assign digA[14] = digA[13]; 
	MD5RoundF #(.s(5'd17), .p(0)) round1_14(clk, digC[14], digD[14], digA[14], digB[14], buf14[14], 32'ha679438e, digC[15]);
		assign digD[15] = digD[14];
		assign digA[15] = digA[14]; 
		assign digB[15] = digB[14];
	MD5RoundF #(.s(5'd22), .p(0)) round1_15(clk, digB[15], digC[15], digD[15], digA[15], 32'h0, 32'h49b40821, digB[16]);
		assign digD[16] = digD[15];
		assign digA[16] = digA[15]; 
		assign digC[16] = digC[15];
		
	//ROUND 2
	MD5RoundG #(.s(5'd5), .p(0)) round2_0(clk, digA[16], digB[16], digC[16], digD[16], buf1[16], 32'hf61e2562, digA[17]);
		assign digB[17] = digB[16];
		assign digC[17] = digC[16];
		assign digD[17] = digD[16];
	MD5RoundG #(.s(5'd9), .p(0)) round2_1(clk, digD[17], digA[17], digB[17], digC[17], 32'h0, 32'hc040b340, digD[18]);
		assign digB[18] = digB[17];
		assign digC[18] = digC[17];
		assign digA[18] = digA[17]; 
	MD5RoundG #(.s(5'd14), .p(0)) round2_2(clk, digC[18], digD[18], digA[18], digB[18], 32'h0, 32'h265e5a51, digC[19]);
		assign digD[19] = digD[18];
		assign digA[19] = digA[18]; 
		assign digB[19] = digB[18];
	MD5RoundG #(.s(5'd20), .p(0)) round2_3(clk, digB[19], digC[19], digD[19], digA[19], buf0[19], 32'he9b6c7aa, digB[20]);
		assign digD[20] = digD[19];
		assign digA[20] = digA[19]; 
		assign digC[20] = digC[19];
	MD5RoundG #(.s(5'd5), .p(0)) round2_4(clk, digA[20], digB[20], digC[20], digD[20], 32'h0, 32'hd62f105d, digA[21]);
		assign digB[21] = digB[20];
		assign digC[21] = digC[20];
		assign digD[21] = digD[20];
	MD5RoundG #(.s(5'd9), .p(0)) round2_5(clk, digD[21], digA[21], digB[21], digC[21], 32'h0, 32'h02441453, digD[22]);
		assign digB[22] = digB[21];
		assign digC[22] = digC[21];
		assign digA[22] = digA[21]; 
	MD5RoundG #(.s(5'd14), .p(0)) round2_6(clk, digC[22], digD[22], digA[22], digB[22], 32'h0, 32'hd8a1e681, digC[23]);
		assign digD[23] = digD[22];
		assign digA[23] = digA[22]; 
		assign digB[23] = digB[22];
	MD5RoundG #(.s(5'd20), .p(0)) round2_7(clk, digB[23], digC[23], digD[23], digA[23], 32'h0, 32'he7d3fbc8, digB[24]);
		assign digD[24] = digD[23];
		assign digA[24] = digA[23]; 
		assign digC[24] = digC[23];
	MD5RoundG #(.s(5'd5), .p(0)) round2_8(clk, digA[24], digB[24], digC[24], digD[24], 32'h0, 32'h21e1cde6, digA[25]);
		assign digB[25] = digB[24];
		assign digC[25] = digC[24];
		assign digD[25] = digD[24];
	MD5RoundG #(.s(5'd9), .p(0)) round2_9(clk, digD[25], digA[25], digB[25], digC[25], buf14[25], 32'hc33707d6, digD[26]);
		assign digB[26] = digB[25];
		assign digC[26] = digC[25];
		assign digA[26] = digA[25]; 
	MD5RoundG #(.s(5'd14), .p(0)) round2_10(clk, digC[26], digD[26], digA[26], digB[26], buf3[26], 32'hf4d50d87, digC[27]);
		assign digD[27] = digD[26];
		assign digA[27] = digA[26]; 
		assign digB[27] = digB[26];
	MD5RoundG #(.s(5'd20), .p(0)) round2_11(clk, digB[27], digC[27], digD[27], digA[27], 32'h0, 32'h455a14ed, digB[28]);
		assign digD[28] = digD[27];
		assign digA[28] = digA[27]; 
		assign digC[28] = digC[27];
	MD5RoundG #(.s(5'd5), .p(0)) round2_12(clk, digA[28], digB[28], digC[28], digD[28], 32'h0, 32'ha9e3e905, digA[29]);
		assign digB[29] = digB[28];
		assign digC[29] = digC[28];
		assign digD[29] = digD[28];
	MD5RoundG #(.s(5'd9), .p(0)) round2_13(clk, digD[29], digA[29], digB[29], digC[29], buf2[29], 32'hfcefa3f8, digD[30]);
		assign digB[30] = digB[29];
		assign digC[30] = digC[29];
		assign digA[30] = digA[29]; 
	MD5RoundG #(.s(5'd14), .p(0)) round2_14(clk, digC[30], digD[30], digA[30], digB[30], 32'h0, 32'h676f02d9, digC[31]);
		assign digD[31] = digD[30];
		assign digA[31] = digA[30]; 
		assign digB[31] = digB[30];
	MD5RoundG #(.s(5'd20), .p(0)) round2_15(clk, digB[31], digC[31], digD[31], digA[31], 32'h0, 32'h8d2a4c8a, digB[32]);
		assign digD[32] = digD[31];
		assign digA[32] = digA[31]; 
		assign digC[32] = digC[31];
	

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
	
	initial begin
		out <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	wire[31:0] f;
	wire[31:0] inner;
	assign f = (b & c) | (~b & d);
	assign inner = a + f + k + t;
	
	wire[63:0] xrrotval = ({inner, inner} << s);
	wire[31:0] rrotval = xrrotval[63:32];
	
	//"Funnel shift" for left rotate
	always @(posedge clk) begin
		if(p) begin
			$display("------");
			$display("  A = %x", a);
			$display("  B = %x", b);
			$display("  C = %x", c);
			$display("  D = %x", d);
			$display("  X = %x", k);
			$display("  T = %x", t);
			$display("  fval = %x", f);
			$display("  rotval = %x", inner);
			$display("  rrotval = %x", rrotval);
		end
		out <= b + rrotval;
	end
	
endmodule

/**
	@brief Does an G round
	
	@param clk		Clock
	@param a			Current A word
	@param b			Current B word
	@param c			Current C word
	@param d			Current D word
	@param k			Word #K from the message
	@param t			Input constant T
	
	@param s			Shift amount
 */
module MD5RoundG(clk, a, b, c, d, k, t, out);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] out;
	
	initial begin
		out <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	wire[31:0] f;
	wire[31:0] inner;
	assign f = (b & d) | (~d & c);
	assign inner = a + f + k + t;
	
	wire[63:0] xrrotval = ({inner, inner} << s);
	wire[31:0] rrotval = xrrotval[63:32];
	
	//"Funnel shift" for left rotate
	always @(posedge clk) begin
		if(p) begin
			$display("------");
			$display("  A = %x", a);
			$display("  B = %x", b);
			$display("  C = %x", c);
			$display("  D = %x", d);
			$display("  X = %x", k);
			$display("  T = %x", t);
			$display("  fval = %x", f);
			$display("  rotval = %x", inner);
			$display("  rrotval = %x", rrotval);
		end
		out <= b + rrotval;
	end
	
endmodule
