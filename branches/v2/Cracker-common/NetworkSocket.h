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
* NetworkSocket.h - cross-platform socket library                             *
*                                                                             *
******************************************************************************/

#ifndef Z_NETWORKSOCKET_H
#define Z_NETWORKSOCKET_H

#include "config.h"

#include "Cracker-common.h"

//Pull in portable stuff
#include <string>
#include <cstdlib>
#include <memory.h>

//Pull in some OS specific stuff
#if WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#define ZSOCKLEN int
#define ZSOCKET SOCKET

#elif UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define ZSOCKLEN socklen_t
#define ZSOCKET int

#else
#error Unrecognized platform.
#endif

//CRACKER_COMMON_EXPORT_FUNC std::string HttpTransaction(std::string host,std::string file);

//Class representing a network socket.
//IPv4 only for now.
class CRACKER_COMMON_EXPORT NetworkSocket
{
public:
	NetworkSocket(int af,int type,int protocol);

	//Create a Socket object from an existing socket
	NetworkSocket(ZSOCKET sock, int af=PF_INET);

	//Destructor
	~NetworkSocket(void);

	//Connect to a host (automatic DNS resolution)
	void Connect(const char* host,unsigned short port);

	//Bind to a port (any available interface)
	void Bind(unsigned short port);

	//Put us in listening mode
	void Listen();

	//Accept a new connection
	NetworkSocket Accept(sockaddr_in* addr,ZSOCKLEN len);

	//Disconnect us from the socket object
	ZSOCKET Detach();

	//Send / receive data
	void Send(const char* data,int len);
	void Send(std::string str);
	int Recv(char* buf,int len);
	std::string Recv(int bufmax);

	//Static init / cleanup (only required on Windows, Linux compiles to an empty function)
	static void StaticInit();
	static void StaticCleanup();

	//Convert us to the native OS socket type
	operator ZSOCKET()
	{ return m_socket; }

protected:
	int m_af;
	ZSOCKET m_socket;
};

#endif
