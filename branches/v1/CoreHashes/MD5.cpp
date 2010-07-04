/************************************************************************************
 * Password Cracker																	*
 *																					*
 * Copyright (c) 2008 RPISec														*
 * All rights reserved.																*
 *																					*
 * Redistribution and use in source and binary forms, with or without				*
 * modification, are permitted provided that the following conditions are met:		*
 *     *Redistributions of source code must retain the above copyright				*
 *      notice, this list of conditions and the following disclaimer.				*
 *     *Redistributions in binary form must reproduce the above copyright			*
 *      notice, this list of conditions and the following disclaimer in the			*
 *      documentation and/or other materials provided with the distribution.		*
 *     *Neither the name of RPISec nor the											*
 *      names of its contributors may be used to endorse or promote products		*
 *      derived from this software without specific prior written permission.		*
 *																					*
 * THIS SOFTWARE IS PROVIDED BY RPISec "AS IS" AND ANY								*
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED		*
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE			*
 * DISCLAIMED. IN NO EVENT SHALL RPISEC BE LIABLE FOR ANY							*
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES		*
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;		*
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND		*
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT		*
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS	*
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.						*
 ************************************************************************************
 *																					*
 * MD5.cpp - implementation of the MD5 class										*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/07/2008		A. Zonenberg		Created initial version (using crypto++)	*
 * 12/09/2008		A. Zonenberg		Implemented MD5 in x86 assembly				*
 * 12/12/2008		A. Zonenberg		Implemented MD5 using SSE instructions		*
 * 01/19/2009		A. Zonenberg		Added FLOP counter							*
 ************************************************************************************
 */

#include "stdafx.h"
#include "hashexports.h"
#include "MD5.h"

char* MD5::GetName()
{
	static char name[]="MD5";
	return name;
}

bool MD5::RequiresSalt()
{
	return false;
}

int MD5::GetHashLength()
{
	return 16;
}

int MD5::GetFLOPs()
{
	return 628;
}

//This macro defines a single operation for MD5.
//It is to be called immediately after the F function has terminated.
//The result of F is expected to be available in xmm4.
//7 flops
#define sse_md5round_core(a,b,c,d,index,shamt,stepnum) __asm						\
	{																				\
		__asm movups		xmm5,		[x + 16*index]		/*prefetch x[i] */		\
		__asm paddd			a,			xmm4				/*a = a + F*/			\
		__asm paddd			a,			[t + 16*stepnum]	/*a = a + F + T[i]*/	\
		__asm paddd			a,			xmm5				/*a = a + F + T[i] + x[i]*/ \
																					\
		__asm movaps		xmm4,		a					/*rotate a left shamt bits*/	\
		__asm pslld			a,			shamt										\
		__asm psrld			xmm4,		(32-shamt)									\
		__asm orps			a,			xmm4										\
																					\
		__asm paddd			a,			b					/*a = ROTL(~~~,N) + b) */	\
	}

//#define F(b,c,d) (((b) & (c)) | (~(b) & (d)))
//10 flops
#define sse_md5round_f(a,b,c,d,index,shamt,stepnum) __asm							\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm andps			xmm4,		c					/*xmm4 = b & c*/		\
		__asm movaps		xmm5,		b					/*xmm5 = b*/			\
		__asm andnps		xmm5,		d					/*xmm5 = ~b & d*/		\
		__asm orps			xmm4,		xmm5				/*xmm4 = F*/			\
	}																				\
	sse_md5round_core(a,b,c,d,index,shamt,stepnum)

//#define G(b,c,d) (((b) & (d)) | (~(d) & (c)))
//10 flops
#define sse_md5round_g(a,b,c,d,index,shamt,stepnum) __asm							\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm andps			xmm4,		d					/*xmm4 = b & d*/		\
		__asm movaps		xmm5,		d					/*xmm5 = d*/			\
		__asm andnps		xmm5,		c					/*xmm5 = ~d & c*/		\
		__asm orps			xmm4,		xmm5				/*xmm4 = G*/			\
	}																				\
	sse_md5round_core(a,b,c,d,index,shamt,stepnum)

//#define H(b,c,d) ((b) ^ (c) ^ (d))
//9 flops
#define sse_md5round_h(a,b,c,d,index,shamt,stepnum) __asm							\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm xorps			xmm4,		c					/*xmm4 = b ^ c*/			\
		__asm xorps			xmm4,		d					/*xmm4 = H*/			\
	}																				\
	sse_md5round_core(a,b,c,d,index,shamt,stepnum)

