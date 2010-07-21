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
#include <stdlib.h>
#include <math.h>
#include "mpicrack.h"
#include "BaseN.h"

int main(int argc, char* argv[])
{
	//Standard init
	int rank;
	int size;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//TODO: Parse arguments
	const char* fname = "testvectors.txt";
	int maxlength = 4;
	const char* charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int base = strlen(charset);
	
	//Read input
	long linecount = 0;
	unsigned int* hashbuf = read_hashes(&linecount, fname, rank);
	unsigned char* phashbuf = reinterpret_cast<unsigned char*>(hashbuf);

	//ComparisonPerformanceTest();
	//MD5PerformanceTest();
	//HashAndCheckPerformanceTest(hashbuf, linecount);
	//IncrementPerformanceTest();
	//WorkUnitPerformanceTest();
	
	//TODO: preprocess (subtract constants etc) the list
	
	int guessesPerBlock = 1000000;					//1M. Can be changed but must be divisible by 4
	int start[MAX_BASEN_LENGTH] = {0};
	for(int len = 1; len <= maxlength; len++)
	{
		if(rank == 0)
			printf("Testing length %d\n", len);
	
		//Initial offset
		//TODO: efficient way of adding quantities which may be over INT_MAX
		memset(start, 0, MAX_BASEN_LENGTH*4);
		for(int i=0; i<rank; i++)
			BaseNAdd(start, base, len, guessesPerBlock);
		
		char* guesses = static_cast<char*>(aligned_malloc(4*MAX_BASEN_LENGTH));
		memset(guesses, 0, 4*MAX_BASEN_LENGTH);
		char* hash = static_cast<char*>(aligned_malloc(64));
		unsigned int* hashes = reinterpret_cast<unsigned int*>(hash);
		
		//Run this length
		bool bDone = false;
		while(!bDone)
		{
			//TODO: Fill known areas of md5 buffer with zeros and length
			//so md5 kernel doesnt have to do this in the inner loop
			
			//Run the block
			//This should take on the order of 0.1 - 1s to minimize collective latency problems
			double tstart = GetTime();
			bool bQuitNextRound = false;
			for(int i=0; i < guessesPerBlock; i+=4)
			{
				//TODO: optimize guess generation for minimal overhead if length hasnt changed etc
				
				//Build four guesses
				for(int j=0; j<4; j++)
				{
					for(int k=0; k<len; k++)
						guesses[j*MAX_BASEN_LENGTH + k] = charset[start[k]];
								
					BaseNAdd1(start, base, len);
				}
				
				//Hash
				MD5Hash(guesses, hash, len);
				
				//Now for the fun part... check if we found anything
				for(int j=0; j<4; j++)
				{
					int hit = CHashSearch(hashes + 4*j, hashbuf, linecount);
						
					if(hit >= 0)
					{
						guesses[j*MAX_BASEN_LENGTH + len] = 0;
						printf("hit: %s\n", guesses + j*MAX_BASEN_LENGTH);
					}
				}
				
				//Check if we're at the end
				if(bQuitNextRound)
					break;
				bQuitNextRound = true;
				for(int k=0; k<len; k++)
				{
					if(start[k] != (base - 1) )
					{
						bQuitNextRound = false;
						break;
					}
				}
			}
			double dt = GetTime() - tstart;
			
			/*if(rank == 0)
				printf("block done (in %.2f ms)\n", dt*1000);*/
			
			//See if we hit the end of the list
			
			/*
				Print out successes to logfile
				Send all of our successes out to other nodes and get their successes
				Remove successful plaintexts from target list
			*/
			
			//Are we done yet? TODO collective
			MPI_Barrier(MPI_COMM_WORLD);
			bDone = bQuitNextRound;
		}
		
		aligned_free(hash);
		aligned_free(guesses);
		
		//placeholder for collective speed measuring etc 
		MPI_Barrier(MPI_COMM_WORLD);
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

int CHashSearch(unsigned int* hash, unsigned int* list, int count)
{
	for(int i=0; i<count; i++)
    {
        unsigned int* row = list + (i*4);
        if(hash[0] != row[0])
            continue;
        if(hash[1] != row[1])
            continue;
        if(hash[2] != row[2])
            continue;
        if(hash[3] != row[3])
            continue;
        return i;
    }
    return -1;
}