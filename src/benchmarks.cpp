/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0 DEFCON EDITION                                *
*                                                                             *
* Copyright (c) 2009-2010 RPISEC.                                             *
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
*******************************************************************************/

/**
	@file benchmarks.cpp
	@brief Performance benchmarks for development and optimization
 */

#include "mpicrack.h"
#include <stdlib.h>
#include <string.h>
#include "BaseN.h"

void MD5PerformanceTest()
{
	//Fill the plaintext with junk
	char* plaintext = static_cast<char*>(aligned_malloc(128));
	memset(plaintext, 0, 128);
	strcpy(plaintext, "foobar");
	strcpy(plaintext + 32, "foobaz");
	strcpy(plaintext + 64, "asdfgh");
	strcpy(plaintext + 96, "asdfgj");
	
	//Allocate hash block
	char* hash = static_cast<char*>(aligned_malloc(64));
	
	//Hash it
	int testcount = 1000000;
	double start = GetTime();
	for(int i=0; i<testcount; i++)
		MD5Hash(plaintext, hash, 6);
	double dt = GetTime() - start;
	double speed = 4*testcount / (1E6 * dt);
	printf("Elapsed time for %d iterations: %.2f ms (%.2f MHash/sec)\n", testcount, 1000*dt, speed);
	
	for(int i=0; i<4; i++)
	{
		printf("hash %d: ", i);
		unsigned char* ph = reinterpret_cast<unsigned char*>(hash) + 16*i;
		for(int j=0; j<16; j++)
			printf("%02x", ph[j]);
		printf("\n");
	}	
	
	aligned_free(hash);
	aligned_free(plaintext);
}

void ComparisonPerformanceTest()
{
	//Allocate a bunch of test arrays
	const int testcount = 10000;
	unsigned int* testlist = static_cast<unsigned int*>(aligned_malloc(testcount * 16));
	unsigned int* testhash = static_cast<unsigned int*>(aligned_malloc(16));
	memset(testlist, 0, testcount*16);
	unsigned int* last = testlist + 4*(0);	//now doing end-to-beginning search
	last[0] = testhash[0] = 0xdeadbeef;
	last[1] = testhash[1] = 0xbaadc0de;
	last[2] = testhash[2] = 0xf0000000;
	last[3] = testhash[3] = 0x00c0ffee;
	
	const int iters = 1000;
	int ret = -1;
	double start = GetTime();
	for(int i=0; i<iters; i++)
		ret = HashSearch(testhash, testlist, testcount);
	double dt = GetTime() - start;
	double speed = (iters*testcount) / (1E9 * dt);
	printf("Elapsed time for %d iterations: %.2f ms (%.2f GCmp/sec)\n", iters, 1000*dt, speed);
	printf("index = %d\n", ret);
	
	aligned_free(testlist);
	aligned_free(testhash);
}

void HashAndCheckPerformanceTest(unsigned int* hashbuf, int linecount)
{
	//Allocate a test blob
	const int testcount = 10000;
	unsigned int* testlist = static_cast<unsigned int*>(aligned_malloc(testcount * 16));
	memset(testlist, 0, testcount*16);
	
	//Copy target hashes into the first N, leaving the rest empty to stress the comparisons
	for(int i=0; i<4*linecount; i++)
		testlist[i] = hashbuf[i];
	
	//Fill the plaintext with some sample values
	char* plaintext = static_cast<char*>(aligned_malloc(128));
	memset(plaintext, 0, 128);
	strcpy(plaintext, "foobar");
	strcpy(plaintext + 32, "foobaz");
	strcpy(plaintext + 64, "asdfgh");
	strcpy(plaintext + 96, "asdfgj");
	
	//Allocate hash block
	char* hash = static_cast<char*>(aligned_malloc(64));
	unsigned int* hashes = reinterpret_cast<unsigned int*>(hash);

	int hits[4] = {-2, -2, -2, -2};

	//Hash it
	int iters = 10000;
	double start = GetTime();
	for(int i=0; i<iters; i++)
	{
		//Hash the plaintext
		MD5Hash(plaintext, hash, 6);
		
		//Search each hash against the dump
		for(int j=0; j<4; j++)
			hits[j] = HashSearch(hashes + 4*j, testlist, testcount);
	}
	double dt = GetTime() - start;
	double speed = 4*testcount / (1E6 * dt);
	double cspeed = speed * iters;
	printf("Elapsed time for %d iterations of %d hashes: %.2f ms (%.2f MHash/sec, %.2f MCmp/sec)\n",
		iters, testcount, 1000*dt, speed, cspeed);
	
	for(int i=0; i<4; i++)
	{
		char* p = plaintext + 32*i;
		p[6] = 0;
		printf("%s: hit %d\n", p, hits[i]);
	}
	
	aligned_free(hash);
	aligned_free(plaintext);
	aligned_free(testlist);
}

void IncrementPerformanceTest()
{
	int num[8] = {0};
	int testcount = 100000000;
	double start = GetTime();
	for(int i=0; i<testcount; i++)
		BaseNAdd1(num, 62, 8);
	double dt = GetTime() - start;
	double speed = testcount / (1E9 * dt);
	printf("Elapsed time for %d iterations: %.2f ms (%.2f GInc/sec)\n", testcount, 1000*dt, speed);
}