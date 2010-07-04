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
* ComputeNode.cpp - main file for compute node                                *
*                                                                             *
******************************************************************************/

#include "config.h"

//Turn off Visual Studio deprecation warnings
#if WINDOWS
#pragma warning(disable:4996)
#endif

//Enable performance profiling
#if WINDOWS
//# define PROFILING
#endif

//
#if BSD
# include <sys/types.h>
# include <sys/param.h>
# include <sys/sysctl.h>
#endif

#include "../Cracker-common/NetworkSocket.h"	//must be before anything that includes Windows.h on win32

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <ctime>
#include <cstdio>
#include <cassert>

#include "../Cracker-common/HashingAlgorithm.h"
#include "../Cracker-common/BaseNInteger.h"
#include "Plugin.h"
#include "DirList.h"
#include "../Cracker-common/Thread.h"
#include "CommandLine.h"
#include "CrackThread.h"

using namespace std;

bool g_bQuitting = false;
bool g_bCracking = false;

bool g_bDoneWithWU=false;

//Win32-specific timing code
#ifdef PROFILING
# if WINDOWS
#  define GetTimerTicks(num) QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&num));
#  define GetTimerSpeed(num) QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&num));
# else
#  error Profiling is currently only available for Win32.
# endif
float g_tGenerate;
float g_tHash;
float g_tTest;
float g_tSetup;
#endif

//Portable replacement for win32 Sleep()
void p_sleep(int ms);

//Read a Pascal-format string from a subset of an STL string
string GetPascalString(string& s,unsigned int& start);

//Parse a string into an arbitrary-base integer
//(base must be set to charset.length() before calling this function)
void ReadBaseN(BaseNInteger& i,string& str,string& charset);

//Returns number of CPUs on the system
int GetCPUCount();

#if LINUX
#define OPTERON
bool IsOpteron();
#endif

string GetHostname();

vector<float> g_percent;
long long g_wsize;
time_t g_start;

//File extension for hash plugins
#if WINDOWS
# define PLUGIN_EXT "dll"
#elif MACOSX
# define PLUGIN_EXT "dylib"
#elif UNIX
# define PLUGIN_EXT "so"
#endif

void UnitTests();

