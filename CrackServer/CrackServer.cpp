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
 * CrackServer.cpp - the crack server												*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/07/2008		A. Zonenberg		Created initial version						*
 * 12/10/2008		A. Zonenberg		Added speed calculation						*
 * 12/13/2008		A. Zonenberg		Added changeable port numbers				*
 * 01/19/2008		A. Zonenberg		Added flop count to stats					*
 * 01/20/2008		A. Zonenberg		Added support for really big charsets		*
 ************************************************************************************
 */

#include "stdafx.h"
#include "CrackServer.h"

//Hashing algorithms
void LoadHashPlugins();
vector<HMODULE> g_dlls;
vector<HashingAlgorithm*> g_algorithms;

//Cracking thread wrappers for beginthread()
void CallCrackThreadProc(void* pData);
void InfoThreadProc(void* pData);

//Set to signaled when it's time to stop
HANDLE g_hTermEvent;

//Status data
int g_percent;
__int64 g_hashesTried;
char g_cracked[32];
bool g_done;

//Networking
void ProcessConnectionRequest(SOCKET client,sockaddr_in client_addr);

//Effective hashes per ms, computed during startup
void BenchmarkCpuSpeed();
int g_hashingSpeed;

int main(int argc, char* argv[])
{
	//Bind the socket to port 0x7260 (29280) by default.
	//This is "RP" xor "IS" xor "ec".
	unsigned short portnum=0x52E3;

	if(argc==2)
	{
		//Read the second argument
		std::string str(argv[1]);
		if(str.find("-port=",0)!=0)
		{
			cout << "Usage: CrackServer [-port=####]";
			return 0;
		}

		//Process it
		std::string num = str.substr(6);
		portnum = atoi(num.c_str());
	}

	cout << "Distributed Password Crack Compute Node v1.0 build 10\n";
	cout << "Copyrght (c) 2008 RPISec. Distributed under BSD License.\n";
	cout << "\n\n";

	cout << "Loading hash plugins..." << endl;
	LoadHashPlugins();

	cout << "Benchmarking processor speed..." << endl;
	BenchmarkCpuSpeed();

	WSADATA wsdat;
	if(0!=WSAStartup(MAKEWORD(2,2),&wsdat))
	{
		cout << "Winsock init failed" << endl;
		exit(1);
	}

	//Open a socket
	SOCKET lsock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(lsock==INVALID_SOCKET)
	{
		cout << "Failed to open socket" << endl;
		exit(1);
	}

	hostent* localhost = gethostbyname("");
	char* localip = inet_ntoa (*(in_addr*)*localhost->h_addr_list);
	cout << "Current IP address is " << localip << endl;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(localip);
	addr.sin_port = htons(portnum);
	if(0!=bind(lsock,reinterpret_cast<sockaddr*>(&addr),sizeof(addr)))
	{
		cout << "Failed to bind socket" << endl;
		exit(-1);
	}
	char portnum_text[3]={portnum>>8,portnum&0xFF,0};

	//Put the socket into listening mode
	if(0!=listen(lsock,SOMAXCONN))
	{
		cout << "Failed to listen to socket" << endl;
		exit(-1);
	}
	cout << "Listening on port " << portnum << " (use -port=##### to change)" << endl << endl;

	//Print status and wait for connections
	cout << "Server running, press Ctrl-C to quit" << endl;
	SOCKET client;
	sockaddr_in client_addr;
	int addrlen=sizeof(client_addr);
	while(client=accept(lsock,reinterpret_cast<sockaddr*>(&client_addr),&addrlen))
	{
		//Process this connection
		ProcessConnectionRequest(client,client_addr);

		//Reset the buffer size
		addrlen=sizeof(client_addr);

		//and wait for the next connection
	}

	//Clean up
	WSACleanup();

	return 0;
}

