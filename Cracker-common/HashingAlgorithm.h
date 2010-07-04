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
* HashingAlgorithm.h - declaration of HashingAlgorithm class                  *
*                                                                             *
******************************************************************************/

#ifndef HASHINGALGORITHM_H
#define HASHINGALGORITHM_H

#include "config.h"

typedef void (*HASHPROC)(unsigned char*,unsigned char*,int,unsigned char*,int);

//Base class definition
#include "Cracker-common.h"
class CRACKER_COMMON_EXPORT HashingAlgorithm
{
public:
	HashingAlgorithm();
	virtual ~HashingAlgorithm();

	virtual const char* GetName()=0;  //Returns the name of the hash (statically allocated buffer)
	virtual int GetHashLength()=0;    //Returns hash length in bytes

	virtual int GetMaxInputSize()=0;  //Returns the maximum legal length of a string to be hashed

	virtual int GetVectorSize()=0;    //Returns the (fixed) size of the input vector.

	virtual bool IsGPUBased()=0;	  //Returns true if we're GPU-based.

	//Hashes a vector of inputs to a vector of outputs.
	//Notes:
	//* The size of this vector is FIXED and may be determined by calling GetVectorSize().
	//* All input strings must be the same length, specified by len.
	//* Values are stored end-to-end in the vector with no padding between them.
	HASHPROC Hash;
};

typedef int (*GETHASHCOUNTPROC)();
typedef HashingAlgorithm* (*GETHASHALGPROC)(int);

#endif
