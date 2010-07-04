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
 * MD4.cpp - implementation of the MD4 class										*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/15/2008		A. Zonenberg		Created initial version						*
 * 01/19/2009		A. Zonenberg		Added FLOP counter							*
 ************************************************************************************
 */

#include "stdafx.h"
#include "hashexports.h"
#include "MD4.h"

char* MD4::GetName()
{
	static char name[]="MD4";
	return name;
}

bool MD4::RequiresSalt()
{
	return false;
}

int MD4::GetHashLength()
{
	return 16;
}

int MD4::GetFLOPs()
{
	return 436;
}

//This macro defines a single operation for MD4.
//It is to be called immediately after the F function has terminated.
//The result of F is expected to be available in xmm4.
//#define md4round_core(a,b,c,d,k,s,f,q) a = ROTL(a + f(b,c,d) + x[k] + q,s);
//5 flops
#define sse_md4round_core(a,b,c,d,index,shamt) __asm							\
	{																				\
		__asm movups		xmm5,		[x + 16*index]		/*prefetch x[i] */		\
		__asm paddd			a,			xmm4				/*a = a + F + rcon*/	\
		__asm paddd			a,			xmm5				/*a = a + F + rcon + x[i]*/ \
																					\
		__asm movaps		xmm4,		a					/*a = ROTL(~~~,shamt)*/	\
		__asm pslld			a,			shamt										\
		__asm psrld			xmm4,		(32-shamt)									\
		__asm orps			a,			xmm4										\
																					\
	}

//#define F(b,c,d) (((b) & (c)) | (~(b) & (d)))
//8 flops
#define sse_md4round_f(a,b,c,d,index,shamt) __asm									\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm andps			xmm4,		c					/*xmm4 = b & c*/		\
		__asm movaps		xmm5,		b					/*xmm5 = b*/			\
		__asm andnps		xmm5,		d					/*xmm5 = ~b & d*/		\
		__asm orps			xmm4,		xmm5				/*xmm4 = F*/			\
	}																				\
	sse_md4round_core(a,b,c,d,index,shamt)

//#define G(b,c,d) (((b) & (c)) | ((b) & (d)) | ((c) & (d)) )
//11 flops
#define sse_md4round_g(a,b,c,d,index,shamt) __asm									\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm andps			xmm4,		c					/*xmm4 = b & c*/		\
		__asm movaps		xmm5,		b					/*xmm5 = b*/			\
		__asm andps			xmm5,		d					/*xmm5 = b & d*/		\
		__asm orps			xmm4,		xmm5				/*xmm4 = bc | bd*/		\
		__asm movaps		xmm5,		c					/*xmm5 = c*/			\
		__asm andps			xmm5,		d					/*xmm5 = c & d*/		\
		__asm orps			xmm4,		xmm5				/*xmm4 = G*/			\
		__asm paddd			xmm4,		xmm6				/*xmm4 = G + rcon*/		\
	}																				\
	sse_md4round_core(a,b,c,d,index,shamt)

//#define H(b,c,d) ((b) ^ (c) ^ (d))	
//8 flops
#define sse_md4round_h(a,b,c,d,index,shamt) __asm									\
	{																				\
		__asm movaps		xmm4,		b					/*xmm4 = b*/			\
		__asm xorps			xmm4,		c					/*xmm4 = b ^ c*/		\
		__asm xorps			xmm4,		d					/*xmm4 = H*/			\
		__asm paddd			xmm4,		xmm7				/*xmm4 = H + rcon*/		\
	}																				\
	sse_md4round_core(a,b,c,d,index,shamt)


