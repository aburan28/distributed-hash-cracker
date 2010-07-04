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
* CommandLine.cpp - master server command line                                *
*                                                                             *
******************************************************************************/

#include "CrackMaster.h"
#include <iomanip>

using namespace std;

#define MILLION ((int)1E6)
#define BILLION ((int)1E9)

extern Mutex g_clientMutex;

THREAD_PROTOTYPE(CommandLineThread,_notused)
{
	while(true)
	{
		cout << "master $ ";

		//Get the command
		char cmd[80];
		cin.getline(cmd,80);
		string command(cmd);

		if(command=="cancel")
		{
			//Cancel the crack (compute nodes will finish current work units)
			g_bCracking = false;
		}
		else if(command=="clear")
		{
#if WINDOWS
			int unused =system("cls");
#elif UNIX
			int unused = system("clear");
#endif
			unused++;	//just so g++ doesnt complain
		}
		else if(command.find("crack")!=string::npos)
		{
			//Make sure we have a trailing space
			command+=' ';

			//Parse args
			vector<string> args;
			string temp;
			for(unsigned int i=6;i<command.length();i++)
			{
				//Space? Flush the command
				if(isspace(command[i]))
				{
					if(!temp.empty())
						args.push_back(temp);
					temp.clear();
				}

				//Add it
				else
					temp+=command[i];
			}

			if(args.size() < 2)
			{
				cout << "Usage: crack [alg] [hash]" << endl;
				continue;
			}

			//Make sure we're not already cracking
			if(g_bCracking)
			{
				cout << "The current crack must be finished before starting a new one." 
					<< endl;
				continue;
			}

			//Reset stats
			g_crackstart = GetTime();
			g_completedHashes=0;
			g_completedWorkUnits=0;
			g_allocatedWorkUnits=0;
			
			g_clientMutex.Lock();
				for(map<int,Client>::iterator it=g_clients.begin(); it!=g_clients.end(); it++)
				{
					Client& client = it->second;
					client.wusDone = 0;
					client.hashesDone = 0;
					client.lastWorkDone = GetTime();
					client.first="";
					client.last="";
				}
			g_clientMutex.Unlock();

			//Save status
			g_bCracking = true;
			//cout << "Cracking " << args[0].c_str() << " hash " << args[1].c_str() << endl;
			
			//Start the scheduler to do all the work
			g_pSched = new Thread(SchedulerThread,new CrackInfo(args[1],args[0]));
		}

		else if(command=="quit" || command=="exit")
		{
			//TODO: show stats

			//TODO: clean up
			exit(0);
		}

		else if(command.find("set")!=string::npos)
		{
			//Extract name and value
			string name;
			string val;
			unsigned int i=4;
			for(;i<command.length();i++)
			{
				if(isspace(command[i]))
					break;
				name+=command[i];
			}
			while(isspace(command[i]))
				i++;
			val=command.substr(i);

			//Process it
			if(name=="charset")
			{
				if(val == "1")
					g_nCharset = 0;
				else if(val=="a")
					g_nCharset = 1;
				else if(val=="A")
					g_nCharset = 2;
				else if(val=="aA")
					g_nCharset = 3;
				else if(val=="hex")
					g_nCharset = 4;
				else if(val=="base64")
					g_nCharset = 5;
				else if(val=="a1")
					g_nCharset = 6;
				else if(val=="A1")
					g_nCharset = 7;
				else if(val=="aA1")
					g_nCharset = 8;
				else if(val=="aA1!")
					g_nCharset = 9;
				else if(val=="all")
					g_nCharset = 10;
				else
					cout << "Unrecognized charset - must be one of " << endl <<
						"[aA1!]" << endl << " hex" << endl << " base64" << "all" << endl;
			}
			else if(name=="length")
			{
				int n = atoi(val.c_str());
				if(n<1 || n >= 50)
					cout << "Length must be between 1 and 50";
				else
					g_maxLength = n;
			}
			else if(name=="wusize")
			{
				float f = atof(val.c_str());
				if(f < 0.1 || f > 10000)
					cout << "Please enter work unit size, in millions" << endl;
				else
					g_wusize = static_cast<int>(1E6 * f);
			}
			else
				cout << "Unrecognized variable \"" << name.c_str() << "\"" << endl;
		}

		else if(command=="stats")
		{
			cout << g_clients.size() << " compute nodes online" << endl;

			if(g_bCracking)
			{
				//TODO: print time in hh::mm::ss
				double n = GetTime();
				double dt = n - g_crackstart;
				cout << "Elapsed time: " << dt << " seconds" << endl;

				double throughput = 0;
				for(map<int,Client>::iterator it=g_clients.begin(); it!=g_clients.end(); it++)
				{
					Client& c = it->second;
				
					//Calculate throughput
					double dt = c.lastWorkDone - g_crackstart;
					if(c.wusDone != 0)
						throughput += static_cast<double>(c.hashesDone) / dt;
				}
				
				throughput /= BILLION;
				
				
				//TODO: per-node throughput

				cout << "Work units allocated: " << g_allocatedWorkUnits << endl;
				cout << "Work units completed: " << g_completedWorkUnits << endl;
				cout << "Guesses tried: " << static_cast<double>(g_completedHashes) / BILLION << " billion" << endl;
				cout << "Throughput: " << throughput << " GHashes / sec" << endl;
			}
			else
			{
				cout << "No crack in progress";

				//TODO: Print stats of last completed / aborted crack
			}
			cout << endl << endl;
		}
		
		else if(command=="fullstats")
		{
			cout 
				<< endl
				<< setw(6) << "Node" 
				<< setw(24) << "Hostname"
				<< setw(16) << "Platform"
				<< setw(10) << "WUs done"
				<< setw(9) << "Status"
				<< setw(24) << "Current WU"
				<< setw(12) << "Speed"
				<< endl;
			for(map<int,Client>::iterator it=g_clients.begin(); it!=g_clients.end(); it++)
			{
				Client& c = it->second;
				
				//Calculate throughput
				double dt = c.lastWorkDone - g_crackstart;
				double hspeed = static_cast<double>(c.hashesDone) / (1000000 * dt);
				
				//Pretty-print setup
				string wu;
				string status;
				char speed[20];
				if(c.workUnit == NULL)
					status = "ready";
				else
					status = "busy";
				wu = c.first + " - " + c.last;
				if(c.wusDone == 0)
					sprintf(speed,"?.?? MHz");
				else
					sprintf(speed,"%.2lf MHz",hspeed);
				
				//Print stuff
				cout << setw(6) << c.id 
					<< setw(24) << c.hostname.c_str()
					<< setw(16) << c.platform.c_str()
					<< setw(10) << c.wusDone
					<< setw(9) << status.c_str()
					<< setw(24) << wu.c_str()
					<< setw(12) << speed
					<< endl;
			}
		}

		else if(command.find("help")!=string::npos)
		{
			//Display list of commands
			if(command=="help")
			{
				cout << "Available commands (type \"help XXX\" for details):" << endl
					<< "cancel" << endl
					<< "crack" << endl
					<< "exit" << endl
					<< "help" << endl
					<< "quit" << endl
					<< "set" << endl
					<< "stats" << endl;
			}

			else
			{
				//Help XXX - display help for XXX
				cout << "Not implemented!";
			}
		}

		else if(!command.empty())
			cout << "Unrecognized command (type \"help\" to see list)" << endl;
	}

	THREAD_RETURN(0);
}
