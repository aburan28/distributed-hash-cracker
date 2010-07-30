/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
******************************************************************************/

/*!
	@file sha1_kernel_core.h
	
	@brief Shared stuff used by sha1Kernel() and sha1batchKernel()
 */
 
//////////////////////////////////////////////////////////////////////////////
//STEP 1: Copy buffer

//Main computation buffer
unsigned int w0=0;
unsigned int w1=0;
unsigned int w2=0;
unsigned int w3=0;
unsigned int w4=0;
unsigned int w5=0;
unsigned int w6=0;
unsigned int w7=0;
unsigned int w8=0;
unsigned int w9=0;
unsigned int w10=0;
unsigned int w11=0;
unsigned int w12=0;
unsigned int w13=0;
unsigned int w14=0;
unsigned int w15=(len+8)*8;
unsigned int w16=0;

//Scope temporary variables locally
{
	//Message data (deliberate fall-through)
	//Assume message is a multiple of 4, we will zero out extra bytes in the padding stage.
	//Ripple carry, etc is done in the InitGuess macros.
	unsigned int lstart0=0, lstart1=0, lstart2=0, lstart3=0;
	unsigned int lm1 = len-1;
	unsigned int lo4 = len/4;
	unsigned int carry = 0;		//no initial carry-in
	switch(lo4)
	{
	case 7:
		InitGuess(7, 31,30,29,28);
	case 6:
		InitGuess(6, 27,26,25,24);
	case 5:
		InitGuess(5, 23,22,21,20);
	case 4:
		InitGuess(4, 19,18,17,16);
	case 3:
		InitGuess(3, 15,14,13,12);
	case 2:
		InitGuess(2, 11,10,9,8);
	case 1:
		InitGuess(1, 7,6,5,4);
	case 0:
	default:
		InitGuess(0, 3,2,1,0);
	}
	
	//////////////////////////////////////////////////////////////////////////////
	//Append salt
	switch(len)
	{
	case 1:
		w0 = (w0 & 0x000000ff) | (gsalt[2] << 24) | (gsalt[1] << 16) | (gsalt[0] << 8);
		w1 = (gsalt[6] << 24) | (gsalt[5] << 16) | (gsalt[4] << 8) | (gsalt[3]);
		w2 = (gsalt[7]) | 0x00008000;
		break;
		
	case 2:
		w0 = (w0 & 0x0000ffff) | (gsalt[1] << 24) | (gsalt[0] << 16);
		w1 = (gsalt[5] << 24) | (gsalt[4] << 16) | (gsalt[3] << 8) | (gsalt[2]);
		w2 = (gsalt[7] << 8) | (gsalt[6]) | 0x00800000;
		break;
		
	case 3:
		w0 = (w0 & 0x00ffffff) | (gsalt[0] << 24);
		w1 = (gsalt[4] << 24) | (gsalt[3] << 16) | (gsalt[2] << 8) | (gsalt[1]);
		w2 = (gsalt[7] << 16) | (gsalt[6] << 8) | (gsalt[5]) | 0x80000000;
		break;
		
	case 4:
		w1 = (gsalt[3] << 24) | (gsalt[2] << 16) | (gsalt[1] << 8) | (gsalt[0]);
		w2 = (gsalt[7] << 24) | (gsalt[6] << 16) | (gsalt[5] << 8) | (gsalt[4]);
		w3 = 0x80;
		break;
	}
}

//Convert to big endian
w0 = bswap(w0);
w1 = bswap(w1);
w2 = bswap(w2);
w3 = bswap(w3);
w4 = bswap(w4);
w5 = bswap(w5);
w6 = bswap(w6);
w7 = bswap(w7);
w8 = bswap(w8);
w9 = bswap(w9);
w10 = bswap(w10);
w11 = bswap(w11);
w12 = bswap(w12);
w13 = bswap(w13);
w14 = bswap(w14);
//Don't bswap w15 (length) or w16 (not set yet)

//////////////////////////////////////////////////////////////////////////////
//STEP 3: Initialize Constants
unsigned int
	a=0x67452301,
	b=0xEFCDAB89,
	c=0x98BADCFE,
	d=0x10325476,
	e=0xC3D2E1F0;

//////////////////////////////////////////////////////////////////////////////
//STEP 4: SHA-1 Rounds
{
	unsigned int temp;

	//20 rounds of F1
	//0-15 are pre-bootstrapped
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w0 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w1 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w2 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w3 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w4 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w5 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w6 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w7 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w8 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w9 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w10 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w11 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w12 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w13 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w14 + 0x5A827999;	PostShift();
	temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w15 + 0x5A827999;	PostShift();
	//16-19 are normal
	RoundFromBlock1();
	RoundFromBlock1();
	RoundFromBlock1();
	RoundFromBlock1();

	//20 rounds of f2
	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();
	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();
	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();
	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();	RoundFromBlock2();

	//20 rounds of f3
	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();
	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();
	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();
	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	RoundFromBlock3();	
		
	//20 rounds of f4
	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();
	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();
	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();
	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();	RoundFromBlock4();
	
	//Add in starting values (don't flip endianness since target was flipped during init)
	a+=0x67452301,
	b+=0xEFCDAB89,
	c+=0x98BADCFE,
	d+=0x10325476,
	e+=0xC3D2E1F0;
}
