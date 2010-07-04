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
* NodeLauncher.cpp - batch launcher for compute nodes                         *
*                                                                             *
******************************************************************************/
#include <config.h>
#include "../Cracker-common/Thread.h"
#include "XmlParser.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

string g_host = "rpisec.net";

THREAD_PROTOTYPE(LaunchThread,_tag);

int main(int argc, char* argv[])
{
	XmlParser parser("computenodes.xml");
	const XmlNode* pRoot = parser.GetRoot();
	
	//Get server
	if(argc==2)
		g_host = argv[1];
	
	//Spawn the nodes
	vector<Thread*> threads;	
	cout << "Spawning compute nodes for server " << g_host.c_str() << "..." << endl;
	for(int i=0; i<pRoot->GetChildCount(); i++)
	{
		//Get the node
		const XmlNode* pNode = pRoot->GetChildNode(i);
		if(pNode->GetNodeType() != XmlNode::NODETYPE_TAG)
			continue;
			
		//Sanity check
		if(pNode->GetType() != "node")
		{
			cout << "Warning: skipping unrecognized tag \"" << pNode->GetType() << "\"" << endl;
			continue;
		}
		
		//Spawn it
		threads.push_back(new Thread(LaunchThread,const_cast<XmlNode*>(pNode)));
			
		//Short delay so we don't max out the CPU too much
#ifdef UNIX
		usleep(1000*5);
#elif defined(WINDOWS)
		Sleep(5);
#endif
	}
	
	cout << endl << "Ready" << endl;
	
	//Block on threads
	for(int i=0; i<threads.size(); i++)
	{
		threads[i]->WaitUntilTermination();
		delete threads[i];
	}

	return 0;
}

THREAD_PROTOTYPE(LaunchThread,_tag)
{
	//Get the node info
	const XmlNode* pTag = reinterpret_cast<XmlNode*>(_tag);
	string user,host,path,file;
	
	//Extract stuff from the XML
	for(int i=0; i<pTag->GetAttributeCount(); i++)
	{
		if(pTag->GetAttributeName(i) == "user")
			user = pTag->GetAttributeValue(i);
		if(pTag->GetAttributeName(i) == "host")
			host = pTag->GetAttributeValue(i);
		if(pTag->GetAttributeName(i) == "path")
			path = pTag->GetAttributeValue(i);
		if(pTag->GetAttributeName(i) == "file")
			file = pTag->GetAttributeValue(i);
	}
	
	//Sanity check (TODO: good errors)
	if(host.length() > 256)
		return 0;
	if(user.length() > 32)
		return 0;
	if(path.length() > 128)
		return 0;
	if(file.length() > 32)
		return 0;
	
	//Print some info
	cout << " * " << host.c_str() << endl;	
	
	//Set up the command
	char cmd[640];
	sprintf(cmd,"ssh %s@%s \"cd %s;./%s --server %s\" > /dev/null ",user.c_str(), host.c_str(), path.c_str(), file.c_str(), g_host.c_str());
	system(cmd);

	THREAD_RETURN(0);
}
