/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0 DEFCON EDITION                                *
*                                                                             *
* Copyright (c) 2009-2010 RPISEC.                                             *
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
*******************************************************************************/

#include "mpicrack.h"
#include <memory.h>
#include <string>
using namespace std;

//This is where the fun stuff starts...
#define F(X,Y,Z) (((X) & (Y)) | (~(X) & (Z)))
#define G(X,Y,Z) (((X) & (Z)) | ((Y) & ~(Z)))
#define H(X,Y,Z) ((X) ^ (Y) ^ (Z))
#define I(X,Y,Z) ((Y) ^ ((X) | ~(Z)))

#define md5round_f(a,b,c,d,k,s,i) md5round(a,b,c,d,k,s,i-1,F)
#define md5round_g(a,b,c,d,k,s,i) md5round(a,b,c,d,k,s,i-1,G)
#define md5round_h(a,b,c,d,k,s,i) md5round(a,b,c,d,k,s,i-1,H)
#define md5round_i(a,b,c,d,k,s,i) md5round(a,b,c,d,k,s,i-1,I)

#define ROTL(a,shamt) (((a) << (shamt)) | ((a) >> (32 - (shamt))))

#define md5round(a,b,c,d,k,s,i,f) a = ROTL(a + f(b,c,d) + x[k] + t[i],s) + b;

void MD5Hash(unsigned char *in, unsigned char *out, int len)
{
	//Buffer consists of data, then 0x80 (1000 0000), then zeros, then block length as a 64 bit int.
	int x[16]={0};
	if(len>50)
		return;
	memcpy(x,in,len);
	reinterpret_cast<unsigned char*>(&x[0])[len]=0x80;
	x[14]=len*8;

	//Initialize constants
	unsigned int
		A = 0x67452301,
		B = 0xefcdab89,
		C = 0x98badcfe,
		D = 0x10325476;

	//Round constants
	static const unsigned int t[64]=
	{
		//Round 1
		0xd76aa478,	0xe8c7b756,	0x242070db,	0xc1bdceee,
		0xf57c0faf, 0x4787c62a,	0xa8304613,	0xfd469501,
		0x698098d8,	0x8b44f7af,	0xffff5bb1,	0x895cd7be,
		0x6b901122, 0xfd987193,	0xa679438e,	0x49b40821,

		//Round 2
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

		//Round 3
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

		//Round 4
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	//Round 1
	md5round_f(A,B,C,D,0,7,1);	
		md5round_f(D,A,B,C,1,12,2);	
		md5round_f(C,D,A,B,2,17,3);
		md5round_f(B,C,D,A,3,22,4);
	md5round_f(A,B,C,D,4,7,5);	
		md5round_f(D,A,B,C,5,12,6);	
		md5round_f(C,D,A,B,6,17,7);
		md5round_f(B,C,D,A,7,22,8);
	md5round_f(A,B,C,D,8,7,9);
		md5round_f(D,A,B,C,9,12,10);
		md5round_f(C,D,A,B,10,17,11);
		md5round_f(B,C,D,A,11,22,12);
	md5round_f(A,B,C,D,12,7,13);
		md5round_f(D,A,B,C,13,12,14);
		md5round_f(C,D,A,B,14,17,15);
		md5round_f(B,C,D,A,15,22,16);

	//Round 2
	md5round_g(A,B,C,D,1,5,17);	
		md5round_g(D,A,B,C,6,9,18);	
		md5round_g(C,D,A,B,11,14,19);
		md5round_g(B,C,D,A,0,20,20);
	md5round_g(A,B,C,D,5,5,21);
		md5round_g(D,A,B,C,10,9,22);
		md5round_g(C,D,A,B,15,14,23);
		md5round_g(B,C,D,A,4,20,24);
	md5round_g(A,B,C,D,9,5,25);	
		md5round_g(D,A,B,C,14,9,26);
		md5round_g(C,D,A,B,3,14,27);
		md5round_g(B,C,D,A,8,20,28);
	md5round_g(A,B,C,D,13,5,29);	
		md5round_g(D,A,B,C,2,9,30);	
		md5round_g(C,D,A,B,7,14,31);
		md5round_g(B,C,D,A,12,20,32);

	//Round 3
	md5round_h(A,B,C,D,5,4,33);
		md5round_h(D,A,B,C,8,11,34);
		md5round_h(C,D,A,B,11,16,35);
		md5round_h(B,C,D,A,14,23,36);
	md5round_h(A,B,C,D,1,4,37);
		md5round_h(D,A,B,C,4,11,38);
		md5round_h(C,D,A,B,7,16,39);
		md5round_h(B,C,D,A,10,23,40);
	md5round_h(A,B,C,D,13,4,41);
		md5round_h(D,A,B,C,0,11,42);
		md5round_h(C,D,A,B,3,16,43);
		md5round_h(B,C,D,A,6,23,44);
	md5round_h(A,B,C,D,9,4,45);	
		md5round_h(D,A,B,C,12,11,46);
		md5round_h(C,D,A,B,15,16,47);
		md5round_h(B,C,D,A,2,23,48);

	//Round 4
	md5round_i(A,B,C,D,0,6,49);	
		md5round_i(D,A,B,C,7,10,50);
		md5round_i(C,D,A,B,14,15,51);
		md5round_i(B,C,D,A,5,21,52);
	md5round_i(A,B,C,D,12,6,53);
		md5round_i(D,A,B,C,3,10,54);
		md5round_i(C,D,A,B,10,15,55);
		md5round_i(B,C,D,A,1,21,56);
	md5round_i(A,B,C,D,8,6,57);	
		md5round_i(D,A,B,C,15,10,58);
		md5round_i(C,D,A,B,6,15,59);
		md5round_i(B,C,D,A,13,21,60);
	md5round_i(A,B,C,D,4,6,61);
		md5round_i(D,A,B,C,11,10,62);
		md5round_i(C,D,A,B,2,15,63);
		md5round_i(B,C,D,A,9,21,64);

	//Add in the original values
	A+=0x67452301;
	B+=0xefcdab89;
	C+=0x98badcfe;
	D+=0x10325476;

	//and output the result
	memcpy(out,&A,4);
	memcpy(out+4,&B,4);
	memcpy(out+8,&C,4);
	memcpy(out+12,&D,4);
}
