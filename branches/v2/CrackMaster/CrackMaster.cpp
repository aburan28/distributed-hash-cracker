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
* CrackMaster.cpp - main file for master server                               *
*                                                                             *
******************************************************************************/

#define CRACKMASTER_CPP
#include "CrackMaster.h"
#include <cassert>

using namespace std;

//Set to true if we're currently cracking a hash
bool g_bCracking = false;

//Welcome banner with protocol version
string banner = "Master server, protocol v2.0";

//Sparse array of clients and sync mutexes. Index is connection ID.
map<int,Client> g_clients;
map<int,Mutex*> g_mutexes;
Mutex g_clientMutex;					//Global mutex used to sync mods to client list

//Crack data
Thread* g_pSched = NULL;				//The scheduler thread
list<Client*> g_idleClients;			//List of clients that say they're not doing anything
Mutex g_idleClientMutex;				//Mutex for syncing g_idleClients
list<WorkUnit*> g_abortedWork;			//List of work units that got interrupted
Mutex g_abortedWorkMutex;				//Mutext for syncing g_abortedWork

//Stats data
double g_crackstart;
//time_t g_lastwuFinish;
unsigned long long g_completedHashes;
int g_completedWorkUnits;
int g_allocatedWorkUnits;
map<int,long long> g_nodeHashesComplete; 
map<int,double> g_throughput;
Mutex g_statsMutex;

double GetTime();

void UnitTests();

const char* charsets[]=
{
	"0123456789",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	"0123456789abcdefABCDEF-",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/=",
	"0123456789abcdefghijklmnopqrstuvwxyz",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz !@#$%^&*()_+-=~`[]\\{}|;':\",./<>?",
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a"
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
		"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
		"\x2b\x2c\x2d\x2e\x2f\x3a\x3b\x3c\x3d\x3e\x3f\x40\x5b\x5c\x5d\x5e\x5f\x60"
		"\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c"
		"\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e"
		"\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0"
		"\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2"
		"\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4"
		"\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6"
		"\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8"
		"\xf9\xfa\xfb\xfc\xfd\xfe\xff"
};

int g_maxLength = 7;
int g_nCharset = 1;
int g_wusize = static_cast<int>(1E9);


//Entry point
int main(int argc,char* argv[])
{
	//Everything executed by the main thread is done within this try block.
	//All errors not caught elsewhere will throw an exception which will be
	//trapped here and result in program termination.
	try
	{
		//Run unit tests
		if(argc==2 && string(argv[1]) == "--unit-tests")
		{
			UnitTests();
			return 0;
		}
	
		//Version banner
		cout << "DISTRIBUTED HASH CRACKER: MASTER SERVER (built on " << __DATE__ << " at " << __TIME__ << ")" << endl
			<< "Copyright (c) 2009 RPISEC. Distributed under BSD license." << endl << endl;

		//Parse args
		string sdesc;
		for(int i=1;i<argc;i++)
		{
			string arg(argv[i]);
			//no recognized args atm
			//TODO: add port to listen on
			throw string("Unrecognized command-line argument ") + arg;
		}

		//Set up networking
		cout << "Initializing networking subsystem..." << endl;
		NetworkSocket::StaticInit();

		//RP xor IS xor ec
		//TODO: allow this to be overridden by cmdline arg
		unsigned short port = 0x52E3;
		cout << "Listening on port " << port << endl;

		//This socket is used to sit around and accept new connections.
		//Set it up and bind to our port.
		NetworkSocket listener(PF_INET,SOCK_STREAM,0);
		listener.Bind(port);

		//Put us in listening mode
		listener.Listen();

		//Print info
		cout << "Server ready, type \"exit\" to quit" << endl << endl;
		int id=0;

		//Start the command interpreter
		Thread cmdline(CommandLineThread,NULL);

		//Loop ad infinitum, accepting new connections.
		sockaddr_in client;
		int len=sizeof(client);
		while(NetworkSocket conn = listener.Accept(&client,len))
		{
			//Send banner
			conn.Send(banner);

			//Lock the thread so it will freeze as soon as it starts
			g_mutexes[++id]=new Mutex();
			g_mutexes[id]->Lock();

			//Spawn a thread to process the socket. Make sure we get write access
			//to the client list first, though!
			g_clientMutex.Lock();
				g_clients[id] = Client(
					new Thread(ConnectionThread,reinterpret_cast<void*>(id)),
					new NetworkSocket(conn.Detach()),
					id);
			g_clientMutex.Unlock();

			//Let the thread continue after we've set up its global data
			g_mutexes[id]->Unlock();

			//Get ready for another connection
			listener.Listen();
		}

		NetworkSocket::StaticCleanup();
	}
	catch(string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
	}
	return 0;
}