//#define I(b,c,d) ((c) ^ ((b) | ~(d)))
//10 flops
#define sse_md5round_i(a,b,c,d,index,shamt,stepnum) __asm							\
	{																				\
		__asm movaps		xmm4,		d					/*xmm4 = d*/			\
		__asm xorps			xmm4,		all_ones			/*xmm4 = ~d*/			\
		__asm orps			xmm4,		b					/*xmm4 = b | ~d*/		\
		__asm xorps			xmm4,		c					/*xmm4 = I*/			\
	}																				\
	sse_md5round_core(a,b,c,d,index,shamt,stepnum)

//NOTES:
//	If length is >50 bytes, returns immediately with no change to outputs.
//	Input buffer must be at least 56 bytes in length.
void MD5::Hash(
		unsigned char** inbuf,
		unsigned char** outbuf,
		unsigned char** salt,
		int len)
{
	//////////////////////////////////////////////////////////////////////////////
	//CONSTANTS
	__declspec(align(16)) static const int init_a[4]={0x67452301,0x67452301,0x67452301,0x67452301};
	__declspec(align(16)) static const int init_b[4]={0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89};
	__declspec(align(16)) static const int init_c[4]={0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe};
	__declspec(align(16)) static const int init_d[4]={0x10325476,0x10325476,0x10325476,0x10325476};

	__declspec(align(16)) static const int all_ones[4]={0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

	__declspec(align(16)) static const unsigned __int32 t[256]=
	{
		//Round 1
		0xd76aa478,	0xd76aa478,	0xd76aa478,	0xd76aa478,	
		0xe8c7b756,	0xe8c7b756,	0xe8c7b756,	0xe8c7b756,	
		0x242070db, 0x242070db, 0x242070db, 0x242070db,
		0xc1bdceee, 0xc1bdceee, 0xc1bdceee, 0xc1bdceee, 

		0xf57c0faf, 0xf57c0faf, 0xf57c0faf, 0xf57c0faf, 
		0x4787c62a,	0x4787c62a,	0x4787c62a,	0x4787c62a,	
		0xa8304613,	0xa8304613,	0xa8304613,	0xa8304613,	
		0xfd469501, 0xfd469501, 0xfd469501, 0xfd469501, 

		0x698098d8,	0x698098d8,	0x698098d8,	0x698098d8,	
		0x8b44f7af,	0x8b44f7af,	0x8b44f7af,	0x8b44f7af,	
		0xffff5bb1,	0xffff5bb1,	0xffff5bb1,	0xffff5bb1,	
		0x895cd7be, 0x895cd7be, 0x895cd7be, 0x895cd7be, 

		0x6b901122, 0x6b901122, 0x6b901122, 0x6b901122, 
		0xfd987193,	0xfd987193,	0xfd987193,	0xfd987193,	
		0xa679438e,	0xa679438e,	0xa679438e,	0xa679438e,	
		0x49b40821, 0x49b40821, 0x49b40821, 0x49b40821, 


		//Round 2
		0xf61e2562, 0xf61e2562, 0xf61e2562, 0xf61e2562, 
		0xc040b340, 0xc040b340, 0xc040b340, 0xc040b340, 
		0x265e5a51, 0x265e5a51, 0x265e5a51, 0x265e5a51, 
		0xe9b6c7aa, 0xe9b6c7aa, 0xe9b6c7aa, 0xe9b6c7aa, 
		
		0xd62f105d, 0xd62f105d, 0xd62f105d, 0xd62f105d, 
		0x02441453, 0x02441453, 0x02441453, 0x02441453, 
		0xd8a1e681, 0xd8a1e681, 0xd8a1e681, 0xd8a1e681, 
		0xe7d3fbc8, 0xe7d3fbc8, 0xe7d3fbc8, 0xe7d3fbc8, 

		0x21e1cde6, 0x21e1cde6, 0x21e1cde6, 0x21e1cde6, 
		0xc33707d6, 0xc33707d6, 0xc33707d6, 0xc33707d6, 
		0xf4d50d87, 0xf4d50d87, 0xf4d50d87, 0xf4d50d87, 
		0x455a14ed, 0x455a14ed, 0x455a14ed, 0x455a14ed, 

		0xa9e3e905, 0xa9e3e905, 0xa9e3e905, 0xa9e3e905, 
		0xfcefa3f8, 0xfcefa3f8, 0xfcefa3f8, 0xfcefa3f8, 
		0x676f02d9, 0x676f02d9, 0x676f02d9, 0x676f02d9, 
		0x8d2a4c8a, 0x8d2a4c8a, 0x8d2a4c8a, 0x8d2a4c8a, 

		//Round 3
		0xfffa3942, 0xfffa3942, 0xfffa3942, 0xfffa3942, 
		0x8771f681, 0x8771f681, 0x8771f681, 0x8771f681, 
		0x6d9d6122, 0x6d9d6122, 0x6d9d6122, 0x6d9d6122, 
		0xfde5380c, 0xfde5380c, 0xfde5380c, 0xfde5380c, 

		0xa4beea44, 0xa4beea44, 0xa4beea44, 0xa4beea44, 
		0x4bdecfa9, 0x4bdecfa9, 0x4bdecfa9, 0x4bdecfa9, 
		0xf6bb4b60, 0xf6bb4b60, 0xf6bb4b60, 0xf6bb4b60, 
		0xbebfbc70, 0xbebfbc70, 0xbebfbc70, 0xbebfbc70, 

		0x289b7ec6, 0x289b7ec6, 0x289b7ec6, 0x289b7ec6, 
		0xeaa127fa, 0xeaa127fa, 0xeaa127fa, 0xeaa127fa, 
		0xd4ef3085, 0xd4ef3085, 0xd4ef3085, 0xd4ef3085, 
		0x04881d05, 0x04881d05, 0x04881d05, 0x04881d05, 

		0xd9d4d039, 0xd9d4d039, 0xd9d4d039, 0xd9d4d039, 
		0xe6db99e5, 0xe6db99e5, 0xe6db99e5, 0xe6db99e5, 
		0x1fa27cf8, 0x1fa27cf8, 0x1fa27cf8, 0x1fa27cf8, 
		0xc4ac5665, 0xc4ac5665, 0xc4ac5665, 0xc4ac5665, 

		//Round 4
		0xf4292244, 0xf4292244, 0xf4292244, 0xf4292244, 
		0x432aff97, 0x432aff97, 0x432aff97, 0x432aff97, 
		0xab9423a7, 0xab9423a7, 0xab9423a7, 0xab9423a7, 
		0xfc93a039, 0xfc93a039, 0xfc93a039, 0xfc93a039, 

		0x655b59c3, 0x655b59c3, 0x655b59c3, 0x655b59c3, 
		0x8f0ccc92, 0x8f0ccc92, 0x8f0ccc92, 0x8f0ccc92, 
		0xffeff47d, 0xffeff47d, 0xffeff47d, 0xffeff47d, 
		0x85845dd1, 0x85845dd1, 0x85845dd1, 0x85845dd1, 
		
		0x6fa87e4f, 0x6fa87e4f, 0x6fa87e4f, 0x6fa87e4f, 
		0xfe2ce6e0, 0xfe2ce6e0, 0xfe2ce6e0, 0xfe2ce6e0, 
		0xa3014314, 0xa3014314, 0xa3014314, 0xa3014314, 
		0x4e0811a1, 0x4e0811a1, 0x4e0811a1, 0x4e0811a1, 
		
		0xf7537e82, 0xf7537e82, 0xf7537e82, 0xf7537e82, 
		0xbd3af235, 0xbd3af235, 0xbd3af235, 0xbd3af235, 
		0x2ad7d2bb, 0x2ad7d2bb, 0x2ad7d2bb, 0x2ad7d2bb, 
		0xeb86d391, 0xeb86d391, 0xeb86d391, 0xeb86d391
	};

	//////////////////////////////////////////////////////////////////////////////
	//DATA

	//The four input buffers, interleaved together.
	__declspec(align(16)) __int32 x[64]={0};

	//Output buffers
	__declspec(align(16)) __int32 oa[4];
	__declspec(align(16)) __int32 ob[4];
	__declspec(align(16)) __int32 oc[4];
	__declspec(align(16)) __int32 od[4];

	//////////////////////////////////////////////////////////////////////////////
	//STEP 0: Initialization

	//Validate length
	if(len>50)
		return;

	//Copy input
	for(int i=0,j=0;j<len;i++,j+=4)
	{
		x[i*4]=reinterpret_cast<int*>(inbuf[0])[i];
		x[i*4 + 1]=reinterpret_cast<int*>(inbuf[1])[i];
		x[i*4 + 2]=reinterpret_cast<int*>(inbuf[2])[i];
		x[i*4 + 3]=reinterpret_cast<int*>(inbuf[3])[i];
	}

	//////////////////////////////////////////////////////////////////////////////
	//STEP 1: Append Padding Bits
	unsigned char* b=reinterpret_cast<unsigned char*>(&x[0]);
	int base=len & 0xFC;
	int off=len-base;
	int fbase = 4*base;
	b[fbase + off]=0x80;
	b[fbase + 4 + off]=0x80;
	b[fbase + 8 + off]=0x80;
	b[fbase + 12 + off]=0x80;
	off++;
	while(off % 4)
	{
		b[fbase+off]=0;
		b[fbase+off+4]=0;
		b[fbase+off+8]=0;
		b[fbase+off+12]=0;
		off++;
	}
	
	//////////////////////////////////////////////////////////////////////////////
	//STEP 2: Append Length
	int fourn = len*8;
	x[56]=fourn;
	x[57]=fourn;
	x[58]=fourn;
	x[59]=fourn;

	__asm
	{	
		//////////////////////////////////////////////////////////////////////////////
		//STEP 3: Initialize Constants
		movaps				xmm0,			init_a
		movaps				xmm1,			init_b
		movaps				xmm2,			init_c
		movaps				xmm3,			init_d

		//////////////////////////////////////////////////////////////////////////////
		//STEP 4: MD5 Rounds
		
		//16 * 10 = 160 flops
		sse_md5round_f(xmm0,xmm1,xmm2,xmm3,0,7,0);
			sse_md5round_f(xmm3,xmm0,xmm1,xmm2,1,12,1);
			sse_md5round_f(xmm2,xmm3,xmm0,xmm1,2,17,2);
			sse_md5round_f(xmm1,xmm2,xmm3,xmm0,3,22,3);
		sse_md5round_f(xmm0,xmm1,xmm2,xmm3,4,7,4);
			sse_md5round_f(xmm3,xmm0,xmm1,xmm2,5,12,5);
			sse_md5round_f(xmm2,xmm3,xmm0,xmm1,6,17,6);
			sse_md5round_f(xmm1,xmm2,xmm3,xmm0,7,22,7);
		sse_md5round_f(xmm0,xmm1,xmm2,xmm3,8,7,8);
			sse_md5round_f(xmm3,xmm0,xmm1,xmm2,9,12,9);
			sse_md5round_f(xmm2,xmm3,xmm0,xmm1,10,17,10);
			sse_md5round_f(xmm1,xmm2,xmm3,xmm0,11,22,11);
		sse_md5round_f(xmm0,xmm1,xmm2,xmm3,12,7,12);
			sse_md5round_f(xmm3,xmm0,xmm1,xmm2,13,12,13);
			sse_md5round_f(xmm2,xmm3,xmm0,xmm1,14,17,14);
			sse_md5round_f(xmm1,xmm2,xmm3,xmm0,15,22,15);

		//16 * 10 = 160 flops
		sse_md5round_g(xmm0,xmm1,xmm2,xmm3,1,5,16);
			sse_md5round_g(xmm3,xmm0,xmm1,xmm2,6,9,17);
			sse_md5round_g(xmm2,xmm3,xmm0,xmm1,11,14,18);
			sse_md5round_g(xmm1,xmm2,xmm3,xmm0,0,20,19);
		sse_md5round_g(xmm0,xmm1,xmm2,xmm3,5,5,20);
			sse_md5round_g(xmm3,xmm0,xmm1,xmm2,10,9,21);
			sse_md5round_g(xmm2,xmm3,xmm0,xmm1,15,14,22);
			sse_md5round_g(xmm1,xmm2,xmm3,xmm0,4,20,23);
		sse_md5round_g(xmm0,xmm1,xmm2,xmm3,9,5,24);
			sse_md5round_g(xmm3,xmm0,xmm1,xmm2,14,9,25);
			sse_md5round_g(xmm2,xmm3,xmm0,xmm1,3,14,26);
			sse_md5round_g(xmm1,xmm2,xmm3,xmm0,8,20,27);
		sse_md5round_g(xmm0,xmm1,xmm2,xmm3,13,5,28);
			sse_md5round_g(xmm3,xmm0,xmm1,xmm2,2,9,29);
			sse_md5round_g(xmm2,xmm3,xmm0,xmm1,7,14,30);
			sse_md5round_g(xmm1,xmm2,xmm3,xmm0,12,20,31);

		//16 * 9 = 144 flops
		sse_md5round_h(xmm0,xmm1,xmm2,xmm3,5,4,32);
			sse_md5round_h(xmm3,xmm0,xmm1,xmm2,8,11,33);
			sse_md5round_h(xmm2,xmm3,xmm0,xmm1,11,16,34);
			sse_md5round_h(xmm1,xmm2,xmm3,xmm0,14,23,35);
		sse_md5round_h(xmm0,xmm1,xmm2,xmm3,1,4,36);
			sse_md5round_h(xmm3,xmm0,xmm1,xmm2,4,11,37);
			sse_md5round_h(xmm2,xmm3,xmm0,xmm1,7,16,38);
			sse_md5round_h(xmm1,xmm2,xmm3,xmm0,10,23,39);
		sse_md5round_h(xmm0,xmm1,xmm2,xmm3,13,4,40);
			sse_md5round_h(xmm3,xmm0,xmm1,xmm2,0,11,41);
			sse_md5round_h(xmm2,xmm3,xmm0,xmm1,3,16,42);
			sse_md5round_h(xmm1,xmm2,xmm3,xmm0,6,23,43);
		sse_md5round_h(xmm0,xmm1,xmm2,xmm3,9,4,44);
			sse_md5round_h(xmm3,xmm0,xmm1,xmm2,12,11,45);
			sse_md5round_h(xmm2,xmm3,xmm0,xmm1,15,16,46);
			sse_md5round_h(xmm1,xmm2,xmm3,xmm0,2,23,47);

		//16 * 10 = 160 flops
		sse_md5round_i(xmm0,xmm1,xmm2,xmm3,0,6,48);
			sse_md5round_i(xmm3,xmm0,xmm1,xmm2,7,10,49);
			sse_md5round_i(xmm2,xmm3,xmm0,xmm1,14,15,50);
			sse_md5round_i(xmm1,xmm2,xmm3,xmm0,5,21,51);
		sse_md5round_i(xmm0,xmm1,xmm2,xmm3,12,6,52);
			sse_md5round_i(xmm3,xmm0,xmm1,xmm2,3,10,53);
			sse_md5round_i(xmm2,xmm3,xmm0,xmm1,10,15,54);
			sse_md5round_i(xmm1,xmm2,xmm3,xmm0,1,21,55);
		sse_md5round_i(xmm0,xmm1,xmm2,xmm3,8,6,56);
			sse_md5round_i(xmm3,xmm0,xmm1,xmm2,15,10,57);
			sse_md5round_i(xmm2,xmm3,xmm0,xmm1,6,15,58);
			sse_md5round_i(xmm1,xmm2,xmm3,xmm0,13,21,59);
		sse_md5round_i(xmm0,xmm1,xmm2,xmm3,4,6,60);
			sse_md5round_i(xmm3,xmm0,xmm1,xmm2,11,10,61);
			sse_md5round_i(xmm2,xmm3,xmm0,xmm1,2,15,62);
			sse_md5round_i(xmm1,xmm2,xmm3,xmm0,9,21,63);

		//624 flops core plus four at end
		//628 flops total

		//////////////////////////////////////////////////////////////////////////////
		//STEP 5: Output

		//Add initial values back to registers
		paddd				xmm0,			init_a
		paddd				xmm1,			init_b
		paddd				xmm2,			init_c
		paddd				xmm3,			init_d

		//and move to output
		movaps				oa,				xmm0
		movaps				ob,				xmm1
		movaps				oc,				xmm2
		movaps				od,				xmm3
	}

	//Copy output buffers
	*reinterpret_cast<__int32*>(outbuf[0]+0) = oa[0];
	*reinterpret_cast<__int32*>(outbuf[1]+0) = oa[1];
	*reinterpret_cast<__int32*>(outbuf[2]+0) = oa[2];
	*reinterpret_cast<__int32*>(outbuf[3]+0) = oa[3];
	*reinterpret_cast<__int32*>(outbuf[0]+4) = ob[0];
	*reinterpret_cast<__int32*>(outbuf[1]+4) = ob[1];
	*reinterpret_cast<__int32*>(outbuf[2]+4) = ob[2];
	*reinterpret_cast<__int32*>(outbuf[3]+4) = ob[3];
	*reinterpret_cast<__int32*>(outbuf[0]+8) = oc[0];
	*reinterpret_cast<__int32*>(outbuf[1]+8) = oc[1];
	*reinterpret_cast<__int32*>(outbuf[2]+8) = oc[2];
	*reinterpret_cast<__int32*>(outbuf[3]+8) = oc[3];
	*reinterpret_cast<__int32*>(outbuf[0]+12) = od[0];
	*reinterpret_cast<__int32*>(outbuf[1]+12) = od[1];
	*reinterpret_cast<__int32*>(outbuf[2]+12) = od[2];
	*reinterpret_cast<__int32*>(outbuf[3]+12) = od[3];
}