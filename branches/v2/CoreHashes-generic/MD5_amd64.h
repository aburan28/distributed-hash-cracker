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
* MD5_amd64.h - C++ bindings for amd64 assembly language MD5 hash             *
*                                                                             *
******************************************************************************/

#ifndef MD5_AMD64_H
#define MD5_AMD64_H

#include "config.h"

//Compiler-specific directive declarations:
//DOALIGN16 -> align the following variable on a 16-byte boundary and use C linkage
//ASMSYMBOL(x) -> either x or _x as appropriate
#ifdef _MSC_VER
# define DOALIGN16 extern "C" __declspec(align(16))
# define ASMSYMBOL(x) x
#elif defined(MACOSX)
# define DOALIGN16 __attribute__((aligned (16)))
# define ASMSYMBOL(x) x
#else
# define DOALIGN16 __attribute__((aligned (16)))
# define ASMSYMBOL(x) _ ## x
#endif 

DOALIGN16 int ASMSYMBOL(MD5init_a)[4]={0x67452301,0x67452301,0x67452301,0x67452301};
DOALIGN16 int ASMSYMBOL(MD5init_b)[4]={0xefcdab89,0xefcdab89,0xefcdab89,0xefcdab89};
DOALIGN16 int ASMSYMBOL(MD5init_c)[4]={0x98badcfe,0x98badcfe,0x98badcfe,0x98badcfe};
DOALIGN16 int ASMSYMBOL(MD5init_d)[4]={0x10325476,0x10325476,0x10325476,0x10325476};
DOALIGN16 int ASMSYMBOL(MD5all_ones)[4]={0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

DOALIGN16 unsigned int ASMSYMBOL(MD5tbuf)[256]=
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

extern "C" void ASMSYMBOL(MD5SSE2Hash)(unsigned char* in,    unsigned char* out, int len);

#endif //#define MD5_AMD64_H