void BenchmarkCpuSpeed()
{
	//Calculate as many MD5 hashes as possible in two seconds.

	//Find MD5
	HashingAlgorithm* pAlg=NULL;
	std::string algname="MD5";
	for(unsigned int i=0;i<g_algorithms.size();i++)
	{
		if(algname.compare(g_algorithms[i]->GetName())==0)
		{
			pAlg=g_algorithms[i];
			break;
		}
	}
	if(pAlg==NULL)
	{
		cout << "Error - cannot find core hash MD5" << endl;
		exit(1);
	}

	//Test data
	char* in[]=
	{
		"Hello World",
		"Hello World",
		"Hello World",
		"Hello World",
	};
	unsigned char* out[4];
	for(int i=0;i<4;i++)
		out[i]=new unsigned char[16];

	//Calculate as many 1000-hash blocks as possible in 1 second
	//to calculate effective hashes per millisecond.
	int hashes=0;
	DWORD stop=GetTickCount() + 1000;
	do
	{
		for(int i=0;i<1000;i++)
			pAlg->Hash(reinterpret_cast<unsigned char**>(&in[0]),out,NULL,12);
		hashes+=4;
	} while(GetTickCount() <= stop);

	for(int i=0;i<4;i++)
		delete[] out[i];

	//Convert to gflops
	float gflops = hashes * pAlg->GetFLOPs() / 1E6;

	//Multiply by core count
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	gflops *= info.dwNumberOfProcessors;
	
	g_hashingSpeed = hashes;
	cout << "Speed: " << g_hashingSpeed << " hashes/ms (" << gflops << " GFlops)" << endl << endl;
}

