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
* MD5crypt.h - CPU-based MD5 algorithm                                        *
*                                                                             *
******************************************************************************/

#ifndef MD5CRYPT_H
#define MD5CRYPT_H

#include "config.h"

//Constants that don't vary across implementations.
//If one implementation decides to break with these,
//then they should be hard coded and removed from this list.

//22 bytes in base64 format. TODO: decode base64 and call this 16 bytes
#define MD5CRYPT_HASHSIZE	22	
#define MD5CRYPT_MAXINPUT	50

//Generic C++ implementation.
//Slow but will compile for just about anything.
class MD5crypt_Portable : public HashingAlgorithm
{
public:
	MD5crypt_Portable();
	virtual ~MD5crypt_Portable();		//no destructor needed, just to make g++ shut up

	virtual const char* GetName();  //Returns the name of the hash (statically allocated buffer)
	virtual int GetHashLength();    //Returns hash length in bytes

	virtual int GetMaxInputSize();  //Returns the maximum legal length of a string to be hashed

	virtual int GetVectorSize();    //Returns the (fixed) size of the input vector.

	virtual bool IsGPUBased();		//Returns true if we're GPU based.

	//Do the hash
	static void DoHash(unsigned char* in, unsigned char* out, int len,unsigned char* salt, int saltlen);
};

#endif
