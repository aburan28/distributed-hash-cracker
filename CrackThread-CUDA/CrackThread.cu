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
* CrackThread.cu - main code goes here                                        *
*                                                                             *
******************************************************************************/

#include "CrackThread-CUDA.h"
#include "../Cracker-common/HashingAlgorithm.h"
#include "../Cracker-common/BaseNInteger.h"
#include "../ComputeNode/CrackThread.h"

#include "config.h"

using namespace std;

double GetTime();

__global__ void sanityKernel1(int* bOK)
{
	*bOK = 1;	
}

__global__ void sanityKernel2(int* bOK)
{
	*bOK = 2;	
}

typedef void (*CUDA_HASHPROC)(int*, unsigned char*, unsigned char*, int, unsigned char*, int*, int, int,int);

#if WINDOWS
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

int GetOptimalThreadCount(HashingAlgorithm* pAlg);

THREAD_PROTOTYPE(internalComputeThreadProc,_pData)
{
	try
	{
		//Cache some settings
		CrackThreadData* pData = reinterpret_cast<CrackThreadData*>(_pData);
		BaseNInteger start=pData->start;
		BaseNInteger end=pData->end;
		int guesslen = start.GetSize();
		string target=pData->target;
		volatile bool& bDoneWithWU = *pData->bDoneWithWU;
		
		//Convert target from hex to binary
		HashingAlgorithm* pAlg = pData->pAlg;
		int hashsize = pAlg->GetHashLength();
		unsigned char* btarget = new unsigned char[hashsize];
		for(unsigned int i=0;i<hashsize;i++)
		{
			int b;
			sscanf(target.substr(i*2,2).c_str(),"%x",&b);
			btarget[i]=b;
		}
		
		//Set our CUDA device
		cudaError_t err = cudaSuccess;
		if(cudaSuccess != (err = cudaSetDevice(pData->tid)) )
			throw string("Failed to set CUDA device: ") + cudaGetErrorString(err);
			
		//Run a quick test to make sure that the CUDA driver is set up properly for this device
		int* d_sanitybuf;
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_sanitybuf), 4)) )
			throw string("Failed to allocate GPU memory for sanity check: ") + cudaGetErrorString(err);
		int bSanityCheck = 0xCC;
		sanityKernel1<<<128,128>>>(d_sanitybuf);
		if(cudaSuccess != (err = cudaMemcpy(&bSanityCheck, d_sanitybuf, 4, cudaMemcpyDeviceToHost)) )
			throw string("Failed to copy sanity check") + cudaGetErrorString(err);
		if(bSanityCheck != 1)
		{
			char err[1024];
			sprintf(
				err,
				"CUDA sanity check failed: read 0x%x, expected 0x1. Please verify that you have "
				"a CUDA-compatible graphics card and install the latest CUDA drivers from "
				"http://www.nvidia.com/object/cuda_get.html."
				,bSanityCheck);
			throw string(err);
		}
		sanityKernel2<<<128,128>>>(d_sanitybuf);
		if(cudaSuccess != (err = cudaMemcpy(&bSanityCheck, d_sanitybuf, 4, cudaMemcpyDeviceToHost)) )
			throw string("Failed to copy sanity check") + cudaGetErrorString(err);
		if(bSanityCheck != 2)
		{
			char err[1024];
			sprintf(
				err,
				"CUDA sanity check failed: read 0x%x, expected 0x2. Please verify that you have "
				"a CUDA-compatible graphics card and install the latest CUDA drivers from "
				"http://www.nvidia.com/object/cuda_get.html."
				,bSanityCheck);
			throw string(err);
		}
		cudaFree(d_sanitybuf);
		d_sanitybuf=NULL;
			
		//Get pointer to hashing algorithm
		CUDA_HASHPROC DoHash = reinterpret_cast<CUDA_HASHPROC>(pAlg->Hash);

		//Get optimal thread count (will be set to 0 if not known, in which case we need to calculate it)
		int threads = GetOptimalThreadCount(pAlg);
		int blocks = 32768;
			
 		//Copy charset to GPU
 		unsigned char* d_charset=NULL;
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_charset), pData->charset.length())) )
			throw string("Failed to allocate GPU memory for charset: ") + cudaGetErrorString(err);
		if(cudaSuccess != (err = cudaMemcpy(d_charset, pData->charset.c_str(), pData->charset.length(), cudaMemcpyHostToDevice)) )
				throw string("Failed to copy charset buffer: ") + cudaGetErrorString(err);
		
		//Allocate GPU memory for start guess
		int* d_startguess=NULL;
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_startguess), guesslen*sizeof(int))) )
			throw string("Failed to allocate GPU memory for start guess: ") + cudaGetErrorString(err);

		//Allocate GPU memory for cracked value
		unsigned char* d_cracked = NULL;
		int* d_bCracked = NULL;
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_cracked), 32)) )
			throw string("Failed to allocate GPU memory for cracked value: ") + cudaGetErrorString(err);
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_bCracked), 4)) )
			throw string("Failed to allocate GPU memory for crack flag: ") + cudaGetErrorString(err);
		int zero = 0;
		if(cudaSuccess != (err = cudaMemcpy(d_bCracked, &zero, 4, cudaMemcpyHostToDevice)) )
			throw string("Failed to copy zero: ") + cudaGetErrorString(err);
			
		//Allocate and copy target
		unsigned char* d_target = NULL;
		if(cudaSuccess != (err = cudaMalloc(reinterpret_cast<void**>(&d_target), hashsize)) )
			throw string("Failed to allocate GPU memory for target: ") + cudaGetErrorString(err);
		if(cudaSuccess != (err = cudaMemcpy(d_target, btarget, hashsize, cudaMemcpyHostToDevice)) )
			throw string("Failed to copy target: ") + cudaGetErrorString(err);
		
		int* startdigits = &start.digits[0];
		
		//Don't have optimal settings stored? Run a little test
		if(threads==0)
		{
			cout << endl;
			cout << "Algorithm " << pAlg->GetName() << " does not have optimal settings in config file, performing benchmark" << endl;
		
			int optThreads = 1;
			float optSpeed = 0;

			//TODO: allow use of more threads on devices with >8192 regs
			int testBlocks = 4096;
			for(threads=16; threads <= 255; threads++)
			{
				//Copy start guess to GPU memory
				double start = GetTime();
				if(cudaSuccess != (err = cudaMemcpy(d_startguess, startdigits, guesslen * sizeof(int), cudaMemcpyHostToDevice)) )
					throw string("Failed to copy start-guess buffer: ") + cudaGetErrorString(err);
		
				//Test this thread count
				DoHash(
					d_startguess,
					d_target,
					d_charset,
					(int)pData->charset.length(),
					d_cracked,
					d_bCracked,
					guesslen,
					testBlocks,
					threads);
				
				//Copy some data back so we wait for the kernel
				int bCracked = 0;
				if(cudaSuccess != (err = cudaMemcpy(&bCracked, d_bCracked, 4, cudaMemcpyDeviceToHost)) )
					throw string("Failed to copy cracked flag: ") + cudaGetErrorString(err);
					
				double dt = GetTime()-start;
				float speed = static_cast<float>(threads*testBlocks)/(1E6 * dt);
				
				//See what turns out best
				if(speed > optSpeed)
				{
					optSpeed = speed;
					optThreads = threads;
				}
				
				if((threads & 0xF) == 0)
				{
					cout << ".";
					fflush(stdout);
				}
			}
			
			FILE* fp = fopen("cudathreads.conf","a");
			fprintf(fp,"%s %d\n",pAlg->GetName(),optThreads);
			fclose(fp);
			
			cout << endl << "Optimal thread count is " << optThreads << " (speed = " << optSpeed << " MHz)" << endl;
			threads = optThreads;
		}
		
		//Avoid overkill for short WUs
		int totalHashes = static_cast<int>(end.toInt() - start.toInt());
		if(threads*blocks > totalHashes)
			blocks = ceil(static_cast<float>(totalHashes) / threads);
			
		//Loop until we're done with the work unit
		while(!bDoneWithWU)
		{
			/*
			cout << "Starting kernel with " << blocks << " blocks of " << threads << " threads... " << endl;
			cout << "\t";
			for(int i=0;i<guesslen; i++)
				cout << start.digits[i] << " ";
			cout << endl;
			*/
		
			//Copy start guess to GPU memory
			double tStart=GetTime();
			if(cudaSuccess != (err = cudaMemcpy(d_startguess, startdigits, guesslen * sizeof(int), cudaMemcpyHostToDevice)) )
				throw string("Failed to copy start-guess buffer: ") + cudaGetErrorString(err);
			if(cudaSuccess != (err = cudaMemcpy(d_bCracked, &zero, sizeof(int), cudaMemcpyHostToDevice)) )
				throw string("Failed to copy bCracked: ") + cudaGetErrorString(err);
		
			//Process it
			DoHash(
				d_startguess,
				d_target,
				d_charset,
				(int)pData->charset.length(),
				d_cracked,
				d_bCracked,
				guesslen,
				blocks,
				threads);
		
			//Bump our index while waiting for GPU
			start.AddWithSaturation(blocks*threads);
			
			//See what came of it
			int bCracked = 0;
			if(cudaSuccess != (err = cudaMemcpy(&bCracked, d_bCracked, sizeof(int), cudaMemcpyDeviceToHost)) )
				throw string("Failed to copy cracked flag: ") + cudaGetErrorString(err);
			//printf("\tbCracked = %d\n",bCracked);
			if(bCracked)
			{
				//We found it! Copy it back from the device.
				char* sCracked = new char[guesslen];
				if(cudaSuccess != (err = cudaMemcpy(sCracked, d_cracked, guesslen, cudaMemcpyDeviceToHost)) )
					throw string("Failed to copy cracked value: ") + cudaGetErrorString(err);
					
				//Save the string and quit the crack loop
				pData->crackval.clear();
				for(int q=0; q<guesslen; q++)
					pData->crackval += static_cast<char>(sCracked[q]);
				pData->bFound=true;
				bDoneWithWU=true;
				delete[] sCracked;
				break;
			}
			
			/*
			double tEnd = GetTime();
			double dt=tEnd-tStart;
			float speed = static_cast<float>(threads*blocks)/(1E6 * dt);
			cout << "\tdone (in " << dt << " seconds, " << speed << " MHz)" << endl;
			fflush(stdout);
			*/
			
			//Done with last guess? Quit
			if(! (start < end) )
				break;
		}

		//Clean up
		delete[] btarget;
		cudaFree(d_cracked);
		cudaFree(d_bCracked);
		cudaFree(d_cracked);
		cudaFree(d_charset);
		cudaFree(d_startguess);
	}
	catch(std::string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
		exit(1);
	}

	//Done
	THREAD_RETURN(0);
}

