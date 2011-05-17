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
	wire[31:0] digA[64:0];
	wire[31:0] digB[64:0];
	wire[31:0] digC[64:0];
	wire[31:0] digD[64:0];
	
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
			buf0[k] = 32'h0;
			buf1[k] = 32'h0;
			buf2[k] = 32'h0;
			buf3[k] = 32'h0;
			buf14[k] = 32'h0;
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
	
	//Compute the output values
	//TODO: optimize out the final addition
	always @(posedge clk) begin
		//$display("END OF ROUND 1:  %x, %x, %x, %x", digA[16], digB[16], digC[16], digD[16]);
		//$display("END OF ROUND 2:  %x, %x, %x, %x", digA[32], digB[32], digC[32], digD[32]);
		//$display("END OF ROUND 3:  %x, %x, %x, %x", digA[48], digB[48], digC[48], digD[48]);
		//$display("END OF ROUND 4:  %x, %x, %x, %x", digA[64], digB[64], digC[64], digD[64]);
		/*
		hashA <= digA[64] + 32'h67452301;
		hashB <= digB[64] + 32'hefcdab89;
		hashC <= digC[64] + 32'h98badcfe;
		hashD <= digD[64] + 32'h10325476;
		*/
		hashA <= digA[16];
		hashB <= digB[16];
		hashC <= digC[16];
		hashD <= digD[16];
	end
	
	//ROUND 1
	MD5RoundF #(.s(5'd7),  .p(0)) round1_0 (clk, digA[0], digB[0], digC[0], digD[0], buf0[0], 		32'hd76aa478, digA[1], digB[1], digC[1], digD[1]);
	MD5RoundF #(.s(5'd12), .p(0)) round1_1 (clk, digD[1], digA[1], digB[1], digC[1], buf1[1], 		32'he8c7b756, digD[2], digA[2], digB[2], digC[2]);
	MD5RoundF #(.s(5'd17), .p(0)) round1_2 (clk, digC[2], digD[2], digA[2], digB[2], buf2[2], 		32'h242070db, digC[3], digD[3], digA[3], digB[3]);
	MD5RoundF #(.s(5'd22), .p(0)) round1_3 (clk, digB[3], digC[3], digD[3], digA[3], buf3[3], 		32'hc1bdceee, digB[4], digC[4], digD[4], digA[4]);
	MD5RoundF #(.s(5'd7),  .p(0)) round1_4 (clk, digA[4], digB[4], digC[4], digD[4], 32'h0,			32'hf57c0faf, digA[5], digB[5], digC[5], digD[5]);
	MD5RoundF #(.s(5'd12), .p(0)) round1_5 (clk, digD[5], digA[5], digB[5], digC[5], 32'h0,			32'h4787c62a, digD[6], digA[6], digB[6], digC[6]);
	MD5RoundF #(.s(5'd17), .p(0)) round1_6 (clk, digC[6], digD[6], digA[6], digB[6], 32'h0,			32'ha8304613, digC[7], digD[7], digA[7], digB[7]);
	MD5RoundF #(.s(5'd22), .p(0)) round1_7 (clk, digB[7], digC[7], digD[7], digA[7], 32'h0,			32'hfd469501, digB[8], digC[8], digD[8], digA[8]);
	MD5RoundF #(.s(5'd7),  .p(0)) round1_8 (clk, digA[8], digB[8], digC[8], digD[8], 32'h0,			32'h698098d8, digA[9], digB[9], digC[9], digD[9]);
	MD5RoundF #(.s(5'd12), .p(0)) round1_9 (clk, digD[9], digA[9], digB[9], digC[9], 32'h0,			32'h8b44f7af, digD[10], digA[10], digB[10], digC[10]);
	MD5RoundF #(.s(5'd17), .p(0)) round1_10(clk, digC[10], digD[10], digA[10], digB[10], 32'h0,		32'hffff5bb1, digC[11], digD[11], digA[11], digB[11]);
	MD5RoundF #(.s(5'd22), .p(0)) round1_11(clk, digB[11], digC[11], digD[11], digA[11], 32'h0,		32'h895cd7be, digB[12], digC[12], digD[12], digA[12]);
	MD5RoundF #(.s(5'd7), .p(0))  round1_12(clk, digA[12], digB[12], digC[12], digD[12], 32'h0,		32'h6b901122, digA[13], digB[13], digC[13], digD[13]);
	MD5RoundF #(.s(5'd12), .p(0)) round1_13(clk, digD[13], digA[13], digB[13], digC[13], 32'h0,		32'hfd987193, digD[14], digA[14], digB[14], digC[14]);
	MD5RoundF #(.s(5'd17), .p(0)) round1_14(clk, digC[14], digD[14], digA[14], digB[14], buf14[14], 32'ha679438e, digC[15], digD[15], digA[15], digB[15]);
	MD5RoundF #(.s(5'd22), .p(0)) round1_15(clk, digB[15], digC[15], digD[15], digA[15], 32'h0,		32'h49b40821, digB[16], digC[16], digD[16], digA[16]);
	
	/*
	//ROUND 2
	MD5RoundG #(.s(5'd5), .p(0))	round2_0(clk, digA[16], digB[16], digC[16], digD[16], buf1[16],	32'hf61e2562, digA[17], digB[17], digC[17], digD[17]);
	MD5RoundG #(.s(5'd9), .p(0)) 	round2_1(clk, digD[17], digA[17], digB[17], digC[17], 32'h0,		32'hc040b340, digD[18], digA[18], digB[18], digC[18]);
	MD5RoundG #(.s(5'd14), .p(0)) round2_2(clk, digC[18], digD[18], digA[18], digB[18], 32'h0,		32'h265e5a51, digC[19], digD[19], digA[19], digB[19]);
	MD5RoundG #(.s(5'd20), .p(0)) round2_3(clk, digB[19], digC[19], digD[19], digA[19], buf0[19],	32'he9b6c7aa, digB[20], digC[20], digD[20], digA[20]);
	MD5RoundG #(.s(5'd5), .p(0)) 	round2_4(clk, digA[20], digB[20], digC[20], digD[20], 32'h0,		32'hd62f105d, digA[21], digB[21], digC[21], digD[21]);
	MD5RoundG #(.s(5'd9), .p(0)) 	round2_5(clk, digD[21], digA[21], digB[21], digC[21], 32'h0,		32'h02441453, digD[22], digA[22], digB[22], digC[22]);
	MD5RoundG #(.s(5'd14), .p(0)) round2_6(clk, digC[22], digD[22], digA[22], digB[22], 32'h0,		32'hd8a1e681, digC[23], digD[23], digA[23], digB[23]);
	MD5RoundG #(.s(5'd20), .p(0)) round2_7(clk, digB[23], digC[23], digD[23], digA[23], 32'h0,		32'he7d3fbc8, digB[24], digC[24], digD[24], digA[24]);
	MD5RoundG #(.s(5'd5), .p(0)) 	round2_8(clk, digA[24], digB[24], digC[24], digD[24], 32'h0,		32'h21e1cde6, digA[25], digB[25], digC[25], digD[25]);
	MD5RoundG #(.s(5'd9), .p(0))	round2_9(clk, digD[25], digA[25], digB[25], digC[25], buf14[25],	32'hc33707d6, digD[26], digA[26], digB[26], digC[26]);
	MD5RoundG #(.s(5'd14), .p(0)) round2_10(clk, digC[26], digD[26], digA[26], digB[26], buf3[26],	32'hf4d50d87, digC[27], digD[27], digA[27], digB[27]);
	MD5RoundG #(.s(5'd20), .p(0)) round2_11(clk, digB[27], digC[27], digD[27], digA[27], 32'h0,		32'h455a14ed, digB[28], digC[28], digD[28], digA[28]);
	MD5RoundG #(.s(5'd5), .p(0))	round2_12(clk, digA[28], digB[28], digC[28], digD[28], 32'h0,		32'ha9e3e905, digA[29], digB[29], digC[29], digD[29]);
	MD5RoundG #(.s(5'd9), .p(0))	round2_13(clk, digD[29], digA[29], digB[29], digC[29], buf2[29],	32'hfcefa3f8, digD[30], digA[30], digB[30], digC[30]);
	MD5RoundG #(.s(5'd14), .p(0)) round2_14(clk, digC[30], digD[30], digA[30], digB[30], 32'h0,		32'h676f02d9, digC[31], digD[31], digA[31], digB[31]);
	MD5RoundG #(.s(5'd20), .p(0)) round2_15(clk, digB[31], digC[31], digD[31], digA[31], 32'h0,		32'h8d2a4c8a, digB[32], digC[32], digD[32], digA[32]);
			
	//ROUND 3
	MD5RoundH #(.s(5'd4), .p(0)) round3_0(clk, digA[32], digB[32], digC[32], digD[32], 32'h0,			32'hfffa3942, digA[33], digB[33], digC[33], digD[33]);
	MD5RoundH #(.s(5'd11), .p(0)) round3_1(clk, digD[33], digA[33], digB[33], digC[33], 32'h0,		32'h8771f681, digD[34], digA[34], digB[34], digC[34]);
	MD5RoundH #(.s(5'd16), .p(0)) round3_2(clk, digC[34], digD[34], digA[34], digB[34], 32'h0,		32'h6d9d6122, digC[35], digD[35], digA[35], digB[35]);
	MD5RoundH #(.s(5'd23), .p(0)) round3_3(clk, digB[35], digC[35], digD[35], digA[35], buf14[35],	32'hfde5380c, digB[36], digC[36], digD[36], digA[36]);
	MD5RoundH #(.s(5'd4), .p(0)) round3_4(clk, digA[36], digB[36], digC[36], digD[36], buf1[37],		32'ha4beea44, digA[37], digB[37], digC[37], digD[37]);
	MD5RoundH #(.s(5'd11), .p(0)) round3_5(clk, digD[37], digA[37], digB[37], digC[37], 32'h0,		32'h4bdecfa9, digD[38], digA[38], digB[38], digC[38]);
	MD5RoundH #(.s(5'd16), .p(0)) round3_6(clk, digC[38], digD[38], digA[38], digB[38], 32'h0,		32'hf6bb4b60, digC[39], digD[39], digA[39], digB[39]);
	MD5RoundH #(.s(5'd23), .p(0)) round3_7(clk, digB[39], digC[39], digD[39], digA[39], 32'h0,		32'hbebfbc70, digB[40], digC[40], digD[40], digA[40]);
	MD5RoundH #(.s(5'd4), .p(0)) round3_8(clk, digA[40], digB[40], digC[40], digD[40], 32'h0,			32'h289b7ec6, digA[41], digB[41], digC[41], digD[41]);
	MD5RoundH #(.s(5'd11), .p(0)) round3_9(clk, digD[41], digA[41], digB[41], digC[41], buf0[41],	32'heaa127fa, digD[42], digA[42], digB[42], digC[42]);
	MD5RoundH #(.s(5'd16), .p(0)) round3_10(clk, digC[42], digD[42], digA[42], digB[42], buf3[42],	32'hd4ef3085, digC[43], digD[43], digA[43], digB[43]);
	MD5RoundH #(.s(5'd23), .p(0)) round3_11(clk, digB[43], digC[43], digD[43], digA[43], 32'h0,		32'h04881d05, digB[44], digC[44], digD[44], digA[44]);
	MD5RoundH #(.s(5'd4), .p(0)) round3_12(clk, digA[44], digB[44], digC[44], digD[44], 32'h0,		32'hd9d4d039, digA[45], digB[45], digC[45], digD[45]);
	MD5RoundH #(.s(5'd11), .p(0)) round3_13(clk, digD[45], digA[45], digB[45], digC[45], 32'h0, 		32'he6db99e5, digD[46], digA[46], digB[46], digC[46]);
	MD5RoundH #(.s(5'd16), .p(0)) round3_14(clk, digC[46], digD[46], digA[46], digB[46], 32'h0,		32'h1fa27cf8, digC[47], digD[47], digA[47], digB[47]);
	MD5RoundH #(.s(5'd23), .p(0)) round3_15(clk, digB[47], digC[47], digD[47], digA[47], buf2[47],	32'hc4ac5665, digB[48], digC[48], digD[48], digA[48]);
		
	//ROUND 4
	MD5RoundI #(.s(5'd6), .p(0)) round4_0(clk, digA[48], digB[48], digC[48], digD[48], buf0[48],		32'hf4292244, digA[49], digB[49], digC[49], digD[49]);
	MD5RoundI #(.s(5'd10), .p(0)) round4_1(clk, digD[49], digA[49], digB[49], digC[49], 32'h0,		32'h432aff97, digD[50], digA[50], digB[50], digC[50]);
	MD5RoundI #(.s(5'd15), .p(0)) round4_2(clk, digC[50], digD[50], digA[50], digB[50], buf14[50],	32'hab9423a7, digC[51], digD[51], digA[51], digB[51]);
	MD5RoundI #(.s(5'd21), .p(0)) round4_3(clk, digB[51], digC[51], digD[51], digA[51], 32'h0,		32'hfc93a039, digB[52], digC[52], digD[52], digA[52]);
	MD5RoundI #(.s(5'd6), .p(0)) round4_4(clk, digA[52], digB[52], digC[52], digD[52], 32'h0,			32'h655b59c3, digA[53], digB[53], digC[53], digD[53]);
	MD5RoundI #(.s(5'd10), .p(0)) round4_5(clk, digD[53], digA[53], digB[53], digC[53], buf3[53],	32'h8f0ccc92, digD[54], digA[54], digB[54], digC[54]);
	MD5RoundI #(.s(5'd15), .p(0)) round4_6(clk, digC[54], digD[54], digA[54], digB[54], 32'h0,		32'hffeff47d, digC[55], digD[55], digA[55], digB[55]);
	MD5RoundI #(.s(5'd21), .p(0)) round4_7(clk, digB[55], digC[55], digD[55], digA[55], buf1[55],	32'h85845dd1, digB[56], digC[56], digD[56], digA[56]);
	MD5RoundI #(.s(5'd6), .p(0)) round4_8(clk, digA[56], digB[56], digC[56], digD[56], 32'h0,			32'h6fa87e4f, digA[57], digB[57], digC[57], digD[57]);
	MD5RoundI #(.s(5'd10), .p(0)) round4_9(clk, digD[57], digA[57], digB[57], digC[57], 32'h0,		32'hfe2ce6e0, digD[58], digA[58], digB[58], digC[58]);
	MD5RoundI #(.s(5'd15), .p(0)) round4_10(clk, digC[58], digD[58], digA[58], digB[58], 32'h0,		32'ha3014314, digC[59], digD[59], digA[59], digB[59]);
	MD5RoundI #(.s(5'd21), .p(0)) round4_11(clk, digB[59], digC[59], digD[59], digA[59], 32'h0,		32'h4e0811a1, digB[60], digC[60], digD[60], digA[60]);
	MD5RoundI #(.s(5'd6), .p(0)) round4_12(clk, digA[60], digB[60], digC[60], digD[60], 32'h0,		32'hf7537e82, digA[61], digB[61], digC[61], digD[61]);
	MD5RoundI #(.s(5'd10), .p(0)) round4_13(clk, digD[61], digA[61], digB[61], digC[61], 32'h0,		32'hbd3af235, digD[62], digA[62], digB[62], digC[62]);
	MD5RoundI #(.s(5'd15), .p(0)) round4_14(clk, digC[62], digD[62], digA[62], digB[62], buf2[62],	32'h2ad7d2bb, digC[63], digD[63], digA[63], digB[63]);
	MD5RoundI #(.s(5'd21), .p(0)) round4_15(clk, digB[63], digC[63], digD[63], digA[63], 32'h0,		32'heb86d391, digB[64], digC[64], digD[64], digA[64]);
	*/

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
module MD5RoundF(clk, a, b, c, d, k, t, outa, outb, outc, outd);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] outa;
	output reg[31:0] outb;
	output reg[31:0] outc;
	output reg[31:0] outd;
	
	initial begin
		outa <= 32'd0;
		outb <= 32'd0;
		outc <= 32'd0;
		outd <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	reg[31:0] f;
	reg[31:0] left;
	reg[31:0] right;
	wire[31:0] inner;
	initial begin
		f <= 32'h0;
		left <= 32'h0;
		right <= 32'h0;
	end
	always @(posedge clk) begin
		f <= (b & c) | (~b & d);
		left <= a + f;
		right <= k + t;
	end
	assign inner = left + right;
	wire[63:0] xrrotval = ({inner, inner} << s);
	reg[31:0] rrotval;
	initial begin
		rrotval <= 32'h0;
	end
	always @(posedge clk) begin
		rrotval <= xrrotval[63:32];
	end
	
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
		
		outa <= b + rrotval;
		outb <= b;
		outc <= c;
		outd <= d;
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
module MD5RoundG(clk, a, b, c, d, k, t, outa, outb, outc, outd);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] outa;
	output reg[31:0] outb;
	output reg[31:0] outc;
	output reg[31:0] outd;
	
	initial begin
		outa <= 32'd0;
		outb <= 32'd0;
		outc <= 32'd0;
		outd <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	reg[31:0] f;
	reg[31:0] left;
	reg[31:0] right;
	wire[31:0] inner;
	initial begin
		f <= 32'h0;
		left <= 32'h0;
		right <= 32'h0;
	end
	always @(posedge clk) begin
		f <= (b & d) | (~d & c);
		left <= a + f;
		right <= k + t;
	end
	
	assign inner = left + right;
	
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
		
		outa <= b + rrotval;
		outb <= b;
		outc <= c;
		outd <= d;
	end
	
