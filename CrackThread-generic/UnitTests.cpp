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
* UnitTests.cpp - unit tests for this module                                  *
*                                                                             *
******************************************************************************/
#include "config.h"
#include "CrackThread-generic.h"
#include "../Cracker-common/BaseNInteger.h"
#include "../Cracker-common/HashingAlgorithm.h"
#include "../ComputeNode/CrackThread.h"
#include <iostream>
using namespace std;

void ATest();
void XXTest();

#ifdef _MSC_VER
# ifdef CRACKTHREAD_GENERIC_EXPORTS
#  define CRACKTHREAD_GENERIC_EXPORT __declspec(dllexport)
#  define CRACKTHREAD_GENERIC_EXPORT_FUNC extern "C" __declspec(dllexport)
# else
#  define CRACKTHREAD_GENERIC_EXPORT __declspec(dllimport)
#  define CRACKTHREAD_GENERIC_EXPORT_FUNC extern "C" __declspec(dllimport)
# endif
#else //#ifdef _MSC_VER
# define CRACKTHREAD_GENERIC_EXPORT
# define CRACKTHREAD_GENERIC_EXPORT_FUNC
#endif

extern "C" CRACKTHREAD_GENERIC_EXPORT void UnitTests();

//The "A" hashing algorithm: hashes everything to "\x1"
class AHash : public HashingAlgorithm
{
public:
	AHash()
	{
		Hash = &AHash::DoHash;
	}

	~AHash()
	{}

	virtual const char* GetName()
	{ return "a"; }

	virtual int GetHashLength()
	{ return 1; }

	virtual int GetMaxInputSize()
	{ return 40; }

	virtual int GetVectorSize()
	{ return 1; }

	static void DoHash(unsigned char* in,    unsigned char* out, int len, unsigned char* salt, int saltlen)
	{	out[0] = 1; }

	virtual bool IsGPUBased()
	{ return false; }
};

//The XX hashing algorithm: hashes everything to "\0" except "xx", which hashes to "\x1"
class XXHash : public HashingAlgorithm
{
public:
	XXHash()
	{
		Hash = &XXHash::DoHash;
	}

	~XXHash()
	{}

	virtual const char* GetName()
	{ return "xx"; }

	virtual int GetHashLength()
	{ return 1; }

	virtual int GetMaxInputSize()
	{ return 40; }

	virtual int GetVectorSize()
	{ return 1; }

	static void DoHash(unsigned char* in,    unsigned char* out, int len, unsigned char* salt, int saltlen)
	{
		if(len==2 && in[0]=='x' && in[1]=='x')
			out[0] = 1;
		else
			out[0] = 0;
	}

	virtual bool IsGPUBased()
	{ return false; }
};

void UnitTests()
{
	cout << "\033[1;34mBeginning unit tests for module CrackThread-generic\033[0m" << endl;
	
	ATest();
	XXTest();
	
	cout << endl;
}

void ATest()
{
	cout << " * \"a\" test hash... ";
	
	//Create the params structure
	CrackThreadData* pData = new CrackThreadData(26);
	HashingAlgorithm* pAlg = new AHash;
	pData->pAlg = pAlg;
	pData->target = "01";
	pData->start.SetLength(1);
	pData->start.digits[0] = 0;
	pData->end.SetLength(1);
	pData->end.digits[0] = 25;	//go from a - z
	pData->charset = "abcdefghijklmnopqrstuvwxyz";
	pData->tid = 1;
	pData->bFound = false;
	bool bDoneWithWU = false;
	pData->bDoneWithWU = &bDoneWithWU;
	
	//Run a test crack
	Thread mythread(internalComputeThreadProc, pData);
	mythread.WaitUntilTermination();
	
	//See what comes out in the wash
	if(!pData->bFound)
		cout << "\033[1;31mFAILED (1)\033[0m" << endl;
	else if(pData->crackval != "a")
		cout << "\033[1;31mFAILED (2)\033[0m" << endl;
	else
		cout << "\033[32mpassed\033[0m" << endl;
	
	//Clean up
	delete pAlg;
	delete pData;
}

void XXTest()
{
	cout << " * \"xx\" test hash... ";
	
	//Create the params structure
	CrackThreadData* pData = new CrackThreadData(26);
	HashingAlgorithm* pAlg = new XXHash;
	pData->pAlg = pAlg;
	pData->target = "01";
	pData->start.SetLength(2);
	pData->start.digits[0] = 0;
	pData->start.digits[1] = 0;
	pData->end.SetLength(2);
	pData->end.digits[0] = 25;	//go from aa - zz
	pData->end.digits[1] = 25;
	pData->charset = "abcdefghijklmnopqrstuvwxyz";
	pData->tid = 1;
	pData->bFound = false;
	bool bDoneWithWU = false;
	pData->bDoneWithWU = &bDoneWithWU;
	
	//Run a test crack
	Thread mythread(internalComputeThreadProc, pData);
	mythread.WaitUntilTermination();
	
	//See what comes out in the wash
	if(!pData->bFound)
		cout << "\033[1;31mFAILED (1)\033[0m" << endl;
	else if(pData->crackval != "xx")
		cout << "\033[1;31mFAILED (2)\033[0m" << endl;
	else
		cout << "\033[32mpassed\033[0m" << endl;
	
	//Clean up
	delete pAlg;
	delete pData;
}
