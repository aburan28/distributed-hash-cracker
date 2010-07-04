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
* UnitTests.cpp - unit tests for this module                                  *
*                                                                             *
******************************************************************************/
#include "config.h"
#include "CoreHashes-CUDA.h"
#include <iostream>
#include <cstring>
using namespace std;

extern "C" EXPORTED void UnitTests();

//void MD5CudaTest();

string BinToHex(unsigned char* str, int len);
void HexToBin(unsigned char* out, int len, unsigned char* in);

void UnitTests()
{
	cout << "\033[1;34mBeginning unit tests for module CoreHashes-CUDA\033[0m" << endl;
	
	cout << " * Not implemented!" << endl;
	
//	MD5CudaTest();

	cout << endl;
}

/*
void MD5CudaTest()
{
	unsigned char out[128] = "";
	const int test_point_count = 2;

	//Test points
	unsigned char testpoints[test_point_count][2][64]=
	{
		{ "aaaaa",  "594f803b380a41396ed63dca39503542" },
		{ "abadzz", "2849c5533f5f5dcb474f129f442285c0" }
	};
	unsigned char charset[] = "abcdefghijklmnopqrstuvwxyz";

	//Run the test
	cout << " * MD5_CUDA... " ;
	for(int i=0; i<test_point_count; i++)
	{
		//Get the test vector
		unsigned char target[16];
		HexToBin(target, 16, testpoints[i][1]);
		
		int start[]={0,0,0,0,0,0,0,0};
		
		int glen = strlen(reinterpret_cast<char*>(testpoints[i][0]));
		
		//TODO: copy mem to device and back :D
		
		//Try cracking it
		unsigned char outbuf[32];
		int bFound = 0;
		md5CudaHashProc(start,target,charset,26,outbuf,&bFound,glen,128,64);
		
		cout << bFound;
		cout << (char*)&outbuf[0] << endl;
		
	
		//Process the result
		if(hexout != string(reinterpret_cast<char*>(testpoints[i][1])))
		{
			//show error and return
			cout << "\033[1;31mFAILED\033[0m" << endl;
			
			cout << "    MD5(\"" << reinterpret_cast<char*>(testpoints[i][0]) << "\") should be " << 
				reinterpret_cast<char*>(testpoints[i][1]) << ", calculated " <<
				hexout.c_str() << endl;
			
			return;
		}
	}
	
	cout << "\033[32mpassed\033[0m" << endl;
}
*/
void HexToBin(unsigned char* out, int len, unsigned char* in)
{
	if(len & 1)
	{
		cout << "\033[1;31mhex values must be of even length\033[0m" << endl;
		return;
	}

	//Decode
	for(int i=0; i<len; i++)
	{
		char buf[] = {'0','x', in[i*2], in[i*2 + 1], '\0'};
		unsigned int temp;
		sscanf(buf, "%X", &temp);
		out[i] = temp;
	}
}

string BinToHex(unsigned char* str, int len)
{
	string ret;
	char buf[3];
	for(int i=0; i<len; i++)
	{
		sprintf(buf,"%02x",str[i] & 0xFF);
		buf[2] = '\0';
		ret += buf;
	}
	return ret;
}
