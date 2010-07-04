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
* CrackThread.h - interface for cracking threads                              *
*                                                                             *
******************************************************************************/

#ifndef CRACKTHREAD_H
#define CRACKTHREAD_H

#include "config.h"

class CrackThreadData
{
public:
	CrackThreadData( int base )
		: start(base)
		, end(base)
	{
	}

	HashingAlgorithm* pAlg;
	std::string target;
	BaseNInteger start;
	BaseNInteger end;
	std::string charset;

	int tid;
	bool bFound;
	std::string crackval;
	bool* bDoneWithWU;
};

//List of potential crack thread implementations
//in order of increasing preference
enum CrackTypes
{
	CRACK_TYPE_CPU_GENERIC = 1,
	CRACK_TYPE_CPU_ASM = 2,
	CRACK_TYPE_GPU = 3
};

typedef bool (*INITIALIZEPROC)();
typedef void (*CLEANUPPROC)();
typedef int (*GETMAXTHREADSPROC)();
typedef int (*GETCRACKTYPEPROC)();
typedef ZTHREADPROC (*GETCOMPUTETHREADPROC)();

#endif