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
* CommandLine.cpp - command prompt                                            *
*                                                                             *
******************************************************************************/

#include "../Cracker-common/NetworkSocket.h"
#include <iostream>
#include <string>
#include <vector>
#include "../Cracker-common/Thread.h"
#include "CommandLine.h"
#include <time.h>

using namespace std;

extern bool g_bCracking;
extern bool g_bQuitting;

extern vector<float> g_percent;
extern time_t g_start;
extern long long g_wsize;

THREAD_PROTOTYPE(CommandLineThread,_pSock)
{
	while(true)
	{
		cout << "node $ ";

		//Get the command
		char cmd[80];
		cin.getline(cmd,80);
		string command(cmd);

		//Process it
		if(command=="exit")
		{
			g_bQuitting = true;
			if(g_bCracking)
				cout << "Quitting after current work unit is done" << endl;
			else
			{
				NetworkSocket* pSock = reinterpret_cast<NetworkSocket*>(_pSock);
				pSock->Send("leaving");
				exit(0);
			}
			break;
		}
		else if(command=="stats")
		{
			time_t t;
			time(&t);

			float sum=0;
			for(unsigned int i=0;i<g_percent.size();i++)
				sum+=g_percent[i];
			sum/=g_percent.size();

			long long hashes = static_cast<long long>(sum * g_wsize);
			int dt = static_cast<int>(t - g_start);

			cout << "Percent complete: " << sum*100 << endl;
			cout << "Hashes complete: " << hashes << endl;
			cout << "Speed: " << static_cast<float>(hashes) / (1E6 * dt) << " Mhashes/sec" << endl;
		}
		else if(command.find("help")!=string::npos)
		{
			cout << "Sorry, not implemented :P" << endl;
		}
	}

	THREAD_RETURN(0);
}
