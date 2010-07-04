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
* CrackThread-generic.cpp - crack thread for generic systems that don't yet   *
*                           have optimized ASM builds                         *
*                                                                             *
******************************************************************************/

#include "CrackThread-generic.h"
#include "../Cracker-common/BaseNInteger.h"
#include "../Cracker-common/HashingAlgorithm.h"
#include "../ComputeNode/CrackThread.h"
#include <math.h>

using namespace std;

bool Initialize()
{
	//No init
	return true;
}

void Cleanup()
{
	//No cleanup needed
}

int GetCrackType()
{
	//We're a generic (non-ASM) CPU build
	return CRACK_TYPE_CPU_GENERIC;
}

int GetMaxThreads()
{
	//No limit on how many copies can be spawned
	return 99999;
}

ZTHREADPROC GetComputeThread()
{
	return internalComputeThreadProc;
}

THREAD_PROTOTYPE(internalComputeThreadProc,_pData)
{
	try
	{
		//Cache some parameters
		CrackThreadData* pData = reinterpret_cast<CrackThreadData*>(_pData);
		HashingAlgorithm* pAlg = pData->pAlg;
		string target=pData->target;
		BaseNInteger start=pData->start;
		BaseNInteger end=pData->end;
		string _charset = pData->charset;
		char charset[256];	//most we can possibly be
		if(_charset.length() > 256)
			throw string("Charset must be under 256 characters");
		for(unsigned int i=0;i<_charset.length();i++)
			charset[i]=_charset[i];
		long long gcount=0;
		long long smax = end.toInt() - start.toInt();
		int dlen = start.GetSize();
		HASHPROC DoHash = pAlg->Hash;
		volatile bool& bDoneWithWU = *pData->bDoneWithWU;

		//Get the size of the vectors we're passing to the function
		int vecsize = pAlg->GetVectorSize();
		unsigned int hashsize = static_cast<unsigned int>(pAlg->GetHashLength());

		//Allocate hash vectors
		int cdlen = dlen | 1;	//must be even
		unsigned char* guesses = new unsigned char[cdlen*vecsize];
		unsigned char* outs = new unsigned char[hashsize*vecsize];
		
		unsigned char* btarget = NULL;
		unsigned char* salt = NULL;
		int saltlen = 0;
		if(target[0] == '$')
		{
			//If it starts with $1$ parse out the salt
			//and put only the hash (unaltered) in btarget
			if(target[1]=='1' && target[2]=='$')
			{
				int len = target.length();
			
				//Allocate and read the salt
				salt = new unsigned char[16];
				memset(salt, 0, 16);
				int i=3;
				int j=0;
				while(i<len && target[i] != '$')
					salt[j++]=target[i++];
				salt[j++] = '\0';
					
				//Allocate and read the hash
				btarget = new unsigned char[25];
				memset(btarget, 0, 16);
				j=0; i++;
				while(i<len)
					btarget[j++]=target[i++];
				btarget[j++] = '\0';

				//Calculate salt length
				saltlen = strlen((char*)salt);
			}
			else
				throw string("Unrecognized hash format");
		}
		else
		{
			if(hashsize != target.length()/2)
			{
				//TODO: look into graceful termination
				//(i.e. drop this work unit)
				throw string("The supplied hash is not the correct size!");
			}
		
			//Convert target from hex to binary
			btarget = new unsigned char[hashsize];
			for(unsigned int i=0;i<hashsize;i++)
			{
				int b;
				sscanf(target.substr(i*2,2).c_str(),"%x",&b);
				btarget[i]=b;
			}
		}

		int hsize = pAlg->GetHashLength();

		//Generate the lookup table
		int clen = _charset.length();
		int dclen = 2*clen;
		char* tbl = new char[clen * clen * 2];
		for(int a=0;a<clen;a++)
		{
			char* row = tbl + (dclen * a);
			char ch = _charset[a];
			for(int b=0;b<clen;b++)
			{
				row[2*b] = ch;
				row[2*b + 1] = _charset[b];
			}
		}

		//Actual guess generation loop
		int digits[32] = {0};
		memcpy(digits,start.digits,dlen * sizeof(int));
		int guessbound = dlen - 1;
		unsigned short* stbl = reinterpret_cast<unsigned short*>(tbl);
		int hdlen = static_cast<int>(ceil(0.5f * dlen));
		unsigned short* base;
		while(gcount<=smax && !bDoneWithWU)
		{
			unsigned char* pguesses = guesses;
			for(int i=0;i<vecsize;i++)
			{
				//Copy to the buffer in words if possible, adding bytes if necessary.
				//Unfortunately, unaligned memory accesses dont seem to work well on Linux...
				if( static_cast<int>(reinterpret_cast<intptr_t>(pguesses)) & 1)
				{
					for(int j=0;j<dlen;j++)
						pguesses[j] = charset[digits[j]];
				}
				else
				{
					base = reinterpret_cast<unsigned short*>(pguesses);
					for(int j=0;j<hdlen;j++)
						base[j] = stbl[digits[j*2]*clen + digits[j*2 + 1]];
				}
				
				//Ripple carry add
				digits[guessbound]++;
				for(int k=guessbound;k>=0;k--)
				{
					if(digits[k] != clen)
						break;
					digits[k] = 0;
					if(k>0)
						digits[k-1]++;
				}

				pguesses += dlen;
			}

			gcount += vecsize;

			//Hash them
			DoHash(guesses,outs,dlen,salt,saltlen);
		
			//Test results
			//Use optimized comparisons whenever possible but don't die if we get fed something odd
			int nGuess=0;
			switch(hsize)
			{
			case 16:
				{
					for(nGuess=0;nGuess<vecsize;nGuess++)
					{
						int* base = reinterpret_cast<int*>(outs + (nGuess * hashsize));
						if(0 ==
							(
							  (base[0] ^ *reinterpret_cast<int*>(btarget   )) |
							  (base[1] ^ *reinterpret_cast<int*>(btarget+4 )) |
							  (base[2] ^ *reinterpret_cast<int*>(btarget+8 )) |
							  (base[3] ^ *reinterpret_cast<int*>(btarget+12)) )
						   )
						{
							//Save the string and quit the crack loop
							pData->crackval.clear();
							for(int q=dlen*nGuess;q<dlen*(nGuess + 1);q++)
								pData->crackval += static_cast<char>(guesses[q]);
							pData->bFound=true;
							bDoneWithWU=true;
							break;
						}
					}
				}
				break;
			default:
				{
					for(nGuess=0;nGuess<vecsize;nGuess++)
					{
						if(memcmp(btarget,outs+(hashsize * nGuess),hashsize) == 0)
						{
							//Save the string and quit the crack loop
							pData->crackval.clear();
							for(int q=dlen*nGuess;q<dlen*(nGuess + 1);q++)
								pData->crackval += static_cast<char>(guesses[q]);
							pData->bFound=true;
							bDoneWithWU=true;
							break;
						}
					}
				}
				break;
			}
		}

		//Clean up
		delete[] tbl;
		delete[] btarget;
		delete[] salt;
		delete[] guesses;
		delete[] outs;
	}
	catch(std::string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
		exit(1);
	}

	THREAD_RETURN(0);
}