bool Initialize()
{
	//See how many devices we have
	int count;
	if(cudaSuccess != cudaGetDeviceCount(&count))
	{
		cout << "Failed to get CUDA device count" << endl;
		return false;
	}

	//Print some info, hiding settings we don't care about for now
	cudaDeviceProp prop;
	for(int i=0;i<count;i++)
	{
		cudaGetDeviceProperties(&prop,i);
		cout << "Device " << i << " : " << prop.name << endl;
		/*cout << "\tGlobal mem (MB):       " << prop.totalGlobalMem / (1024*1024) << endl;
		//cout << "\tShared mem per block:  " << prop.sharedMemPerBlock << endl;
		cout << "\tRegs per block:        " << prop.regsPerBlock << endl;
		//cout << "\tWarp size:             " << prop.warpSize << endl;
		//cout << "\tMem pitch:             " << prop.memPitch << endl;
		cout << "\tMax threads per block: " << prop.maxThreadsPerBlock << endl;
		//cout << "\tConst mem:             " << prop.totalConstMem << endl;
		cout << "\tCompute capability:    " << prop.major << "." << prop.minor << endl;
		cout << "\tClock rate (MHz):      " << (float)prop.clockRate / 1000 << endl;
		cout << "\tMultiprocessor count:  " << prop.multiProcessorCount << endl;
		*/
	}
 
	return true;
}