//NOTES:
//	If length is >50 bytes, returns immediately with no change to outputs.
//	Input buffer must be at least 56 bytes in length.
void MD4::Hash(
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

	//implicit rcon1 is all zeros and can be optimized away
	__declspec(align(16)) static const int rcon2[4]={0x5A827999,0x5A827999,0x5A827999,0x5A827999};
	__declspec(align(16)) static const int rcon3[4]={0x6ED9EBA1,0x6ED9EBA1,0x6ED9EBA1,0x6ED9EBA1};

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
	int* i1=reinterpret_cast<int*>(inbuf[0]);
	int* i2=reinterpret_cast<int*>(inbuf[1]);
	int* i3=reinterpret_cast<int*>(inbuf[2]);
	int* i4=reinterpret_cast<int*>(inbuf[3]);
	for(int i=0,j=0;j<len;i++,j+=4)
	{
		x[i*4]=i1[i];
		x[i*4 + 1]=i2[i];
		x[i*4 + 2]=i3[i];
		x[i*4 + 3]=i4[i];
	}

	//////////////////////////////////////////////////////////////////////////////
	//STEP 1: Append Padding Bits
	unsigned char* b=reinterpret_cast<unsigned char*>(&x[0]);
	int base=len & 0xFC;
	int off=len-base;
	b[4*base + off]=0x80;
	b[4*base + 4 + off]=0x80;
	b[4*base + 8 + off]=0x80;
	b[4*base + 12 + off]=0x80;
	off++;
	while(off % 4)
	{
		b[4*base+off]=0;
		b[4*base+off+4]=0;
		b[4*base+off+8]=0;
		b[4*base+off+12]=0;
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
		//STEP 4: MD4 Rounds

		//Cache round constants in SSE registers
		movaps				xmm6,			rcon2
		movaps				xmm7,			rcon3

		//Round 1
		//16 * 8 = 128 flops
		sse_md4round_f(xmm0,xmm1,xmm2,xmm3,0,3);
			sse_md4round_f(xmm3,xmm0,xmm1,xmm2,1,7);
			sse_md4round_f(xmm2,xmm3,xmm0,xmm1,2,11);
			sse_md4round_f(xmm1,xmm2,xmm3,xmm0,3,19);
		sse_md4round_f(xmm0,xmm1,xmm2,xmm3,4,3);
			sse_md4round_f(xmm3,xmm0,xmm1,xmm2,5,7);
			sse_md4round_f(xmm2,xmm3,xmm0,xmm1,6,11);
			sse_md4round_f(xmm1,xmm2,xmm3,xmm0,7,19);
		sse_md4round_f(xmm0,xmm1,xmm2,xmm3,8,3);
			sse_md4round_f(xmm3,xmm0,xmm1,xmm2,9,7);
			sse_md4round_f(xmm2,xmm3,xmm0,xmm1,10,11);
			sse_md4round_f(xmm1,xmm2,xmm3,xmm0,11,19);
		sse_md4round_f(xmm0,xmm1,xmm2,xmm3,12,3);
			sse_md4round_f(xmm3,xmm0,xmm1,xmm2,13,7);
			sse_md4round_f(xmm2,xmm3,xmm0,xmm1,14,11);
			sse_md4round_f(xmm1,xmm2,xmm3,xmm0,15,19);

		//Round 2
		//16 * 11 = 176 flops
		sse_md4round_g(xmm0,xmm1,xmm2,xmm3,0,3);
			sse_md4round_g(xmm3,xmm0,xmm1,xmm2,4,5);
			sse_md4round_g(xmm2,xmm3,xmm0,xmm1,8,9);
			sse_md4round_g(xmm1,xmm2,xmm3,xmm0,12,13);
		sse_md4round_g(xmm0,xmm1,xmm2,xmm3,1,3);
			sse_md4round_g(xmm3,xmm0,xmm1,xmm2,5,5);
			sse_md4round_g(xmm2,xmm3,xmm0,xmm1,9,9);
			sse_md4round_g(xmm1,xmm2,xmm3,xmm0,13,13);
		sse_md4round_g(xmm0,xmm1,xmm2,xmm3,2,3);
			sse_md4round_g(xmm3,xmm0,xmm1,xmm2,6,5);
			sse_md4round_g(xmm2,xmm3,xmm0,xmm1,10,9);
			sse_md4round_g(xmm1,xmm2,xmm3,xmm0,14,13);
		sse_md4round_g(xmm0,xmm1,xmm2,xmm3,3,3);
			sse_md4round_g(xmm3,xmm0,xmm1,xmm2,7,5);
			sse_md4round_g(xmm2,xmm3,xmm0,xmm1,11,9);
			sse_md4round_g(xmm1,xmm2,xmm3,xmm0,15,13);

		//Round 3
		//16 * 8 = 128 flops
		sse_md4round_h(xmm0,xmm1,xmm2,xmm3,0,3);
			sse_md4round_h(xmm3,xmm0,xmm1,xmm2,8,9);
			sse_md4round_h(xmm2,xmm3,xmm0,xmm1,4,11);
			sse_md4round_h(xmm1,xmm2,xmm3,xmm0,12,15);
		sse_md4round_h(xmm0,xmm1,xmm2,xmm3,2,3);
			sse_md4round_h(xmm3,xmm0,xmm1,xmm2,10,9);
			sse_md4round_h(xmm2,xmm3,xmm0,xmm1,6,11);
			sse_md4round_h(xmm1,xmm2,xmm3,xmm0,14,15);
		sse_md4round_h(xmm0,xmm1,xmm2,xmm3,1,3);
			sse_md4round_h(xmm3,xmm0,xmm1,xmm2,9,9);
			sse_md4round_h(xmm2,xmm3,xmm0,xmm1,5,11);
			sse_md4round_h(xmm1,xmm2,xmm3,xmm0,13,15);
		sse_md4round_h(xmm0,xmm1,xmm2,xmm3,3,3);
			sse_md4round_h(xmm3,xmm0,xmm1,xmm2,11,9);
			sse_md4round_h(xmm2,xmm3,xmm0,xmm1,7,11);
			sse_md4round_h(xmm1,xmm2,xmm3,xmm0,15,15);

		//////////////////////////////////////////////////////////////////////////////
		//STEP 5: Output

		//432 flops core + 4 final = 436 flops

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
	memcpy(outbuf[0]+0,&oa[0],4);
	memcpy(outbuf[1]+0,&oa[1],4);
	memcpy(outbuf[2]+0,&oa[2],4);
	memcpy(outbuf[3]+0,&oa[3],4);

	memcpy(outbuf[0]+4,&ob[0],4);
	memcpy(outbuf[1]+4,&ob[1],4);
	memcpy(outbuf[2]+4,&ob[2],4);
	memcpy(outbuf[3]+4,&ob[3],4);

	memcpy(outbuf[0]+8,&oc[0],4);
	memcpy(outbuf[1]+8,&oc[1],4);
	memcpy(outbuf[2]+8,&oc[2],4);
	memcpy(outbuf[3]+8,&oc[3],4);

	memcpy(outbuf[0]+12,&od[0],4);
	memcpy(outbuf[1]+12,&od[1],4);
	memcpy(outbuf[2]+12,&od[2],4);
	memcpy(outbuf[3]+12,&od[3],4);
}