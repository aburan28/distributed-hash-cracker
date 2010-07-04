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
* CoreHashes-CUDA.h - interface code for CUDA-based hashes                    *
*                                                                             *
******************************************************************************/

#include "config.h"

#ifdef _MSC_VER
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED 
#endif

//pull in HashingAlgorithm
#include "../Cracker-common/HashingAlgorithm.h"

typedef void (*CUDA_HASHPROC)(int*, unsigned char*, unsigned char*,int, unsigned char*, int*,int, int,int);

//CUDA-based MD5
class MD5_CUDA : public HashingAlgorithm
{
public:	
	virtual const char* GetName();  //Returns the name of the hash (statically allocated buffer)
	virtual int GetHashLength();    //Returns hash length in bytes

	virtual int GetMaxInputSize();  //Returns the maximum legal length of a string to be hashed

	virtual int GetVectorSize();    //not used

	virtual bool IsGPUBased();		//Returns true if we're GPU based.

	MD5_CUDA();
	~MD5_CUDA();
};

//CUDA-based SHA1
class SHA1_CUDA : public HashingAlgorithm
{
public:	
	virtual const char* GetName();  //Returns the name of the hash (statically allocated buffer)
	virtual int GetHashLength();    //Returns hash length in bytes

	virtual int GetMaxInputSize();  //Returns the maximum legal length of a string to be hashed

	virtual int GetVectorSize();    //not used

	virtual bool IsGPUBased();		//Returns true if we're GPU based.

	SHA1_CUDA();
	~SHA1_CUDA();
};


void md5CudaHashProc(int* startguess, unsigned char* target, unsigned char* charset, int csetsize,
	unsigned char* output, int* bFound, int guesslen, int blocks, int threads);

void sha1CudaHashProc(int* startguess, unsigned char* target, unsigned char* charset, int csetsize,
	unsigned char* output, int* bFound, int guesslen, int blocks, int threads);