void Cleanup()
{
	//No global cleanup - CUDA resources will be automatically freed at termination
}

int GetCrackType()
{
	return CRACK_TYPE_GPU;
}

int GetMaxThreads()
{
	//One thread per GPU
	int count;
	if(cudaSuccess != cudaGetDeviceCount(&count))
	{
		cout << "Failed to get CUDA device count" << endl;
		exit(-1);
	}
	return count;
}

ZTHREADPROC GetComputeThread()
{
	//Get our compute thread
	return internalComputeThreadProc;
}

int GetOptimalThreadCount(HashingAlgorithm* pAlg)
{
	//Do we have it already? Take a look;
	ifstream fin("cudathreads.conf");
	if(!fin)
		return 0;
	while(!fin.eof())
	{
		char name[128];
		int threads;
		fin >> name;
		fin >> threads;
		if(string(name) == pAlg->GetName())
			return threads;
	}
	return 0;
}

double GetTime()
{
#if LINUX
	timespec t;
	clock_gettime(CLOCK_REALTIME,&t);
	double d = static_cast<double>(t.tv_nsec) / 1E9f;
	d += t.tv_sec;
	return d;

#elif BSD
	timeval t;
	gettimeofday(&t, NULL);
	double d = static_cast<double>(t.tv_usec) / 1E6f;
	d += t.tv_sec;
	return d;

#elif WINDOWS
	static __int64 freq;
	static bool init=false;
	if(!init)
	{
		init=true;
		QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	}

	__int64 t;
	QueryPerformanceCounter((LARGE_INTEGER*)&t);
	return static_cast<double>(t)/freq;
#else
#error Unrecognized platform. Please define either LINUX, BSD, or WINDOWS.
#endif
}
