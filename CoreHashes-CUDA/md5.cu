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
* md5.cu - control code for CUDA-based MD5                                    *
*                                                                             *
******************************************************************************/

//main includes
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "CoreHashes-CUDA.h"

//kernels
#include "md5_kernel.cu"

////////////////////////////////////////////////////////////////////////////////
// MD5_CUDA
////////////////////////////////////////////////////////////////////////////////

MD5_CUDA::MD5_CUDA()
{
	Hash = reinterpret_cast<HASHPROC>(md5CudaHashProc);
}

MD5_CUDA::~MD5_CUDA()
{

}

const char* MD5_CUDA::GetName()
{
	return "md5";
}

int MD5_CUDA::GetMaxInputSize()
{
	return 40;
}

int MD5_CUDA::GetVectorSize()
{
	return 1;	//only used in CPU based hashes
}

int MD5_CUDA::GetHashLength()
{
	return 16;
}

bool MD5_CUDA::IsGPUBased()
{
	return true;
}

void md5CudaHashProc(int* startguess, unsigned char* target, unsigned char* charset, int csetsize,
	unsigned char* output, int* bFound, int guesslen,int blocks, int threads)
{
	dim3  vBlocks(blocks, 1, 1);
	dim3  vThreads(threads, 1, 1);
	
	//charset + startguess + target
	int shmem = 255 + 20*sizeof(int) + 16;
	
	md5Kernel<<< vBlocks, vThreads, shmem >>>(startguess,target,charset,csetsize,output,bFound,guesslen);
}