THREAD_PROTOTYPE(ConnectionThread,_tid)
{
	try
	{
		//Get our thread ID
		int tid = static_cast<int>(reinterpret_cast<intptr_t>(_tid));

		//Get hold of the mutex (once we do so, global data is ready for pickup)
		g_mutexes[tid]->Lock();

		//Pick up our global data
		g_clientMutex.Lock();
			Client& client = g_clients[tid];
		g_clientMutex.Unlock();

		//Cache some stuff
		NetworkSocket* pSocket = client.sock;
		Thread* pThread = client.thread;

		//Initialize stats
		g_statsMutex.Lock();
			g_nodeHashesComplete[tid]=0;
			g_throughput[tid]=0;
		g_statsMutex.Unlock();

		//Init - grab the client hello
		string hello = pSocket->Recv(64);
		//cout << tid << ": " << hello.c_str() << endl;
		if(hello.find("protocol v2.0") == string::npos)
		{
			//cout << tid << ": Client is running wrong protocol version, dropping connection\n";
			delete pSocket;
			delete client.thread;
			THREAD_RETURN(0);
		}
		
		//Parse the client hello
		size_t iHostname = hello.find(":",0);
		string hostname = "(unknown)";
		if(iHostname != string::npos)
			hostname = hello.substr(iHostname+2);
		if(hostname[hostname.length()-1]=='\n')
			hostname[hostname.length()-1] = '\0';
		client.hostname = hostname;
		string platform = "(unknown)";
		size_t iPlatform = hello.find(" ",0);
		if(iPlatform != string::npos)
			platform=hello.substr(0,iPlatform);
		client.platform=platform;
		
		//Set up a few more vars
		client.wusDone = 0;
		client.hashesDone = 0;
		client.lastWorkDone = GetTime();

		//Add us to the list of idle clients
		g_idleClientMutex.Lock();
			g_idleClients.push_back(&client);
		g_idleClientMutex.Unlock();

		//Handshake A-OK - sit around and wait for status reports to come in
		bool bNormalTerm=true;
		while(true)
		{
			//Get the update; check for error
			char buf[128];
			bool recv_err = false;
			int len=0;
			try
			{
				len = pSocket->Recv(buf,128);
			}
			catch(std::string err)
			{
				recv_err = true;
			}
			
			//Connection dropped? Handle this gracefully
			if(recv_err || len==0)
			{
				bNormalTerm=false;

				//Do we have an active work unit? If so, reallocate it to somebody else.
				if(client.workUnit!=NULL)
				{
					//cout << "Returning active work unit to pool.";
					g_abortedWorkMutex.Lock();
						g_abortedWork.push_back(client.workUnit);
					g_abortedWorkMutex.Unlock();
				}
				break;
			}

			//Null terminate the string
			buf[len]='\0';
			string sitrep(buf);

			//Save the time and update stats
			if(client.workUnit != NULL)
			{
				g_statsMutex.Lock();
					g_completedWorkUnits++;

					assert(client.workUnit->size > 0);
					assert(client.workUnit->size <= g_wusize);
					g_completedHashes += client.workUnit->size;
					g_nodeHashesComplete[tid] += client.workUnit->size;

					double temp=GetTime();
					g_throughput[tid] = static_cast<double>(g_nodeHashesComplete[tid]) / (temp - g_crackstart);
				g_statsMutex.Unlock();
			}

			//Process SITREPs
			if(sitrep=="continue")			//Done and ready for next work unit
			{
				//Make a note of it
				if(client.workUnit != NULL)
					client.hashesDone += client.workUnit->size;
				client.lastWorkDone = GetTime();
				client.workUnit=NULL;
				client.wusDone++;

				//We're done with the work unit - put us back in the queue for more work.
				g_idleClientMutex.Lock();
					g_idleClients.push_back(&client);
				g_idleClientMutex.Unlock();
			}
			else if(sitrep=="leaving")		//Done and quitting
			{
				//Make a note of it
				if(client.workUnit != NULL)
					client.hashesDone += client.workUnit->size;
				client.lastWorkDone = GetTime();
				client.workUnit=NULL;
				client.wusDone++;

				bNormalTerm=true;
				break;
			}
			else if(sitrep.find("found",0)==0)
			{
				//We found it!
				if(client.workUnit != NULL)	//technically should be a fraction of the WU
					client.hashesDone += client.workUnit->size;
				client.lastWorkDone = GetTime();
				client.workUnit=NULL;
				client.wusDone++;
	
				//Parse out the input string
				string val = sitrep.substr(6);

				//Print the hash value
				//TODO: optionally save to disk
				double n = GetTime();
				double dt = n - g_crackstart;
				cout << endl << "  Hash cracked (in " << dt << " sec) \"" << val << "\"" << endl << "master $ ";

				//Terminate the crack but do not drop the connection.
				//Let current clients finish their work units - no harm will be done.
				g_bCracking = false;

				//We're done with the work unit - put us back in the queue for the next crack
				g_idleClientMutex.Lock();
					g_idleClients.push_back(&client);
				g_idleClientMutex.Unlock();
			}
		}

		//Remove us from client list
		g_clientMutex.Lock();
			g_clients.erase(tid);
		g_clientMutex.Unlock();

		//Remove us from idle list if applicable
		g_idleClientMutex.Lock();
			g_idleClients.remove(&client);
		g_idleClientMutex.Unlock();

		//Free memory
		delete pSocket;
		delete pThread;
		THREAD_RETURN(0);
	}
	catch(string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
		exit(1);
	}

	THREAD_RETURN(0);
}


double GetTime()
{
#if LINUX
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;

#elif BSD
	timeval t;
	gettimeofday(&t, NULL);
	double d = static_cast<double>(t.tv_usec) / 1E6f;
	d += t.tv_sec;
	return d;

#elif WINDOWS
	static __int64 freq;
	static bool init=false;
	if(!init)
	{
		init=true;
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	}

	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);
	return static_cast<double>(t)/freq;
#endif
}