endmodule

/**
	@brief Does an H round
	
	@param clk		Clock
	@param a			Current A word
	@param b			Current B word
	@param c			Current C word
	@param d			Current D word
	@param k			Word #K from the message
	@param t			Input constant T
	
	@param s			Shift amount
 */
module MD5RoundH(clk, a, b, c, d, k, t, outa, outb, outc, outd);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] outa;
	output reg[31:0] outb;
	output reg[31:0] outc;
	output reg[31:0] outd;
	
	initial begin
		outa <= 32'd0;
		outb <= 32'd0;
		outc <= 32'd0;
		outd <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	reg[31:0] f;
	reg[31:0] left;
	reg[31:0] right;
	wire[31:0] inner;
	initial begin
		f = 32'h0;
		left = 32'h0;
		right = 32'h0;
	end
	always @(posedge clk) begin
		f <= b ^ c ^ d;
		left <= a + f;
		right <= k + t;
	end
	assign inner = left + right;
	
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
	
		outa <= b + rrotval;
		outb <= b;
		outc <= c;
		outd <= d;
	
	end
	
endmodule

/**
	@brief Does an I round
	
	@param clk		Clock
	@param a			Current A word
	@param b			Current B word
	@param c			Current C word
	@param d			Current D word
	@param k			Word #K from the message
	@param t			Input constant T
	
	@param s			Shift amount
 */
module MD5RoundI(clk, a, b, c, d, k, t, outa, outb, outc, outd);
	input wire clk;
	input wire[31:0] a;
	input wire[31:0] b;
	input wire[31:0] c;
	input wire[31:0] d;
	input wire[31:0] k;
	input wire[31:0] t;
	output reg[31:0] outa;
	output reg[31:0] outb;
	output reg[31:0] outc;
	output reg[31:0] outd;
	
	initial begin
		outa <= 32'd0;
		outb <= 32'd0;
		outc <= 32'd0;
		outd <= 32'd0;
	end
	
	parameter s = 7;		//shift value
	parameter p = 0;		//true to print

	//Core round function
	reg[31:0] f;
	reg[31:0] left;
	reg[31:0] right;
	wire[31:0] inner;
	initial begin
		f = 32'h0;
		left = 32'h0;
		right = 32'h0;
	end
	always @(posedge clk) begin
		f <= c ^ (b | ~d);
		left <= a + f;
		right <= k + t;
	end
	assign inner = left + right;
	
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
		
		outa <= b + rrotval;
		outb <= b;
		outc <= c;
		outd <= d;
		
	end
	
endmodule