int main(int argc,char* argv[])
{
	try
	{
		//Run unit tests
		if(argc==2 && string(argv[1]) == "--unit-tests")
		{
			UnitTests();
			return 0;
		}
	
		//Show banner iff we're not the child of a fork
		if(argc>1 && string(argv[1]) != "--forkchild")
		{
			cout << "DISTRIBUTED HASH CRACKER: COMPUTE NODE (built on " << __DATE__ << " at " << __TIME__ << ")" << endl;
#ifdef PROFILING
			cout << "Note: This is an instrumented test build. Performance may suffer." << endl;
#endif
			cout << "Copyright (c) 2009 RPISEC. Distributed under BSD license." << endl << endl;
		}

		//Parse args
		int port=21219;
		string host;
		int nproc=GetCPUCount();
		bool bChild=false;					//set to true if we were forked off by a parent process
		bool bNoCuda = false;
		for(int i=1;i<argc;i++)
		{
			string arg(argv[i]);
			if(arg=="--server")
				host=argv[++i];
			else if(arg=="--forkchild")
				bChild=true;
			else if(arg=="-np")
				nproc=atoi(argv[++i]);
			else if(arg=="--nocuda")
				bNoCuda = true;
			else
				throw string("Unrecognized command-line argument ") + arg;
		}

		if(host.empty())
		{
			cout << "Usage: computenode --server [hostname or IP]" << endl;
			return 0;
		}
		
		#ifdef OPTERON
		//Are we on an Opteron? If so, fork off np processes for better performance
		if(IsOpteron())
		{
			//Are we a child? If not, we need to fork N-1 times
			if(!bChild)
			{
				cout << "Opteron processor detected, forking" << endl;
				for(int p=0; p<nproc-1; p++)
				{
					pid_t pid = fork();
					if(pid!=0)
					{
						//We got forked! Set up the args and execv()	
						//so we have a new socket, etc.
						vector<char*> args;
						args.push_back((char*)"computenode");	//cast to char* to eliminate g++ warning
						args.push_back((char*)"--forkchild");
						for(int i=1;i<argc;i++)
							args.push_back(argv[i]);
						args.push_back(NULL);
						execv("./computenode",&args[0]);						
						
						//Make sure we don't forkbomb if things go bad
						return 0;
					}				
				}
			}
			
			nproc = 1;	//Force only one compute thread
		}
		#endif

		//Load hash plugins
		cout << "Loading plugins..." << endl;
		vector<Plugin*> modules;
		vector<CLEANUPPROC> cleanups;
		map<string,HashingAlgorithm*> cpu_hashes;
		map<string,HashingAlgorithm*> gpu_hashes;
		vector<string> binaries;
		DirListing(".",PLUGIN_EXT,binaries);
		ZTHREADPROC ComputeThreadProc=NULL;
		int ComputeThreadCaps = -1;
		int MaxComputeThreads = 99999;
		bool use_gpu_hash=false;
		for(unsigned int b=0;b<binaries.size();b++)
		{
			//Load the binary
			Plugin* pPlug=new Plugin(binaries[b].c_str());
			if(pPlug==NULL)
				throw string("Memory allocation failed!");

			//See if it's a hash or thread file
			GETHASHCOUNTPROC GetHashCount=reinterpret_cast<GETHASHCOUNTPROC>(pPlug->GetFunctionPtr("GetHashCount"));
			GETHASHALGPROC GetHashAlg=reinterpret_cast<GETHASHALGPROC>(pPlug->GetFunctionPtr("GetHashAlg"));
			GETCOMPUTETHREADPROC GetComputeThread=
				reinterpret_cast<GETCOMPUTETHREADPROC>(pPlug->GetFunctionPtr("GetComputeThread"));
					
			if(GetHashCount != NULL && GetHashAlg != NULL)
			{
				//Yes - load each hash algorithm
				modules.push_back(pPlug);
				int n=GetHashCount();

				
				for(int i=0;i<n;i++)
				{
					HashingAlgorithm* pHash = GetHashAlg(i);
					if(pHash==NULL)
					{
						//TODO: decide if this should be a fatal error or not
						continue;
					}

					//Save it
					if(pHash->IsGPUBased())
					{
						if(bNoCuda)
							cout << "Algorithm " << pHash->GetName() << " [GPU] skipped" << endl;
						else
						{
							gpu_hashes[pHash->GetName()] = pHash;
							cout << "Algorithm " << pHash->GetName() << " [GPU] loaded" << endl;
						}
					}
					else
					{
						cpu_hashes[pHash->GetName()] = pHash;
						cout << "Algorithm " << pHash->GetName() << " [CPU] loaded" << endl;
					}
				}
			}
			else if(GetComputeThread != NULL)
			{
				//Get other functions
				INITIALIZEPROC init = reinterpret_cast<INITIALIZEPROC>(pPlug->GetFunctionPtr("Initialize"));
				CLEANUPPROC clean = reinterpret_cast<CLEANUPPROC>(pPlug->GetFunctionPtr("Cleanup"));
				GETMAXTHREADSPROC maxthreads = reinterpret_cast<GETMAXTHREADSPROC>(pPlug->GetFunctionPtr("GetMaxThreads"));
				GETCRACKTYPEPROC cracktype = reinterpret_cast<GETCRACKTYPEPROC>(pPlug->GetFunctionPtr("GetCrackType"));

				//Load the function
				modules.push_back(pPlug);
				cout << "Loading crack thread " << binaries[b].c_str() << endl;
				if(init())
				{
					cleanups.push_back(clean);

					//Get some capabilities from the module and decide which to use
					//TODO: save best CPU crack thread AND gpu crack thread
					//so that we can spawn some of each, as well as
					//doing CPU fallbacks on algorithms not supported in GPU
					int caps = cracktype();
					if(caps > ComputeThreadCaps)
					{
						if(bNoCuda && caps == CRACK_TYPE_GPU)
							cout << "Skipping GPU crack thread (remove --nocuda to enable)" << endl;
						else
						{
							ComputeThreadProc = GetComputeThread();
							ComputeThreadCaps = caps;
							MaxComputeThreads = maxthreads();
						}
					}
					
					if(caps == CRACK_TYPE_GPU)
						use_gpu_hash=true;
				}
				else
				{
					//Something went wrong - fall back to the previous, one if any
					cout << "\tFailed to properly initialize crack thread!" << endl;
				}
			}
			else
				delete pPlug;
		}

		//Make sure we have a valid crack thread
		if(ComputeThreadProc == NULL)
			throw string("No crack threads available");
		
		//TODO: Make sure we have at least one hash algorithm


		cout << endl << "Initializing networking subsystem..." << endl;
		NetworkSocket::StaticInit();

		//braces needed so that all sockets will go out of scope
		//before NetworkSocket::StaticCleanup() is called
		{
			//Connect to the server
			cout << "Connecting to " << host << ":" << port << "..." << endl;
			NetworkSocket sock(PF_INET,SOCK_STREAM,0);
			sock.Connect(host.c_str(),port);

			//Get the server banner
			string banner = sock.Recv(64);
			cout << banner.c_str() << endl << endl;
			if(banner.find("protocol v2.0") == string::npos)
				throw string("Server is running wrong protocol version!");
			
			//Send our banner
#if WINDOWS
			char siggy[64]="x86-Windows compute node, protocol v2.0";

			//TODO: 64 bit windows

#elif UNIX
#warning The signature is not correct for all platforms (Mac, BSD)
# ifdef PS3
			char siggy[64]="PS3-Linux compute node, protocol v2.0";
# elif defined(AMD64)
			char siggy[64]="amd64-Linux compute node, protocol v2.0";
#else
			char siggy[64]="x86-Linux compute node, protocol v2.0";
# endif
			
#endif
			//Set up the banner
			int remainder = 63-strlen(siggy);
			strncat(siggy, ": ",remainder);
			remainder-= 2;
			string host=GetHostname();
			strncat(siggy,host.c_str(),remainder);
			sock.Send(siggy,64);

			//Start command prompt
			Thread cmdPrompt(CommandLineThread,&sock);

			for(int i=0;i<nproc;i++)
				g_percent.push_back(0);

			//Main loop...
			while(true)
			{
				//Get a work unit
				char wu[512];
				int len=sock.Recv(wu,511);
				if(len==0)
				{
					throw string("Connection dropped");
				}
				wu[len]='\0';
				//cout << wu << endl;
				g_bCracking = true;
				time(&g_start);

#ifdef PROFILING
				g_tSetup = g_tGenerate = g_tHash = g_tTest = 0;
				long long setupStart;
				GetTimerTicks(setupStart);
#endif

				//Read verb and get target hash
				string w(wu);
				if(w.find("CRACK",0)!=0)
				{
					cout << "Invalid work unit";
					continue;
				}
				unsigned int s=6;	//index into work unit string
				string target = GetPascalString(w,s);
				s=w.find("\n",s);
				
				//Read the headers
				map<string,string> headers;
				while(true)
				{
					//Read the name, skipping the \n
					string::size_type np=w.find(":",++s);
					if(np==string::npos)
						break;
					if(s>=w.length())
						break;	
					string name = w.substr(s,np-s);
					s=np+2;

					//Read the value
					string value = GetPascalString(w,s);
					headers[name]=value;
				}

				//TODO: Make sure all mandatory headers are present.
				//Do sanity checks (start and end guess must be same length to avoid infinite loop)

				//Try to find the hash algorithm
				HashingAlgorithm* pAlg;
				if(use_gpu_hash)
					pAlg=gpu_hashes[headers["Algorithm"]];
				else
					pAlg=cpu_hashes[headers["Algorithm"]];
				if(pAlg==NULL)
				{
					//TODO: see if we can say "no thanks" and handle this gracefully while waiting for another hash
					throw string("Unable to find algorithm ") + headers["Algorithm"];
				}				

				//Pase start and end guess
				string charset=headers["Charset"];
				BaseNInteger start(charset.length());
				ReadBaseN(start,headers["Start-Guess"],charset);
				BaseNInteger end(charset.length());
				ReadBaseN(end,headers["End-Guess"],charset);
				g_wsize = end.toInt() - start.toInt();

				//Spawn compute threads, capping if necessary
				nproc = min(nproc, MaxComputeThreads);
#if WINDOWS
				if(IsDebuggerPresent())	//disable multithreading if being debugged in win32
					nproc=1;
#endif
				g_bDoneWithWU = false;	//not quitting yet...
				long long delta = ( end.toInt() - start.toInt() ) / nproc;		//Number of hashes per thread
				vector<Thread*> threads;
				vector<CrackThreadData*> crackdat;
				BaseNInteger t = start;
				for(int tid=0;tid<nproc;tid++)
				{
					//Set up the basic crack data
					CrackThreadData* pDat = new CrackThreadData(charset.size());
					pDat->pAlg = pAlg;
					pDat->bFound = false;
					pDat->target=target;
					pDat->tid=tid;
					pDat->charset=charset;
					pDat->bDoneWithWU = &g_bDoneWithWU;
					crackdat.push_back(pDat);

					//Allocate search space
					pDat->start=t;
					t.AddWithSaturation(delta);
					if(tid == (nproc-1))
						pDat->end=end;	//prevent round-off error from skipping last guess or two
					else
						pDat->end=t;

					//Create the thread and save the pointer
					threads.push_back(new Thread(ComputeThreadProc,pDat));
				}

#ifdef PROFILING
				long long setupEnd;
				GetTimerTicks(setupEnd);
				long long freq;
				GetTimerSpeed(freq);
				g_tSetup = static_cast<float>(setupEnd - setupStart) / freq;
#endif

				//Wait until the threads are done, check status of each
				bool bFound=false;
				string crackval;
				for(unsigned int tid=0;tid<threads.size();tid++)
				{
					threads[tid]->WaitUntilTermination();

					if(crackdat[tid]->bFound)
					{
						bFound=true;
						crackval=crackdat[tid]->crackval;
					}
				}

				//Print stats
#ifdef PROFILING
				float total = g_tSetup + g_tGenerate + g_tHash + g_tTest;
				printf(
					"\n\n"
					"Setup      : %.5f ms (%.2f %%)\n"
					"Generation : %.5f ms (%.2f %%)\n"
					"Hashing    : %.5f ms (%.2f %%)\n"
					"Testing    : %.5f ms (%.2f %%)\n",
					1000*g_tSetup, g_tSetup*100 / total,
					1000*g_tGenerate, g_tGenerate*100 / total,
					1000*g_tHash, g_tHash*100 / total,
					1000*g_tTest, g_tTest*100 / total);
#endif

				//Clean up
				for(unsigned int tid=0;tid<threads.size();tid++)
				{
					delete threads[tid];
					delete crackdat[tid];
				}

				//Report status
				if(bFound)
				{
					//Tell the boss we finished
					//TODO: Use Pascal string format for this
					sock.Send(string("found\n") + crackval);

					//Print status
					cout << endl << "Hash cracked: " << crackval.c_str() << endl << "node $ ";
				}
				else
				{
					//Tell the boss we finished
					if(g_bQuitting)
					{
						sock.Send("leaving");
						break;
					}
					else
						sock.Send("continue");

					//Print status
					//cout << "Work unit complete: no results found" << endl << endl;
				}

				g_bCracking = false;
			}
		}

		//Clean up crack threads
		for(unsigned int i=0;i<cleanups.size();i++)
			cleanups[i]();

		//Clean up hash algs
		for(map<string,HashingAlgorithm*>::iterator it=cpu_hashes.begin(); it!=cpu_hashes.end(); it++)
			delete it->second;
		for(map<string,HashingAlgorithm*>::iterator it=gpu_hashes.begin(); it!=gpu_hashes.end(); it++)
			delete it->second;
		cpu_hashes.clear();
		gpu_hashes.clear();
		for(unsigned int i=0;i<modules.size();i++)
			delete modules[i];
		modules.clear();

		NetworkSocket::StaticCleanup();
	}
	catch(string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
	}
	return 0;
}

