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
* CrackMaster.h - master server global include                                *
*                                                                             *
******************************************************************************/

#ifndef CRACKMASTER_H
#define CRACKMASTER_H

#include "config.h"

#if WINDOWS
//This must be included before any file that includes windows.h.
# include <winsock2.h>

//Disable VS warnings for "deprecation" of standard C runtime functions like sprintf.
//They're portable, and safe as long as some basic precautions are followed.
# pragma warning(disable:4996)
#endif

#if UNIX
# include <sys/time.h>
#endif

#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <ctime>

#include "Mutex.h"
#include "../Cracker-common/Thread.h"
#include "../Cracker-common/NetworkSocket.h"

//Prototypes of thread procedures
THREAD_PROTOTYPE(ConnectionThread,_tid);
THREAD_PROTOTYPE(CommandLineThread,_notused);
THREAD_PROTOTYPE(SchedulerThread,_hashdata);

//A currently connected client
class WorkUnit;
class Client
{
public:
	Client(Thread* t=NULL,NetworkSocket* s=NULL, int i=-1)
	{ thread=t; sock=s; id=i; workUnit=NULL; }

	Thread* thread;			//The thread handling stuff for this client
	NetworkSocket* sock;	//The socket
	int id;					//Our client ID
	
	std::string hostname;	//Our hostname
	std::string platform;	//Architecture and platform
	
	int wusDone;			//Work units completed
	long long hashesDone;
	double lastWorkDone;	//last time we completed a WU

	WorkUnit* workUnit;		//Our current work unit
	
	std::string first;			//stats from our last WU
	std::string last;
};

//Data passed to the scheduler for a single crack
class CrackInfo
{
public:
	CrackInfo(std::string h,std::string a)
	{ hash=h; alg=a; }

	std::string hash;
	std::string alg;
};

//Class representing a work unit
class WorkUnit
{
public:
	std::string start;
	std::string end;
	long long size;		//not sent - stats only
};


//Define pointer-sized integer type
#if defined(AMD64)
#define intptr_t long long
#else
#define intptr_t int
#endif

//Define globals if not in main source file
#ifndef CRACKMASTER_CPP
extern Thread* g_pSched;
extern std::vector<WorkUnit*> g_workUnits;
extern std::list<Client*> g_idleClients;
extern Mutex g_idleClientMutex;
extern std::list<WorkUnit*> g_abortedWork;
extern Mutex g_abortedWorkMutex;	
extern bool g_bCracking;
extern double g_crackstart;
extern unsigned long long g_completedHashes;
extern int g_completedWorkUnits;
extern int g_allocatedWorkUnits;
extern Mutex g_statsMutex;
extern std::map<int,Client> g_clients;
extern int g_maxLength;
extern int g_nCharset;
extern int g_wusize;
extern std::map<int,long long> g_nodeHashesComplete; 
extern std::map<int,double> g_throughput;
#endif

void p_sleep(int ms);
double GetTime();

#endif
