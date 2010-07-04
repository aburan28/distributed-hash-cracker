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
* CoreHashes-generic.cpp - general exported functions                         *
*                                                                             *
******************************************************************************/

#include "CoreHashes-generic.h"
#include "MD5.h"
#include "SHA1.h"
#include "MD5crypt.h"
#include <memory.h>

//The null hashing algorithm: hashes everything to an 8-bit block of nulls.
//Used to allow performance profiling of guess-generation code.
class NullHash : public HashingAlgorithm
{
public:
	NullHash()
	{
		Hash = &NullHash::DoHash;
	}

	~NullHash()
	{}

	virtual const char* GetName()
	{ return "null"; }

	virtual int GetHashLength()
	{ return 1; }

	virtual int GetMaxInputSize()
	{ return 40; }

	virtual int GetVectorSize()
	{ return 1; }

	static void DoHash(unsigned char* in,    unsigned char* out, int len, unsigned char* salt, int saltlen)
	{ out[0]=0; }

	virtual bool IsGPUBased()
	{ return false; }
};



int GetHashCount()
{
	return 4;
}

HashingAlgorithm* GetHashAlg(int i)
{
	if(i==0)
		return new NullHash;
	else if(i==1)
	{
#if defined(X86) || defined(AMD64)
		return new MD5_SSE2;
#endif
		return new MD5_Portable;
	}
	else if(i==2)
	{
#ifdef X86
		//return new SHA1_SSE2;
#endif
		return new SHA1_Portable;
	}
	else if(i==3)
	{
#ifdef X86
		//return new MD5crypt_SSE2;
#endif
		return new MD5crypt_Portable;
	}
	else
		return NULL;
}
