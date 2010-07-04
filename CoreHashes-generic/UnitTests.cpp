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

#include "CoreHashes-generic.h"

#include <iostream>
#include <string>
#include <cstring>
using namespace std;

#include "MD5.h"
#include "SHA1.h"
#include "MD5crypt.h"

#ifdef _MSC_VER
# define DOALIGN16 extern "C" __declspec(align(16))
# define ASMSYMBOL(x) x
#elif defined(MACOSX)
# define DOALIGN16 __attribute__((aligned (16)))
# define ASMSYMBOL(x) x
#else
# define DOALIGN16 __attribute__((aligned (16)))
# define ASMSYMBOL(x) _ ## x
#endif 


#if defined(X86) || defined(AMD64)
extern "C" void ASMSYMBOL(MD5SSE2Hash)(unsigned char* in,    unsigned char* out, int len);
#endif

ZEXPORT void UnitTests();

void MD5PortableTest();
#if defined(X86) || defined(AMD64)
void MD5SSE2Test();
#endif

void SHA1PortableTest();

void MD5CryptTest();

string BinToHex(unsigned char* str, int len);

void UnitTests()
{
	cout << "\033[1;34mBeginning unit tests for module CoreHashes-generic\033[0m" << endl;

	MD5PortableTest();
#if defined(X86) || defined(AMD64)
	MD5SSE2Test();
#endif
	
	SHA1PortableTest();
	
	MD5CryptTest();
	
	cout << endl;
}

void MD5PortableTest()
{
	unsigned char out[128] = "";
	const int test_point_count = 2;

	//Test points
	unsigned char testpoints[test_point_count][2][64]=
	{
		{ "hello", "5d41402abc4b2a76b9719d911017c592" },
		{ "abc",   "900150983cd24fb0d6963f7d28e17f72" }
	};

	//Run the test
	cout << " * MD5_Portable... " ;
	for(int i=0; i<test_point_count; i++)
	{
		//Calculate the hash
		MD5_Portable::DoHash(
			testpoints[i][0],
			out, 
			strlen(reinterpret_cast<char*>(testpoints[i][0])),
			NULL,
			0);
			
		//Convert to hex
		string hexout = BinToHex(out,16);
		
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


#if defined(X86) || defined(AMD64)
void MD5SSE2Test()
{
	unsigned char out[128] = "";
	const int test_point_count = 2;

	//Test points
	unsigned char testpoints[test_point_count][2][64]=
	{
		{ "hello", "5d41402abc4b2a76b9719d911017c592" },
		{ "abc",   "900150983cd24fb0d6963f7d28e17f72" }
	};
	
	//Run the test
	cout << " * MD5_SSE2... " ;
	for(int i=0; i<test_point_count; i++)
	{
		int inlen = strlen(reinterpret_cast<char*>(testpoints[i][0]));
	
		unsigned char inbuf[1024];
		unsigned char* pIn = &inbuf[0];
		for(int j=0; j < 4; j++)
		{
			memcpy(pIn, testpoints[i][0], inlen);
			pIn += inlen;
		}
	
		//Calculate the hash
		ASMSYMBOL(MD5SSE2Hash)(	inbuf, out, inlen);
			
		//Process the result
		string expected = string(reinterpret_cast<char*>(testpoints[i][1]));
		for(int j=0; j<4; j++)
		{
			//Convert to hex
			string hexout = BinToHex(&out[16*j],16);
		
			if(hexout != expected)
			{
				//show error and return
				cout << "\033[1;31mFAILED\033[0m" << endl;
			
				cout << "    MD5(\"" << reinterpret_cast<char*>(testpoints[i][0]) << "\") should be " << 
					expected.c_str() << ", calculated " << hexout.c_str() << " in block " << j << endl;
			
				return;
			}
		}
	}
	
	cout << "\033[32mpassed\033[0m" << endl;
}
#endif

void SHA1PortableTest()
{
	unsigned char out[128] = "";
	const int test_point_count = 2;

	//Test points
	unsigned char testpoints[test_point_count][2][64]=
	{
		{ "hello", "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d" },
		{ "abc",   "a9993e364706816aba3e25717850c26c9cd0d89d" }
	};

	//Run the test
	cout << " * SHA1_Portable... " ;
	for(int i=0; i<test_point_count; i++)
	{
		//Calculate the hash
		SHA1_Portable::DoHash(
			testpoints[i][0],
			out, 
			strlen(reinterpret_cast<char*>(testpoints[i][0])),
			NULL,
			0);
			
		//Convert to hex
		string hexout = BinToHex(out,20);
		
		//Process the result
		if(hexout != string(reinterpret_cast<char*>(testpoints[i][1])))
		{
			//show error and return
			cout << "\033[1;31mFAILED\033[0m" << endl;
			
			cout << "    SHA1(\"" << reinterpret_cast<char*>(testpoints[i][0]) << "\") should be " << 
				reinterpret_cast<char*>(testpoints[i][1]) << ", calculated " <<
				hexout.c_str() << endl;
			
			return;
		}
	}
	
	cout << "\033[32mpassed\033[0m" << endl;
}

void MD5CryptTest()
{
	unsigned char out[128] = "";
	const int test_point_count = 1;

	//Test points
	unsigned char testpoints[test_point_count][3][64]=
	{
		{ "hello", "NJH253TEOnwiUlwFIuqlR", "4CiZcJmt" }, //end in R
	};

	//Run the test
	cout << " * MD5crypt_Portable... " ;
	for(int i=0; i<test_point_count; i++)
	{
		//Calculate the hash
		MD5crypt_Portable::DoHash(
			testpoints[i][0],
			out, 
			strlen(reinterpret_cast<char*>(testpoints[i][0])),
			testpoints[i][2],
			strlen(reinterpret_cast<char*>(testpoints[i][2])));
			
		out[21]='\0';
			
		//Process the result
		if(string(reinterpret_cast<char*>(out)) != string(reinterpret_cast<char*>(testpoints[i][1])))
		{
			//show error and return
			cout << "\033[1;31mFAILED\033[0m" << endl;
			
			cout << "    MD5crypt(\"" << reinterpret_cast<char*>(testpoints[i][0]) << "\", \"" << 
				reinterpret_cast<char*>(testpoints[i][2]) << "\") should be " << 
				reinterpret_cast<char*>(testpoints[i][1]) << ", calculated " <<
				reinterpret_cast<char*>(out) << endl;
			
			return;
		}
	}
	
	cout << "\033[32mpassed\033[0m" << endl;
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
