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
* NetworkSocket.cpp - cross-platform socket library                           *
*                                                                             *
******************************************************************************/

#if WINDOWS
# pragma warning(disable:4996)
#endif


#include "NetworkSocket.h"

using namespace std;

NetworkSocket::NetworkSocket(int af,int type,int protocol)
: m_af(af)
{
	//For once - a nice, portable call, no #ifdefs required.
	m_socket=socket(af,type,protocol);

	//Too bad error checking isn't portable!
#if WINDOWS
	if(INVALID_SOCKET == m_socket)
		throw string("Socket creation failed");
#elif UNIX
	if(0 == m_socket)
		throw string("Socket creation failed");
#endif
}

NetworkSocket::NetworkSocket(ZSOCKET sock, int af)
: m_af(af)
, m_socket(sock)
{

}

NetworkSocket::~NetworkSocket(void)
{
	//There are a couple of different ways to close a socket...
	if(m_socket)
	{
#if WINDOWS
		closesocket(m_socket);
#elif UNIX
		close(m_socket);
#endif
	}
}

void NetworkSocket::Connect(const char* host,unsigned short port)
{
#if WINDOWS
	//Do a DNS lookup
	char servname[]="http";	//we have to do lookups using a service name, so pick something
	ADDRINFO* address=NULL;
	if(0!=getaddrinfo(host,servname,NULL,&address))
		throw string("DNS lookup failed");
	if(address==NULL)
		throw string("DNS lookup failed");
	unsigned long ip = reinterpret_cast<sockaddr_in*>(address->ai_addr)->sin_addr.S_un.S_addr;
	freeaddrinfo(address);
#elif UNIX
	//Do a DNS lookup
	hostent* ent=gethostbyname(host);
	unsigned long ip = *reinterpret_cast<unsigned long*>(ent->h_addr_list[0]);
#endif

	//Set up the full address
	sockaddr_in saddr;
	saddr.sin_family=m_af;
	saddr.sin_port=htons(port);
	memcpy(&saddr.sin_addr,&ip,sizeof(ip));

	//Connect to the socket
	if(0 != connect(m_socket,reinterpret_cast<sockaddr*>(&saddr),sizeof(saddr)))
		throw string("Failed to connect to server");
}

void NetworkSocket::Send(const char* data,int len)
{
	send(m_socket,data,len,0);
}

void NetworkSocket::Send(std::string str)
{
	Send(str.c_str(), str.length());
}

int NetworkSocket::Recv(char* buf, int len)
{
	int n=recv(m_socket,buf,len,0);
	if(n==-1)
		throw string("connection dropped");
	return n;
}

std::string NetworkSocket::Recv(int bufmax)
{
	//Allocate the buffer and put content in it
	char* buf=new char[bufmax];
	int len = Recv(buf,bufmax-1);
	if(len==-1)
		throw string("connection dropped");

	//Null terminate it
	buf[len]='\0';

	//Move to an STL string and clean up
	string ret(buf);
	delete[] buf;

	//Done
	return ret;
}

void NetworkSocket::StaticInit()
{
#if WINDOWS
	WSADATA wdat;
	if(0 != WSAStartup(MAKEWORD(2,2),&wdat))
		throw string("WSAStartup failed");
#elif UNIX
	//No static init required on POSIX
#endif
}

void NetworkSocket::StaticCleanup()
{
#if WINDOWS
	WSACleanup();
#elif UNIX
	//No static cleanup required on POSIX
#endif
}

void NetworkSocket::Bind(unsigned short port)
{
	//Bind to IP 0.0.0.0 - this means any available interface, using our specified port
	sockaddr_in name;
	memset(&name,0,sizeof(name));
	
	//Set port number
	name.sin_family=m_af;
	name.sin_port=htons(port);

	//Try binding the socket
	if(0 != bind (m_socket,reinterpret_cast<sockaddr*>(&name),sizeof(name)) )
		throw string("Failed to bind socket");
}

void NetworkSocket::Listen()
{
	listen(m_socket,SOMAXCONN);
}

NetworkSocket NetworkSocket::Accept(sockaddr_in* addr,ZSOCKLEN len)
{
	ZSOCKET sock = accept(m_socket,reinterpret_cast<sockaddr*>(addr),&len);

	//Error check
#if WINDOWS
	if(sock==INVALID_SOCKET)
#elif UNIX
	if(sock==0)
#endif
	{
		throw string("Failed to accept socket connection (make sure socket is in listening mode)");
	}

	return NetworkSocket(sock,m_af);
}

/*
//Make a simple HTTP request and return the contents of the page (minus headers).
string HttpTransaction(string host,string file)
{
	//TODO: handle chunked encoding correctly

	//Create a socket	
	NetworkSocket sock(PF_INET,SOCK_STREAM,0);

	//Connect to the host on port 80
	sock.Connect(host.c_str(),80);

	//Send the request
	sock.Send(string("GET /") + file + string(" HTTP/1.1\r\nHost: ") + host +
		"\r\nConnection: close\r\nAccept-Encoding:\r\n\r\n");

	//Get the response (max 32k)
	string response = sock.Recv(32768);

	if(response.find("Transfer-Encoding: chunked",0) > 0)
	{
		int offset=response.find("\r\n\r\n");
		string packet = response.substr(offset + 4);

		//We got the headers - start reading chunks
		string ret;
		while(true)
		{
			//Length\r\npacket
			int p=packet.find("\r\n");
			if(p==string::npos)
				break;
			string len = packet.substr(0,p);
			int n=0;
			sscanf(len.c_str(),"%x",&n);
			string dat = packet.substr(p+2,n);

			//Process it
			if(n==0)
				break;

			//Save it
			ret+=dat;

			//Read next packet
			packet = sock.Recv(32768);
		}

		//Done
		return ret;
	}
	else
	{
		//Look for the first empty line. This means "end of headers"
		int offset=response.find("\r\n\r\n");
		if(offset <= 0)
			throw string("Invalid HTTP response");

		//Return the string (minus headers).
		return response.substr(offset + 4);
	}
}
*/

//Disconnect us from the socket object
ZSOCKET NetworkSocket::Detach()
{
	ZSOCKET s=m_socket;
	m_socket=NULL;
	return s;
}
