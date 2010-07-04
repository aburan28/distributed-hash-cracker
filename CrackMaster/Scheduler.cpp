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
* Scheduler.cpp - core scheduling infrastructure                              *
*                                                                             *
******************************************************************************/

#include "CrackMaster.h"
#include "../Cracker-common/BaseNInteger.h"
#include "Mutex.h"
using namespace std;

extern int g_maxLength;
extern int g_nCharset;
extern const char* charsets[];
extern int g_wusize;

string PascalEncode(string str);

THREAD_PROTOTYPE(SchedulerThread,_hashdata)
{
	try
	{
		CrackInfo* pInfo=reinterpret_cast<CrackInfo*>(_hashdata);

		string charset = charsets[g_nCharset];
		int charset_size = charset.size();

		
		int len=1;								//Length of our current candidate
		int maxlen=g_maxLength;					//The largest size string we're doing during this crack
		int wusize = g_wusize;					//Size of a work unit
		BaseNInteger startpos(charset_size);	//Position we're starting the next work unit at
		bool bAllAllocated=false;				//All WUs allocated?

		startpos.SetLength(len);				//Reset the start position

		//The actual crack loop...		
		while(g_bCracking)
		{
			//Prevent other threads from updating the idle list
			g_idleClientMutex.Lock();
			while(!g_idleClients.empty())
			{
				//Get the client
				Client* pClient = g_idleClients.front();

				//Get the work unit
				WorkUnit* wu=NULL;
				g_abortedWorkMutex.Lock();
					if(!g_abortedWork.empty())
					{
						//Somebody bailed on us - clean up the mess they left.
						wu=g_abortedWork.front();
						g_abortedWork.pop_front();
					}
					else if(!bAllAllocated)
					{
						//Get start of this work unit
						string start;
						for(int i=0;i<len;i++)
							start+=charset[startpos.digits[i]];

						//Bump the start position
						long long s = startpos.toInt();
						bool bDoneWithThisDigit = startpos.AddWithSaturation(wusize);
						long long alen = startpos.toInt() - s;	//actual work unit size

						g_statsMutex.Lock();
							g_allocatedWorkUnits++;
						g_statsMutex.Unlock();

						//Get end of this work unit
						string end;
						for(int i=0;i<len;i++)
							end+=charset[startpos.digits[i]];

						//Bump the start pos one more so we don't duplicate boundary guesses
						if(!bDoneWithThisDigit)
							bDoneWithThisDigit = startpos.AddWithSaturation(1);

						//Did we finish this digit?
						if(bDoneWithThisDigit)
						{
							//Are we done for good?
							if(len == maxlen)
								bAllAllocated=true;
							else
							{
								//No - get ready for the next digit.
								startpos.SetLength(++len);
							}
						}

						//Update the work unit
						wu=new WorkUnit;
						wu->start=start;
						wu->end=end;
						wu->size=alen;
					}
				g_abortedWorkMutex.Unlock();

				//If work unit is NULL then everything was allocated and nobody's abandoned
				//anything yet. Sit back and wait. If not, we have work to do - send it.
				if(wu!=NULL)
				{
					//Format work unit
					string netblob =
						string("CRACK ") + PascalEncode(pInfo->hash) + string("\n") +
						string("Algorithm: ") + PascalEncode(pInfo->alg) + string("\n") +
						string("Charset: ") + PascalEncode(charset) + string("\n") +
						string("Start-Guess: ") + PascalEncode(wu->start) + string("\n") +
						string("End-Guess: ") + PascalEncode(wu->end) + string("\n");

					//Send work unit to node
					pClient->sock->Send(netblob);

					//Bind this work unit to the current node
					pClient->workUnit = wu;
					pClient->first=wu->start;
					pClient->last=wu->end;

					//Remove from idle list
					g_idleClients.pop_front();
				}
			}
			g_idleClientMutex.Unlock();

			//Wait a while
			p_sleep(10);

			//If all work units have been allocated and completed... time to quit
			if(g_allocatedWorkUnits == g_completedWorkUnits && bAllAllocated)
				g_bCracking = false;
		}

		//print status
		cout << "Crack complete" << endl << "master $ ";

		//clean up and finish
		delete g_pSched;
		g_pSched = NULL;
	}

	catch(string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
		exit(1);
	}

	THREAD_RETURN(0);
}

void p_sleep(int ms)
{
#if WINDOWS
	Sleep(ms);
#elif UNIX
	usleep(1000 * ms);
#endif
}

//Encode a string in ASCII-style Pascal format: "3 abc".
//More formally, we have length as an ASCII format integer,
//followed by a space, then the actual string data.
string PascalEncode(string str)
{
	char lenbuf[64];	//more than long enough for any integer, even 64 bits
	sprintf(lenbuf,"%d",static_cast<int>(str.length()));

	return string(lenbuf) + string(" ") + str;
}
