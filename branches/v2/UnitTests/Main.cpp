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
* Main.cpp - unit tests for the cracker                                       *
*                                                                             *
******************************************************************************/
#include <config.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "Plugin.h"
using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		//Print some stuff
		cout << "" << endl;
		cout << "\033[1;35m#defines: ";
		#ifdef AMD64
			cout << " AMD64 = " << AMD64 ;
		#endif	
		
		#ifdef PS3
			cout << " PS3 = " << PS3;
		#endif	
		
		#ifdef X86
			cout << " X86 = " << X86;
		#endif	
		
		#ifdef UNIX
			cout << " UNIX = " << UNIX;
		#endif	
		
		#ifdef MACOSX
			cout << " MACOSX = " << MACOSX;
		#endif	
		
		#ifdef BSD
			cout << " BSD = " << BSD;
		#endif	
		
		#ifdef LINUX
			cout << " LINUX = " << LINUX;
		#endif	
		
		#ifdef WINDOWS
			cout << " WINDOWS = " << WINDOWS;
		#endif	

		cout	<< "\033[0m" << endl << endl;
		
		const int LIB_COUNT = 5;
		
		#ifdef CUDA_ENABLED
		const int ELIB_COUNT = LIB_COUNT;
		#else
		const int ELIB_COUNT = LIB_COUNT - 2;
		#endif
		
		#ifdef MACOSX
		const char* libnames[LIB_COUNT]=
		{
			"libcorehashes-generic.dylib",
			"libcracker-common.dylib",
			"libcrackthread-generic.dylib",
			
			"libcorehashes-cuda.dylib",
			"libcrackthread-cuda.dylib"
		};
		#elif defined(UNIX)
		const char* libnames[LIB_COUNT]=
		{
			"libcorehashes-generic.so",
			"libcracker-common.so",
			"libcrackthread-generic.so",
			
			"libcorehashes-cuda.so",
			"libcrackthread-cuda.so"
		};
		#elif defined(WINDOWS)
		const char* libnames[LIB_COUNT]=
		{
			"libcorehashes-generic.dll",
			"libcracker-common.dll",
			"libcrackthread-generic.dll",
			
			"libcorehashes-cuda.dll",
			"libcrackthread-cuda.dll"
		};		
		#endif
		
		for(int i=0; i<ELIB_COUNT; i++)
		{
			try
			{
				Plugin myplug(libnames[i]);
				PLUGINPROC runit = myplug.GetFunctionPtr("UnitTests");
				if(runit==NULL)
					cout << "\033[1;33mWARNING: No unit tests found in module " <<  libnames[i] << ".\033[0m" << endl << endl;
				else
					runit();
			}
			catch(string err)
			{
				cout << "\033[1;33mWARNING: could not load module " <<  libnames[i] << " (" << err.c_str() << ").\033[0m" << endl << endl;
			}
		}
		
		const int EXE_COUNT = 2;
		#if defined(MACOSX) || defined(UNIX)
		const char* exenames[EXE_COUNT]=
		{
			"./computenode",
			"./master"
		};
		#elif defined(WINDOWS)
		const char* exenames[EXE_COUNT]=
		{
			"computenode.exe",
			"master.exe"
		};
		#endif
		
		for(int i=0; i<EXE_COUNT; i++)
		{
			string str = string(exenames[i]) + " --unit-tests";
			system(str.c_str());
		}
	}
	catch(string err)
	{
		cerr << "ERROR: " << err.c_str();
	}
}
