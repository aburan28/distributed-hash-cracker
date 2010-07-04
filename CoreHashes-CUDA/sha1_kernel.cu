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
* sha1_kernel.cu - CUDA kernel for SHA-1                                      *
*                                                                             *
******************************************************************************/

#ifndef _SHA1_KERNEL_H_
#define _SHA1_KERNEL_H_

__shared__ int g_startguess[20];
__shared__ int g_charset[255];

#define sha1_f1(b,c,d) ( (b & c) | (~b & d) )
#define sha1_f2(b,c,d) (b ^ c ^ d)
#define sha1_f3(b,c,d) ( (b & c) | (b & d) | (c & d) )
#define sha1_f4(b,c,d) (b ^ c ^ d)

#define ROTL(a,shamt) (((a) << shamt) | ((a) >> (32-shamt)))

#define bswap(x) ( (x & 0xFF)<<24 | (x&0xFF00) << 8 | (x&0xFF0000) >> 8 | (x&0xFF000000) >> 24  )


////////////////////////////////////////////////////////////////////////////////
//CUDA kernel for cracking SHA1
////////////////////////////////////////////////////////////////////////////////
__global__ void sha1Kernel(int* startguess, unsigned char* target, unsigned char* charset,int csetsize, 
		unsigned char* output, int* bFound, int guesslen) 
{
	const unsigned int tid = blockDim.x*blockIdx.x + threadIdx.x;		//Cache thread ID
	
	//First node? Cache some stuff
	if(threadIdx.x == 0)
	{
		for(int i=0;i<guesslen;i++)
			g_startguess[i] = startguess[i];
			
		for(int i=0;i<csetsize;i++)
			g_charset[i] = charset[i];
	}
	__syncthreads();
	
	//Calculate the value we're hashing
	int pos[20];
	int guessend = guesslen-1;
	for(int i=0; i<guessend; i++)
		pos[i] = g_startguess[i];
	pos[guessend] = g_startguess[guessend] + tid;
	for(int i=guessend; i>=0; i--)
	{
		//No carry? Quit
		if(pos[i] < csetsize)
			break;
		
		//Split carry from digit
		int digit = pos[i] % csetsize;
		int carry = pos[i] - digit;
		
		//Save digit, push carry over
		pos[i] = digit;
		if(i >= 1)
			pos[i-1] += carry/csetsize;
	}

	//Buffer consists of data, then 0x80 (1000 0000), then zeros, then block length as a 64 bit int.
	unsigned int w[80];
	for(int j=0;j<80;j++)
		w[j]=0;
	unsigned char* p=(unsigned char*)&w[0];			//Byte ptr to buffer
	for(int j=0; j<guesslen; j++)
		p[j]=g_charset[pos[j]];
	reinterpret_cast<unsigned char*>(&w[0])[guesslen]=0x80;
	for(int i=0;i<15;i++)	//convert to big endian
		w[i] = bswap(w[i]);
	w[15]=guesslen*8;

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
	
	//Test the output
	int* pt = (int*)target;
	if(pt[0] != a)
		return;
	if(pt[1] != b)
		return;
	if(pt[2] != c)
		return;
	if(pt[3] != d)
		return;
	if(pt[4] != e)
		return;
	
	//If we get here, we must be a match! Save the result and quit
	*bFound = true;
	for(int j=0; j<guesslen; j++)
		output[j]=g_charset[pos[j]];
}

#endif // #ifndef _MD5_KERNEL_H_