void ProcessConnectionRequest(SOCKET client,sockaddr_in client_addr)
{
	//We got a connection! Print out some info.
	cout << "Connection recieved from " << inet_ntoa(client_addr.sin_addr) << endl;
	
	Sleep(50);
	char banner[]="***RPISEC PASSWORD CRACKER***\n";
	send(client,banner,strlen(banner)+1,0);

	//Listen for data
	char command[512];
	int clen=0;
	while(0 != (clen=recv(client,(char*)&command,512,0)) )
	{
		if(clen == 0)
			break;

		if(clen > 500)
		{
			cout << "Invalid command length (over 500 bytes)" << endl;
			break;
		}
		command[clen]='\0';

		if(!strcmp(command,"STATS"))
		{
			cout << endl << "STATS command received" << endl;

			//Get CPU count
			SYSTEM_INFO info;
			GetSystemInfo(&info);

			char hostname[128];
			DWORD len = 128;
			GetComputerNameA(hostname,&len);

			//Format response
			char buf[512];
			sprintf_s(buf,512,"Host %s has %d CPUs (crack speed=%d)\n",hostname,info.dwNumberOfProcessors,
				g_hashingSpeed);
			puts(buf);

			//Send response
			send(client,buf,strlen(buf)+1,0);
		}
		else if(!strcmp(command,"CRACK"))
		{
			cout << endl << "CRACK command received" << endl;

			//Response values
			char ok=1;
			char err=0;

			//Acknowledge
			send(client,&ok,1,0);

			//Read charset and acknowledge
			char charset[256];
			int len=recv(client,charset,255,0);
			charset[len]='\0';
			cout << "Using charset " << charset << endl;
			send(client,&ok,1,0);

			//Read character range and acknowledge
			char range[128];
			len=recv(client,range,127,0);
			range[len]='\0';
			int start1,start2,end1,end2;
			sscanf_s(range,"%d %d - %d %d",&start1,&start2,&end1,&end2);
			cout << "Testing for passwords starting with " <<
				charset[start1] << charset[start2] << " - " <<
				charset[end1] << charset[end2] << endl;
			send(client,&ok,1,0);

			//Read length range and acknowledge
			len=recv(client,range,127,0);
			range[len]='\0';
			int minlen,maxlen;
			sscanf_s(range,"%d-%d",&minlen,&maxlen);
			cout << "Trying passwords between " << minlen << " and " << maxlen << " characters in length " << endl;
			send(client,&ok,1,0);

			//Read hashing algorithm
			char alg[128];
			len=recv(client,alg,127,0);
			alg[len]='\0';
			std::string algname(alg);
			cout << "Password is hashed with " << alg << endl;
			
			//Find the hashing algorithm and report back
			HashingAlgorithm* pAlg=NULL;
			for(unsigned int i=0;i<g_algorithms.size();i++)
			{
				if(algname.compare(g_algorithms[i]->GetName())==0)
				{
					pAlg=g_algorithms[i];
					cout << "  Found algorithm " << alg << "(" << pAlg->GetHashLength() << " byte output)" << endl;
					break;
				}
			}
			if(pAlg != NULL)
				send(client,&ok,1,0);
			else
				send(client,&err,1,0);

			//Read hash value and acknowledge
			char hash[128];
			len=recv(client,hash,128,0);
			cout << "Target hash value is ";
			for(int i=0;i<pAlg->GetHashLength();i++)
				printf("%02X",static_cast<int>(hash[i]) & 0xFF);
			cout << endl;
			send(client,&ok,1,0);

			//Get cracking!
			cout << endl << "STARTING CRACK" << endl;

			g_hTermEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
		
			//Divide starting-character ranges among threads.
			//If the size doesn't divide evenly the last thread will pick up the slack.
			SYSTEM_INFO info;
			GetSystemInfo(&info);
			//info.dwNumberOfProcessors = 3;

			vector<CrackThreadData*> crackdat;
			vector<HANDLE> threads;
			int charset_size = strlen(charset);
			int nrange = charset_size - start2;
			for(int i=0;i<(end1-start1);i++)
				nrange+=charset_size;
			nrange+=end2;
			int rpt=nrange / info.dwNumberOfProcessors;

			int sa=start1,sb=start2;
			int thread=0;
			for(unsigned int i=0;i<info.dwNumberOfProcessors;i++)
			{
				//Calculate range
				int ea = sa;
				int eb = sb + rpt;
				while(eb >= charset_size)
				{
					eb-=charset_size;
					ea++;
				}

				if(i==info.dwNumberOfProcessors - 1)
				{
					ea=end1;
					eb=end2;
				}
				
				//Set up the thread params
				CrackThreadData* dat=new CrackThreadData;
				strcpy_s(dat->charset,256,charset);
				dat->charset_len=strlen(charset);
				dat->startchar=sa;
				dat->startchar2=sb;
				dat->endchar=ea;
				dat->endchar2=eb;
				dat->minlen=minlen;
				dat->maxlen=maxlen;
				dat->progress=0;
				dat->pAlg=pAlg;
				dat->found=false;
				memset(dat->crackedhash,0,32);
				memcpy(dat->target,hash,128);
				dat->hQuitEvent=g_hTermEvent;

				//Print stats
				cout << "Thread " << thread++ << " gets "
					<< charset[dat->startchar] << charset[dat->startchar2] << " - "
					<< charset[dat->endchar] << charset[dat->endchar2] << endl;

				//Bump indices
				sb=eb + 1;
				sa=ea;
				if(sb >= charset_size)
				{
					sb-=charset_size;
					sa++;
				}

				//Start the thread
				threads.push_back(reinterpret_cast<HANDLE>(_beginthread(CallCrackThreadProc,0,dat)));
				crackdat.push_back(dat);
			}
		
			//We're now cracking. Sit back and wait for the threads to finish, accepting network queries
			//as needed.
			g_percent = 0;
			g_done = false;
			HANDLE hInfoThread = reinterpret_cast<HANDLE>(_beginthread(InfoThreadProc,0,(void*)client));
			while(WaitForMultipleObjects(threads.size(),&threads[0],TRUE,1000) == WAIT_TIMEOUT)
			{
				int p=0;

				//Update status
				__int64 t=0;
				for(unsigned int i=0;i<crackdat.size();i++)
				{
					//Bump percentage
					p+=crackdat[i]->progress;

					//Bump hash count
					t+=crackdat[i]->testcount;
				}

				//no critical section needed because write should be atomic
				//and it's perfectly fine if we get the old value
				g_percent = p / crackdat.size();
				g_hashesTried = t;
			}

			//Wait until we get a query for our value
			WaitForSingleObject(hInfoThread,INFINITE);

			//Done - clean up
			for(unsigned int i=0;i<crackdat.size();i++)
				delete crackdat[i];
			crackdat.clear();
		}
		else if(!strcmp(command,"QUIT"))
			break;

		ZeroMemory(command,sizeof(command));
	}

	closesocket(client);

	cout << "Connection terminated" << endl;
}