string GetPascalString(string& s,unsigned int& start)
{
	//Get the length
	string::size_type lpos=s.find(" ",start);
	if(lpos==string::npos)
		return "";
	string slen=s.substr(start,lpos-start);
	start=lpos+1;
	int len;
	sscanf(slen.c_str(),"%d",&len);

	//Read that many characters
	string out = s.substr(start,len);
	start+=len;
	return out;
}

void ReadBaseN(BaseNInteger& i,string& str,string& charset)
{
	i.SetLength(str.length());

	//Read digits
	for(unsigned int n=0;n<str.length();n++)
	{
		char ch=str[n];
		for(unsigned int j=0;j<charset.length();j++)		//Search for the digit matching us.
		{													//We have to do exhaustive search because
															//charset isn't sorted.
			if(charset[j]==ch)
			{
				i.digits[n]=j;
				break;
			}
		}
	}
}


void p_sleep(int ms)
{
#if WINDOWS
	Sleep(ms);
#elif UNIX
	usleep(1000 * ms);
#endif
}

int GetCPUCount()
{
#ifdef UNIX
#warning See URL at the next line.
// http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
#endif

#if WINDOWS
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwNumberOfProcessors;
	
#elif PS3
	return 2;			//we have 6 SPEs and each crack thread needs three
	
#elif LINUX
#warning This is really stupid, Andrew.
	FILE* fp = popen("cat /proc/cpuinfo | grep processor | wc -l","r");
	char buf[127];
	if(NULL==fgets(buf,128,fp))
		return 1;
	pclose(fp);
	return atoi(buf);

#elif BSD
	int numCPU;
	size_t size = sizeof(numCPU);
	
	if(sysctlbyname("hw.ncpu", &numCPU, &size, NULL, 0))
		return 1;
	else
		return numCPU;

	return numCPU;
#endif
}

#ifdef OPTERON
bool IsOpteron()
{
	FILE* fp = popen("cat /proc/cpuinfo | grep Opteron | wc -l","r");
	char buf[128];
	if(NULL==fgets(buf,127,fp))
		return 1;
	pclose(fp);
	if(atoi(buf) >= 1)
		return true;	
	return false;
}
#endif

string GetHostname()
{
#if WINDOWS
	char buf[128];
	DWORD len=128;
	GetComputerNameA(buf,&len);
	return string(buf);
#elif UNIX
	char buf[128];
	FILE* fp = popen("hostname","r");
	if(NULL==fgets(buf,127,fp))
		return "box";
	pclose(fp);
	return string(buf);
#endif
}
