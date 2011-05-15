`timescale 1ns / 1ps
/**
	@file GuessGenerator.v
	@author Andrew D. Zonenberg
	@brief Guess generation module
	
	@param clk			Clock
	@param charset		Charset number
	@param guesslen	Guess length (in characters)
	@param reset		Active-high reset
	@param guess		Output (current guess)
	@param done			Goes high when we overflow the current guess
 */
module GuessGenerator(clk, charset, guesslen, reset, guess, done

	//all debug outputs from here on
);

	input wire clk;
	input wire[2:0] charset;
	input wire[3:0] guesslen;			//max 16 chars
	output wire[127:0] guess;			//ASCII, up to 16 characters supported
	output reg done;
	input wire reset;
	
	reg[8:0] charsetsize;				//length of the charset
	reg[8:0] charsetmax;					//1 - charsetsize
	reg[3:0] length;						//length of each guess
	reg[3:0] lmax;							//1 - length
	
	reg[8:0] guesspos[15:0];	//Current guess
	
	//TODO: we need a "in progress" flag
	
	//Reset everything to zero
	initial begin
		charsetsize <= 9'd0;
		charsetmax <= 9'd0;
		
		done <= 1'd0;
		
		guesspos[0] <= 9'd0;
		guesspos[1] <= 9'd0;
		guesspos[2] <= 9'd0;
		guesspos[3] <= 9'd0;
		guesspos[4] <= 9'd0;
		guesspos[5] <= 9'd0;
		guesspos[6] <= 9'd0;
		guesspos[7] <= 9'd0;
		guesspos[8] <= 9'd0;
		guesspos[9] <= 9'd0;
		guesspos[10] <= 9'd0;
		guesspos[11] <= 9'd0;
		guesspos[12] <= 9'd0;
		guesspos[13] <= 9'd0;
		guesspos[14] <= 9'd0;
		guesspos[15] <= 9'd0;
	end
	
	//Characters for each digit
	wire[7:0] char00;
	wire[7:0] char01;
	wire[7:0] char02;
	wire[7:0] char03;
	wire[7:0] char04;
	wire[7:0] char05;
	wire[7:0] char06;
	wire[7:0] char07;
	wire[7:0] char08;
	wire[7:0] char09;
	wire[7:0] char10;
	wire[7:0] char11;
	wire[7:0] char12;
	wire[7:0] char13;
	wire[7:0] char14;
	wire[7:0] char15;

	//Flip guesses left to right so we can null terminate more efficiently
	assign guess = {	char00,
							(lmax >= 1) ? char01 : 8'd0,
							(lmax >= 2) ? char02 : 8'd0,
							(lmax >= 3) ? char03 : 8'd0,
							(lmax >= 4) ? char04 : 8'd0,
							(lmax >= 5) ? char05 : 8'd0,
							(lmax >= 6) ? char06 : 8'd0,
							(lmax >= 7) ? char07 : 8'd0,
							(lmax >= 8) ? char08 : 8'd0,
							(lmax >= 9) ? char09 : 8'd0,
							(lmax >= 10) ? char10 : 8'd0,
							(lmax >= 11) ? char11 : 8'd0,
							(lmax >= 12) ? char12 : 8'd0,
							(lmax >= 13) ? char13 : 8'd0,
							(lmax >= 14) ? char14 : 8'd0,
							(lmax >= 15) ? char15 : 8'd0
							};
		
	//Addresses for each digit in the memory banks
	wire[10:0] addr00;
	wire[10:0] addr01;
	wire[10:0] addr02;
	wire[10:0] addr03;
	wire[10:0] addr04;
	wire[10:0] addr05;
	wire[10:0] addr06;
	wire[10:0] addr07;
	wire[10:0] addr08;
	wire[10:0] addr09;
	wire[10:0] addr10;
	wire[10:0] addr11;
	wire[10:0] addr12;
	wire[10:0] addr13;
	wire[10:0] addr14;
	wire[10:0] addr15;
	
	//High order bits choose the charset, the rest are the digit
	assign addr00 = {charset, guesspos[16'd0][7:0]};
	assign addr01 = {charset, guesspos[16'd1][7:0]};
	assign addr02 = {charset, guesspos[16'd2][7:0]};
	assign addr03 = {charset, guesspos[16'd3][7:0]};
	assign addr04 = {charset, guesspos[16'd4][7:0]};
	assign addr05 = {charset, guesspos[16'd5][7:0]};
	assign addr06 = {charset, guesspos[16'd6][7:0]};
	assign addr07 = {charset, guesspos[16'd7][7:0]};
	assign addr08 = {charset, guesspos[16'd8][7:0]};
	assign addr09 = {charset, guesspos[16'd9][7:0]};
	assign addr10 = {charset, guesspos[16'd10][7:0]};
	assign addr11 = {charset, guesspos[16'd11][7:0]};
	assign addr12 = {charset, guesspos[16'd12][7:0]};
	assign addr13 = {charset, guesspos[16'd13][7:0]};
	assign addr14 = {charset, guesspos[16'd14][7:0]};
	assign addr15 = {charset, guesspos[16'd15][7:0]};
	
	//Overflow flags for carry computation
	//Let K be the max value of a digit.
	//If digits i...0 = K, digit i has a carry out.
	wire overflows[15:0];
	assign overflows[0] = (guesspos[16'd0] == charsetmax); 
	assign overflows[1] = (guesspos[16'd1] == charsetmax); 
	assign overflows[2] = (guesspos[16'd2] == charsetmax); 
	assign overflows[3] = (guesspos[16'd3] == charsetmax); 
	assign overflows[4] = (guesspos[16'd4] == charsetmax); 
	assign overflows[5] = (guesspos[16'd5] == charsetmax); 
	assign overflows[6] = (guesspos[16'd6] == charsetmax); 
	assign overflows[7] = (guesspos[16'd7] == charsetmax); 
	assign overflows[8] = (guesspos[16'd8] == charsetmax); 
	assign overflows[9] = (guesspos[16'd9] == charsetmax); 
	assign overflows[10] = (guesspos[16'd10] == charsetmax); 
	assign overflows[11] = (guesspos[16'd11] == charsetmax); 
	assign overflows[12] = (guesspos[16'd12] == charsetmax); 
	assign overflows[13] = (guesspos[16'd13] == charsetmax); 
	assign overflows[14] = (guesspos[16'd14] == charsetmax); 
	assign overflows[15] = (guesspos[16'd15] == charsetmax); 
	
	//Carry computation
	wire carries[15:0];
	assign carries[0] =	overflows[0];
	assign carries[1] =	overflows[0] && overflows[1];
	assign carries[2] =	overflows[0] && overflows[1] && overflows[2];
	assign carries[3] =	overflows[0] && overflows[1] && overflows[2] && overflows[3];
	assign carries[4] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4];
	assign carries[5] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5];
	assign carries[6] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6];
	assign carries[7] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7];
	assign carries[8] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8];
	assign carries[9] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9];
	assign carries[10] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10];
	assign carries[11] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10] && overflows[11];
	assign carries[12] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10] && overflows[11] &&
								overflows[12];
	assign carries[13] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10] && overflows[11] &&
								overflows[12] && overflows[13];
	assign carries[14] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10] && overflows[11] &&
								overflows[12] && overflows[13] && overflows[14];
	assign carries[15] =	overflows[0] && overflows[1] && overflows[2] && overflows[3] &&
								overflows[4] && overflows[5] && overflows[6] && overflows[7] &&
								overflows[8] && overflows[9] && overflows[10] && overflows[11] &&
								overflows[12] && overflows[13] && overflows[15];

	
	//Charset decoder tables
	CharsetTable Digits_00_01 (
		.clka(clk),
		.addra(addr00),
		.douta(char00),
		.clkb(clk),
		.addrb(addr01),
		.doutb(char01));
	CharsetTable Digits_02_03 (
		.clka(clk),
		.addra(addr02),
		.douta(char02),
		.clkb(clk),
		.addrb(addr03),
		.doutb(char03));
	CharsetTable Digits_04_05 (
		.clka(clk),
		.addra(addr04),
		.douta(char04),
		.clkb(clk),
		.addrb(addr05),
		.doutb(char05));
	CharsetTable Digits_06_07 (
		.clka(clk),
		.addra(addr06),
		.douta(char06),
		.clkb(clk),
		.addrb(addr07),
		.doutb(char07));
	CharsetTable Digits_08_09 (
		.clka(clk),
		.addra(addr08),
		.douta(char08),
		.clkb(clk),
		.addrb(addr09),
		.doutb(char09));
	CharsetTable Digits_10_11 (
		.clka(clk),
		.addra(addr10),
		.douta(char10),
		.clkb(clk),
		.addrb(addr11),
		.doutb(char11));
	CharsetTable Digits_12_13 (
		.clka(clk),
		.addra(addr12),
		.douta(char12),
		.clkb(clk),
		.addrb(addr13),
		.doutb(char13));
	CharsetTable Digits_14_15 (
		.clka(clk),
		.addra(addr14),
		.douta(char14),
		.clkb(clk),
		.addrb(addr15),
		.doutb(char15));
	
	always @(posedge clk) begin
	
		//RESET LOGIC
		if(reset) begin
		
			//Recompute charset length
			case(charset)
			3'd0:	charsetsize = 9'd26; 	//lowercase letters
			3'd1:	charsetsize = 9'd26;		//uppercase letters
			3'd2:	charsetsize = 9'd52;		//all letters
			3'd3:	charsetsize = 9'd62;		//all letters and numbers
			3'd4:	charsetsize = 9'd94;		//all keyboard chars
			3'd5:	charsetsize = 9'd256;	//full ASCII
			endcase
			
			//Compute max charset index
			charsetmax <= charsetsize - 9'd1;
			
			//Copy guess length
			length <= guesslen;
			lmax <= guesslen - 4'd1;
			
			//Reset current guess
			guesspos[0] <= 9'd0;
			guesspos[1] <= 9'd0;
			guesspos[2] <= 9'd0;
			guesspos[3] <= 9'd0;
			guesspos[4] <= 9'd0;
			guesspos[5] <= 9'd0;
			guesspos[6] <= 9'd0;
			guesspos[7] <= 9'd0;
			guesspos[8] <= 9'd0;
			guesspos[9] <= 9'd0;
			guesspos[10] <= 9'd0;
			guesspos[11] <= 9'd0;
			guesspos[12] <= 9'd0;
			guesspos[13] <= 9'd0;
			guesspos[14] <= 9'd0;
			guesspos[15] <= 9'd0;
			
		end
		
		//Sequential counting logic
		//(carry lookahead adder)
		else begin
		
			//Set "done" flag
			done <= (guesspos[length] != 0);
			
			//first block has no carry in!
			if(carries[0])
				guesspos[0] <= 9'd0;
			else
				guesspos[0] <= guesspos[0] + 9'd1;
				
			if(carries[1])
				guesspos[1] <= 9'd0;
			else if(carries[0])
				guesspos[1] <= guesspos[1] + 9'd1;
			if(carries[2])
				guesspos[2] <= 9'd0;
			else if(carries[1])
				guesspos[2] <= guesspos[2] + 9'd1;
			if(carries[3])
				guesspos[3] <= 9'd0;
			else if(carries[2])
				guesspos[3] <= guesspos[3] + 9'd1;
			if(carries[4])
				guesspos[4] <= 9'd0;
			else if(carries[3])
				guesspos[4] <= guesspos[4] + 9'd1;
			if(carries[5])
				guesspos[5] <= 9'd0;
			else if(carries[4])
				guesspos[5] <= guesspos[5] + 9'd1;
			if(carries[6])
				guesspos[6] <= 9'd0;
			else if(carries[5])
				guesspos[6] <= guesspos[6] + 9'd1;
			if(carries[7])
				guesspos[7] <= 9'd0;
			else if(carries[6])
				guesspos[7] <= guesspos[7] + 9'd1;
			if(carries[8])
				guesspos[8] <= 9'd0;
			else if(carries[7])
				guesspos[8] <= guesspos[8] + 9'd1;
			if(carries[9])
				guesspos[9] <= 9'd0;
			else if(carries[8])
				guesspos[9] <= guesspos[9] + 9'd1;
			if(carries[10])
				guesspos[10] <= 9'd0;
			else if(carries[9])
				guesspos[10] <= guesspos[10] + 9'd1;
			if(carries[11])
				guesspos[11] <= 9'd0;
			else if(carries[10])
				guesspos[11] <= guesspos[11] + 9'd1;
			if(carries[12])
				guesspos[12] <= 9'd0;
			else if(carries[11])
				guesspos[12] <= guesspos[12] + 9'd1;
			if(carries[13])
				guesspos[13] <= 9'd0;
			else if(carries[12])
				guesspos[13] <= guesspos[13] + 9'd1;
			if(carries[14])
				guesspos[14] <= 9'd0;
			else if(carries[13])
				guesspos[14] <= guesspos[14] + 9'd1;
			if(carries[15])
				guesspos[15] <= 9'd0;
			else if(carries[14])
				guesspos[15] <= guesspos[15] + 9'd1;
			
		end
		
	end

endmodule