void LoadHashPlugins()
{
	//Find all DLLs
	WIN32_FIND_DATA dat;
	HANDLE hFind=FindFirstFile(_T("*.*"),&dat);
	do
	{
		std::string str(dat.cFileName);
		if(str.find(".dll",0) != string::npos)
		{
			//Load the DLL
			HMODULE hMod=LoadLibrary(str.c_str());

			//Get the number of hashing algorithms it contains
			GETALGCOUNT GetAlgCount = reinterpret_cast<GETALGCOUNT>(GetProcAddress(hMod,"GetAlgCount"));
			GETALGPTR GetAlgPtr = reinterpret_cast<GETALGPTR>(GetProcAddress(hMod,"GetAlgPtr"));
			if(GetAlgCount == NULL)
			{
				FreeLibrary(hMod);
				continue;
			}
			int algs=GetAlgCount();
			cout << str.c_str() << " contains " << algs << " hashing algorithms" << endl;

			//Instantiate them
			for(int i=0;i<algs;i++)
			{
				//Try to create an instance of the algorithm
				HashingAlgorithm* pAlg=GetAlgPtr(i);
				if(pAlg==NULL)
				{
					cout << "Failed to load algorithm " << i << " in " << str.c_str() << endl;
					continue;
				}

				//It's good - print stats and save it
				cout << " " << pAlg->GetName() << " successfully loaded" << endl;
				g_algorithms.push_back(pAlg);
			}

			//Save the handle
			g_dlls.push_back(hMod);
		}
	} while(FindNextFile(hFind,&dat));
	FindClose(hFind);

	//Error check
	if(g_algorithms.size()==0)
	{
		cout << endl << "No hashing algorithms found!" << endl;
		exit(-1);
	}

	cout << endl << endl;
}

void CallCrackThreadProc(void* pData)
{
	CrackThreadProc(pData);
	CrackThreadData* pd=(CrackThreadData*)(pData);	
	if(!pd->found)
	{
		cout << "Not found in " << pd->charset[pd->startchar] << pd->charset[pd->startchar2] << " - "
					<< pd->charset[pd->endchar] << pd->charset[pd->endchar2] << endl;
	}
	else
	{
		cout << "FOUND IT: " << pd->crackedhash << endl;
		memcpy(g_cracked,pd->crackedhash,32);
		SetEvent(g_hTermEvent);
		g_done=true;
	}
}

void InfoThreadProc(void* pData)
{
	//Respond to queries
	SOCKET client=(SOCKET)pData;
	char op;
	while(recv(client,&op,1,0))
	{
		//commands: stop, query done, query percent, query cracked value
		switch(op)
		{
		case CRACK_QUERY_PROGRESS:
			{
				unsigned char p=g_percent;
				send(client,(char*)&p,1,0);
				send(client,(char*)&g_hashesTried,8,0);
			}
			break;
		case CRACK_QUERY_ISDONE:
			{
				unsigned char done=0;
				if(g_done)
					done=1;
				send(client,(char*)&done,1,0);
			}
			break;
		case CRACK_QUERY_ABORT:
			cout << "Crack aborted" << endl;
			SetEvent(g_hTermEvent);
			break;
		case CRACK_QUERY_GETHASH:
			{
				cout << "Got hash query: " << g_cracked << endl;
				send(client,g_cracked,strlen(g_cracked),0);
				return;
			}
		}
	}
}