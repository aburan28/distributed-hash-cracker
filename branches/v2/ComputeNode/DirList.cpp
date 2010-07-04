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
* DirList.cpp - directory listing functions                                   *
*                                                                             *
*******************************************************************************/

#include "DirList.h"
using namespace std;

void DirListing(std::string dir,std::string ext,vector<string>& out)
{
#if WINDOWS
	
	//Set up the wildcard pattern properly
	if(dir=="" || dir==".")
		dir=string("*.")+ext;
	else
	{
		if(dir[dir.size()-1]!='\\')
			dir+="\\";
		dir+=string("*.")+ext;
	}
	
	//Do the listing
	WIN32_FIND_DATAA dat;
	HANDLE hFind = FindFirstFileA(dir.c_str(),&dat);
	if(hFind==INVALID_HANDLE_VALUE)
		throw string("FindFirstFile() failed for ")+dir;
	do
	{
		out.push_back(dat.cFileName);
	} while(FindNextFileA(hFind,&dat));

	//and clean up
	FindClose(hFind);

#elif UNIX

	//Open the dir
	DIR *dp;
	dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL)
		throw string("Failed to open directory");

	//Read files
	while ((dirp = readdir(dp)) != NULL)
	{
		//See if it matches
		string f(dirp->d_name);
		if(f.find(string(".")+ext) != string::npos)
			out.push_back(string(dirp->d_name));
	}
	closedir(dp);

#endif
}
