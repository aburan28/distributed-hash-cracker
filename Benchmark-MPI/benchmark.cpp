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
* benchmark.cpp - MPI-based benchmark for MD5                                 *
*                                                                             *
******************************************************************************/

#include <mpi.h>
#include <stdio.h>
#include <time.h>

int g_rank;
int g_size;

void DoHash(unsigned char* in, unsigned char* out, int len);

#define BILLION 1000000000
#define TEST_BILLIONS 20

int main(int argc, char* argv[])
{
	//Get MPI up and running
	MPI_Init(&argc,	&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&g_rank);
	MPI_Comm_size(MPI_COMM_WORLD,&g_size);
	
	time_t ts,te;
	char buf[16] = 	"hello\0\0\0\0\0\0\0\0\0\0";
	
	unsigned long long runcount = TEST_BILLIONS;
	runcount *= BILLION;
	runcount /= g_size;
	
	//Do one MD5 as a sanity check
	if(g_rank == 0)
	{
		printf("Test count: %d billion\n",TEST_BILLIONS);
		printf("MD5(%s) = ",buf);
		
		DoHash((unsigned char*)buf, (unsigned char*)buf, 5);
		
		for(int i=0;i<16;i++)
			printf("%02x",buf[i] & 0xFF);
		printf("\n");
		
	}
	
	//Calculate a lot of MD5s
	MPI_Barrier(MPI_COMM_WORLD);
	time(&ts);
	for(unsigned long long i=0;i<runcount;i++)
		DoHash((unsigned char*)buf, (unsigned char*)buf, 5);
	MPI_Barrier(MPI_COMM_WORLD);
	time(&te);
	
	if(g_rank == 0)
	{
		int dt = te-ts;
		printf("Elapsed time: %d seconds\n",dt);
		printf("Total throughput for %d nodes: %f M/sec\n",g_size,1000 * static_cast<float>(TEST_BILLIONS)/dt);
	}
	
	//Clean up
	MPI_Finalize();
	return 0;
}