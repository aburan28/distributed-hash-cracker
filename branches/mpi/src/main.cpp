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
	@file main.cpp
	@brief Entry point
 */
 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

void* aligned_malloc(size_t size);
void aligned_free(void* ptr);
double GetTime(clockid_t id = CLOCK_REALTIME);
double GetTimeResolution(clockid_t id = CLOCK_REALTIME);

extern "C" int HashSearch(unsigned int* hash, unsigned int* list, int count);

void ComparisonPerformanceTest();

unsigned int* read_hashes(long* pLinecount, const char* fname, int rank);

int main(int argc, char* argv[])
{
	//Standard init
	int rank;
	int size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//Parse arguments
	const char* fname = "testvectors.txt";
	int maxlength = 6;
	
	/*
	printf("Resolution of CLOCK_REALTIME: %.7f ns\n", 1E9 * GetTimeResolution(CLOCK_REALTIME));
	printf("Resolution of CLOCK_MONOTONIC: %.7f ns\n", 1E9 * GetTimeResolution(CLOCK_MONOTONIC));
	printf("Resolution of CLOCK_PROCESS_CPUTIME_ID: %.7f ns\n", 1E9 * GetTimeResolution(CLOCK_PROCESS_CPUTIME_ID));
	*/
	
	//Read input
	long linecount = 0;
	unsigned int* hashbuf = read_hashes(&linecount, fname, rank);

	ComparisonPerformanceTest();
	
	//TODO: preprocess (subtract constants etc)
	
	for(int len = 1; len < maxlength; len++)
	{
		/*
			Find our block's start and end values
			Divide into groups of ~10M guesses
			For each group:
				Fill known areas of md5 buffer
				For each guess:
					generate plaintexts
					hash plaintexts
					check plaintexts against list of hashes, add successes to list if needed
				Print out successes to logfile
				Send all of our successes out to other nodes and get their successes
				Remove successful plaintexts from target list
		 */					
	}
	
	//Clean up
	aligned_free(hashbuf);
	MPI_Finalize();
	return 0;
}

/**
	@brief Allocates memory aligned on a 16 byte boundary
 */
void* aligned_malloc(size_t size)
{
	//32 = 16 + sizeof(void*) rounded to next 16 bytes
	void* ptr = malloc(size + 32);

	uintptr_t nptr = (reinterpret_cast<uintptr_t>(ptr) + 32) & (~0xf);
	*(reinterpret_cast<void**>(nptr) - 1) = ptr;	
	return reinterpret_cast<void*>(nptr);
}

/**
	@brief Frees memory allocated by aligned_malloc()
 */
void aligned_free(void* ptr)
{
	free(*(reinterpret_cast<void**>(ptr) - 1));
}

/**
	@brief Reads hashes from disk
 */
unsigned int* read_hashes(long* pLinecount, const char* fname, int rank)
{

	//Read input file
	//We expect 33 bytes per line (32 hex + \n)
	FILE* fp = fopen(fname, "r");
	if(!fp)
	{
		printf("file open error\n");
		MPI_Abort(MPI_COMM_WORLD, 0);
	}
	if(0 != fseek(fp, 0, SEEK_END))
	{
		printf("file seek error\n");
		MPI_Abort(MPI_COMM_WORLD, 0);
	}
	long count = ftell(fp);
	if(0 != fseek(fp, 0, SEEK_SET))
	{
		printf("file seek error\n");
		MPI_Abort(MPI_COMM_WORLD, 0);
	}
	long linecount = ceil(static_cast<float>(count) / 33);
	if(rank == 0)
		printf("Reading %ld hashes from disk... ", linecount);
	unsigned int* hashbuf = static_cast<unsigned int*>(aligned_malloc(count * 16));
	for(long i=0; i<linecount; i++)
	{
		unsigned int* base = hashbuf + (i*4);
		unsigned char* pbase = reinterpret_cast<unsigned char*>(base);
		unsigned int x;
		for(int j=0; j<16; j++)
		{
			if(1 != fscanf(fp, "%02x", &x))
			{
				printf("Malformed input on line %ld\n", i);
				MPI_Abort(MPI_COMM_WORLD, 0);
			}
			pbase[j] = x & 0xff;
		}
	}
	fclose(fp);
	
	printf("done\n");
	
	*pLinecount = linecount;
	return hashbuf;
}

/**
	@brief Returns a time value, in milliseconds, suitable for performance profiling.
 */
double GetTime(clockid_t id)
{
	timespec t;
	clock_gettime(id,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;
}

/**
	@brief Get resolution of a clock
 */
double GetTimeResolution(clockid_t id)
{
	timespec t;
	clock_getres(id,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;
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
