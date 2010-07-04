/************************************************************************************
 * Password Cracker																	*
 *																					*
 * Copyright (c) 2008 RPISec														*
 * All rights reserved.																*
 *																					*
 * Redistribution and use in source and binary forms, with or without				*
 * modification, are permitted provided that the following conditions are met:		*
 *     *Redistributions of source code must retain the above copyright				*
 *      notice, this list of conditions and the following disclaimer.				*
 *     *Redistributions in binary form must reproduce the above copyright			*
 *      notice, this list of conditions and the following disclaimer in the			*
 *      documentation and/or other materials provided with the distribution.		*
 *     *Neither the name of RPISec nor the											*
 *      names of its contributors may be used to endorse or promote products		*
 *      derived from this software without specific prior written permission.		*
 *																					*
 * THIS SOFTWARE IS PROVIDED BY RPISec "AS IS" AND ANY								*
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED		*
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE			*
 * DISCLAIMED. IN NO EVENT SHALL RPISEC BE LIABLE FOR ANY							*
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES		*
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;		*
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND		*
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT		*
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS	*
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.						*
 ************************************************************************************
 *																					*
 * CrackThread.cpp - implementation of cracking thread								*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/07/2008		A. Zonenberg		Created initial version						*
 * 12/08/2008		A. Zonenberg		Performance testing							*
 * 12/10/2008		A. Zonenberg		Added second letter divisions				*
 * 12/12/2008		A. Zonenberg		Added parallel processing					*
 * 12/13/2008		A. Zonenberg		Minor tweaks to stats reporting				*
 ************************************************************************************
 */
#include "stdafx.h"
#define CRACK_CPP
#include "CrackThread.h"

#define PROFILING

UINT CrackThreadProc(LPVOID pData)
{
	CrackThreadData* pDat=reinterpret_cast<CrackThreadData*>(pData);
	
	//Cache some data
	HashingAlgorithm* pAlg=pDat->pAlg;
	int rangesize=(pDat->endchar+1)-pDat->startchar;
	int charmax=pDat->charset_len;
	char* chars=&pDat->charset[0];
	unsigned char* target=&pDat->target[0];
	int hashlen=pAlg->GetHashLength();

	//Crack variables
	unsigned char* test[4]={NULL};
	unsigned char* hash[4]={NULL};
	for(int i=0;i<4;i++)
	{
		test[i]=reinterpret_cast<unsigned char*>(_aligned_malloc(128,16));
		hash[i]=reinterpret_cast<unsigned char*>(_aligned_malloc(128,16));
	}
	
	//Calculate the total number of passwords we need to try
	__int64 total = 0;
	for(int len=pDat->minlen;len<=pDat->maxlen;len++)
	{
		//Calculate the number of N-digit passwords
		__int64 count = rangesize;
		for(int i=0;i<(len-1);i++)
			count*=charmax;

		//and mix in with the total
		total += count;
	}

	//Loop through target length values
	__int64 count=0;
	bool topquit=false;
	for(int len=max(3,pDat->minlen);len<=pDat->maxlen;len++)
	{
		if(topquit)
			break;

		//Loop through first digits
		int nums[30]={0};
		int second;
		for(int first=pDat->startchar;first<=pDat->endchar;first++)
		{
			test[0][0] = chars[first];
			test[1][0] = chars[first];
			test[2][0] = chars[first];
			test[3][0] = chars[first];
			if(topquit)
				break;

			//Loop through second digits
			//from (start1 start2) to (end1 end2).
			int s_end = pDat->charset_len - 1;
			if(first==pDat->startchar)
				second = pDat->startchar2;
			else
			{
				if(first==pDat->endchar)
					s_end=pDat->endchar2;
				second=0;
			}
			for(;second<=s_end;second++)
			{
				test[0][1] = chars[second];	
				test[1][1] = chars[second];	
				test[2][1] = chars[second];	
				test[3][1] = chars[second];	
				
				//Loop through subsequent digits
				bool quit=false;
				while(!quit && !topquit)
				{
					//Generate four target strings
					for(int q=0;q<4;q++)
					{
						for(int i=2;i<len;i++)
							test[q][i]=chars[nums[i]];
						test[q][len]='\0';
			
						//Bump digits with carry
						nums[len-1]++;
						for(int i=len-1;i>=0;i--)
						{
							if(i==1)	//Time for the next top-level loop
							{
								quit=true;
								break;
							}
							else if(nums[i]>=charmax)
							{
								nums[i]=0;
								nums[i-1]++;
							}
							else
								break;
						}

						if(quit)
							break;
					}

					//Hash it (TODO: add salt support)
					pAlg->Hash(
						&test[0],
						&hash[0],
						NULL,
						len);

					//See if we found it
					bool ok=true;
					for(int n=0;n<4;n++)
					{
						ok=true;
						for(int i=0;i<hashlen;i++)
						{
							if(hash[n][i] != target[i])
							{
								ok=false;
								break;
							}
						}
						
						if(ok)
						{
							memcpy(pDat->crackedhash,test[n],32);
							pDat->found=true;
							topquit=true;
							break;
						}
					}
					if(ok)
						break;

					//We did another hash! Update progress every 1k attempts
					count+=4;
					if(count % 4000 == 0)
					{
						pDat->progress=count * 100 / total;
						pDat->testcount = count;
						if(WaitForSingleObject(pDat->hQuitEvent,0)==WAIT_OBJECT_0)
						{
							//Time to quit
							return 0;
						}
					}
				}
			}
		}
	}

	pDat->testcount = count;

	for(int i=0;i<4;i++)
	{
		_aligned_free(hash[i]);
		_aligned_free(test[i]);
	}

	return 0;
}