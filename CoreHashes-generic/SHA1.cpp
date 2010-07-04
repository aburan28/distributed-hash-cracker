/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v2.0                                               *
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
*******************************************************************************
*                                                                             *
* SHA1.cpp - CPU-based SHA-1 algorithm                                        *
*                                                                             *
******************************************************************************/

#include "CoreHashes-generic.h"
#include "../Cracker-common/HashingAlgorithm.h"
#include "SHA1.h"
#include <memory.h>
#include <string>
using namespace std;

const char* sha1_name = "sha1";

//Pull in architecture-specific stuff as needed
#ifdef X86
#include "SHA1_x86.h"
#endif

///////////////////////////////////////////////////////////////////////////////
//SHA1_Portable: Generic C++ implementation
SHA1_Portable::SHA1_Portable()
{
	Hash = &SHA1_Portable::DoHash;
}

SHA1_Portable::~SHA1_Portable()
{
}

//Informational callbacks
int SHA1_Portable::GetHashLength()
{
	return SHA1_HASHSIZE;
}

int SHA1_Portable::GetMaxInputSize()
{
	return SHA1_MAXINPUT;
}

const char* SHA1_Portable::GetName()
{
	return sha1_name;
}

int SHA1_Portable::GetVectorSize()
{
	//No parallelism at all. What do you expect from generic, portable, unoptimized code?
	return 1;
}

bool SHA1_Portable::IsGPUBased()
{
	return false;
}

#define sha1_f1(b,c,d) ( (b & c) | (~b & d) )
#define sha1_f2(b,c,d) (b ^ c ^ d)
#define sha1_f3(b,c,d) ( (b & c) | (b & d) | (c & d) )
#define sha1_f4(b,c,d) (b ^ c ^ d)

#define ROTL(a,shamt) (((a) << shamt) | ((a) >> (32-shamt)))

#define bswap(x) ( (x & 0xFF)<<24 | (x&0xFF00) << 8 | (x&0xFF0000) >> 8 | (x&0xFF000000) >> 24  )


void SHA1_Portable::DoHash(unsigned char *in, unsigned char *out, int len, unsigned char* salt, int saltlen)
{
	//Buffer consists of data, then 0x80 (1000 0000), then zeros, then block length as a 64 bit int.
	unsigned int w[80]={0};
	if(len>50)
		return;
	memcpy(w,in,len);
	reinterpret_cast<unsigned char*>(&w[0])[len]=0x80;
	for(int i=0;i<15;i++)	//convert to big endian
		w[i] = bswap(w[i]);
	w[15]=len*8;

	//Initialize later blocks of W
	for(int t=16;t<80;t++)
		w[t] = ROTL(w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16],1);

	//Initialize temp buffers
	unsigned int temp;
	unsigned int
		a=0x67452301,
		b=0xEFCDAB89,
		c=0x98BADCFE,
		d=0x10325476,
		e=0xC3D2E1F0;

	//SHA1 rounds
	int round=0;
	for(;round<20;round++)
	{
		//Round function
		temp=ROTL(a,5) + sha1_f1(b,c,d) + e + w[round] + 0x5A827999;
		
		//Update variables
		e=d;
		d=c;
		c=ROTL(b,30);
		b=a;
		a=temp;
	}
	for(;round<40;round++)
	{
		//Round function
		temp=ROTL(a,5) + sha1_f2(b,c,d) + e + w[round] + 0x6ED9EBA1;
		
		//Update variables
		e=d;
		d=c;
		c=ROTL(b,30);
		b=a;
		a=temp;
	}
	for(;round<60;round++)
	{
		//Round function
		temp=ROTL(a,5) + sha1_f3(b,c,d) + e + w[round] + 0x8F1BBCDC;
		
		//Update variables
		e=d;
		d=c;
		c=ROTL(b,30);
		b=a;
		a=temp;
	}
	for(;round<80;round++)
	{
		//Round function
		temp=ROTL(a,5) + sha1_f4(b,c,d) + e + w[round] + 0xCA62C1D6;
		
		//Update variables
		e=d;
		d=c;
		c=ROTL(b,30);
		b=a;
		a=temp;
	}


	//Add in starting values and flip endianness
	a+=0x67452301,
	b+=0xEFCDAB89,
	c+=0x98BADCFE,
	d+=0x10325476,
	e+=0xC3D2E1F0;
	a=bswap(a);
	b=bswap(b);
	c=bswap(c);
	d=bswap(d);
	e=bswap(e);

	//Copy the output
	memcpy(out,&a,4);
	memcpy(out+4,&b,4);
	memcpy(out+8,&c,4);
	memcpy(out+12,&d,4);
	memcpy(out+16,&e,4);
}


///////////////////////////////////////////////////////////////////////////////
//SHA1_SSE2: SSE2 implementation that does four hashes in parallel.

//TODO: support amd64
#if (defined(X86) )

SHA1_SSE2::SHA1_SSE2()
{
//	Hash = ASMSYMBOL(SHA1SSE2Hash);
}

SHA1_SSE2::~SHA1_SSE2()
{
}

//Informational callbacks
int SHA1_SSE2::GetHashLength()
{
	return SHA1_HASHSIZE;
}

int SHA1_SSE2::GetMaxInputSize()
{
	return SHA1_MAXINPUT;
}

const char* SHA1_SSE2::GetName()
{
	return sha1_name;
}

int SHA1_SSE2::GetVectorSize()
{
	//SSE uses four dwords to a block. We're processing dwords
	//so we can do four hashes at once.
	return 4;
}

bool SHA1_SSE2::IsGPUBased()
{
	return false;
}

#endif //#if (defined(X86))
