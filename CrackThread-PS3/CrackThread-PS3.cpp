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
* CrackThread-PS3.cpp - crack thread for PlayStation 3                        *
*                                                                             *
******************************************************************************/

#include "CrackThread-PS3.h"
#include "../Cracker-common/BaseNInteger.h"
#include "../CPUCoreHashes/HashingAlgorithm.h"
#include "../ComputeNode/CrackThread.h"

#include <libspe2.h>
#include <math.h>

using namespace std;

//Generic pthread callback that sits around and waits for the SPE to finish
void *ppu_pthread_function(void *arg);

ZTHREADPROC GetComputeThread()
{
	return internalComputeThreadProc;
}

THREAD_PROTOTYPE(internalComputeThreadProc,_pData)
{
	try
	{
		//Get the hash file name
		CrackThreadData* pData = reinterpret_cast<CrackThreadData*>(_pData);
		HashingAlgorithm* pAlg = pData->pAlg;	//only used for reference (i.e. alg name, hash size)
		string hfname=pAlg->GetName();
		hfname += ".spu";
			
		//Load the appropriate SPU images
		spe_program_handle_t* hGenerate = spe_image_open("Generate.spu");
		if(!hGenerate)
			throw string("Failed to open SPE image \"Generate.spu\"");
		spe_program_handle_t* hHash = spe_image_open(hfname.c_str());
		if(!hHash)
			throw string("Failed to open SPU image \"") + hfname + "\"";
		spe_program_handle_t* hTest = spe_image_open("Test.spu");
		if(!hTest)
			throw string("Failed to open SPE image \"Test.spu\"");
			
		//Create the contexts and get local storage addresses
		spe_context_ptr_t context[3];
		void* lsptr[3];	
		for(int i=0;i<3;i++)
		{
			if(NULL == (context[i] = spe_context_create(0,NULL)))
				throw string("Failed to create SPU context\n");
			lsptr[i]=spe_ls_area_get(context[i]);
		}
		
		//Set the programs
		if(-1 == spe_program_load(context[0],hGenerate))
			throw string("Failed to load generation program\n");
		if(-1 == spe_program_load(context[1],hHash))
			throw string("Failed to load hash program\n");
		if(-1 == spe_program_load(context[2],hTest))
			throw string("Failed to load testing program\n");
			
		//Cache some parameters
		string target=pData->target;
		BaseNInteger start=pData->start;
		BaseNInteger end=pData->end;
		string _charset = pData->charset;
		char __attribute__((aligned(16)) charset[256];	//longest possible charset
		if(_charset.length() > 256)
			throw string("Charset must be under 256 characters");
		for(unsigned int i=0;i<_charset.length();i++)
			charset[i]=_charset[i];
		volatile bool& bDoneWithWU = *pData->bDoneWithWU;
		int tid = pData->tid;
		
		//Start the threads. Tell each one some basic info.
		pthread_t pthread[3];
		unsigned int blocksize = 512;
		unsigned int outbufs[3][2];		//SPU pointers
		unsigned int inready[3];
		unsigned int blockdone[3];
		float total = end.toInt() - start.toInt();
		unsigned int blocks = static_cast<unsigned int>(ceil(total / blocksize));
		for(int i=0;i<3;i++)
		{
			pthread_create(&pthread[i],NULL,&ppu_pthread_function,context[i]);
			usleep(100);	//wait 0.1ms for it to initialize
		
			//Send it the block size
			spe_in_mbox_write(context[i],&blocksize,1,SPE_MBOX_ALL_BLOCKING);
			
			//Wait for it to allocate two memory blocks and send pointers back.
			while(spe_out_mbox_status(context[i]) == 0) 
			{}
			spe_out_mbox_read(context[i],&outbufs[i][0],1);
			while(spe_out_mbox_status (context[i]) == 0) 
			{}
			spe_out_mbox_read(context[i],&outbufs[i][1],1);
			
			//Get a pointer to the "input ready" flag
			//which will be overwritten by the PREVIOUS node when it's done.
			while(spe_out_mbox_status(context[i]) == 0) 
			{}
			spe_out_mbox_read(context[i],&inready[i],1);
			
			//Get a pointer to the "done with block" flag
			//which will be overwritten by the NEXT node when it's done.
			while(spe_out_mbox_status(context[i]) == 0) 
			{}
			spe_out_mbox_read(context[i],&blockdone[i],1);
		}
		
		//Send block count to generation node
		spe_in_mbox_write(context[0],&blocks,1,SPE_MBOX_ALL_BLOCKING);
		
		//Send character set pointer and size to generation node
		unsigned int csize = _charset.length();
		unsigned int pCharset = reinterpret_cast<unsigned int>(&charset[0]);
		spe_in_mbox_write(context[0],&csize,1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[0],&pCharset,1,SPE_MBOX_ALL_BLOCKING);
		
		//Send guess length to all nodes
		unsigned int dlen = start.GetSize();
		spe_in_mbox_write(context[0],&dlen,1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[1],&dlen,1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[2],&dlen,1,SPE_MBOX_ALL_BLOCKING);
		
		//Send search space to generation node
		char __attribute__((aligned(16))) ssbuf[128];
		char* pstart = &ssbuf[0];
		char* pend = &ssbuf[64];
		unsigned int upstart = reinterpret_cast<unsigned int>(pstart);
		for(unsigned int i=0;i<dlen;i++)
		{
			pstart[i] = start.digits[i];
			pend[i] = end.digits[i];
		}
		spe_in_mbox_write(context[0],&upstart,1,SPE_MBOX_ALL_BLOCKING);
		
		//Send hash size to test node
		unsigned int hsize = pAlg->GetHashLength();
		spe_in_mbox_write(context[2],&hsize,1,SPE_MBOX_ALL_BLOCKING);
		
		//Send target to test node
		unsigned char __attribute__((aligned(16)) btarget[128];
		unsigned int pbtarget = reinterpret_cast<unsigned int>(&btarget[0]);
		for(unsigned int i=0;i<hsize;i++)
		{
			int b;
			sscanf(target.substr(i*2,2).c_str(),"%x",&b);
			btarget[i]=b;
		}
		spe_in_mbox_write(context[2],&pbtarget,1,SPE_MBOX_ALL_BLOCKING);		
		
		//Send output buffer to test node
		char __attribute__((aligned(16))) strCrackVal[64];
		unsigned int pcv = reinterpret_cast<unsigned int>(&strCrackVal[0]);
		spe_in_mbox_write(context[2],&pcv,1,SPE_MBOX_ALL_BLOCKING);		
		
		//Set up pointers
		for(int i=0;i<3;i++)
		{
			unsigned int q = reinterpret_cast<unsigned int>(lsptr[i]);
			outbufs[i][0] += q;
			outbufs[i][1] += q;
			inready[i] += q;
			blockdone[i] += q;
		}
		
		//Hash and test nodes need pointers to the previous node's output buffers
		spe_in_mbox_write(context[1],&outbufs[0][0],1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[1],&outbufs[0][1],1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[2],&outbufs[1][0],1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[2],&outbufs[1][1],1,SPE_MBOX_ALL_BLOCKING);
		
		//Hash and test nodes need pointers to the previous node's block-done flags
		spe_in_mbox_write(context[1],&blockdone[0],1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[2],&blockdone[1],1,SPE_MBOX_ALL_BLOCKING);
		
		//Generation and hash nodes need pointers to the next node's input-ready flags
		spe_in_mbox_write(context[0],&inready[1],1,SPE_MBOX_ALL_BLOCKING);
		spe_in_mbox_write(context[1],&inready[2],1,SPE_MBOX_ALL_BLOCKING);
		
		//At this point, the crack will begin.
		
		//Wait until we get a termination status from the testing thread.
		//If we get a bDoneWithWU signal from the other thread, flush
		//the pipeline and quit.
		unsigned int term;
		while(spe_out_mbox_status(context[2]) == 0)
		{
			//If we're done, cancel this crack and clean up
			if(bDoneWithWU && term != 1)
			{
				for(int i=0;i<3;i++)
					pthread_cancel(pthread[i]);
				spe_image_close(hGenerate);
				spe_image_close(hHash);
				spe_image_close(hTest);
				THREAD_RETURN(0);
			}
			usleep(1000);
		}
		spe_out_mbox_read(context[2],&term,1);
	
		//If it's 1, we found it. Let the world know!
		if(term == 1)
		{
			bDoneWithWU = true;
			pData->bFound = true;
			pData->crackval = strCrackVal;
			
			for(int i=0;i<3;i++)
			pthread_cancel(pthread[i]);
			spe_image_close(hGenerate);
			spe_image_close(hHash);
			spe_image_close(hTest);
			THREAD_RETURN(0);
		}
			
		//Barrier until everyone is done
		unsigned int temp;
		for(int i=0;i<3;i++)
		{	
			while(spe_out_mbox_status(context[i]) == 0)
			{}
			spe_out_mbox_read(context[i],&temp,1);
		}
		for(int i=0;i<3;i++)
			spe_in_mbox_write(context[i],&temp,1,SPE_MBOX_ALL_BLOCKING);
			
		//Wait for the threads to finish and clean up
		for(int i=0;i<3;i++)
		{
			pthread_join(pthread[i],NULL);
			spe_context_destroy(context[i]);
		}
				
		//Clean up SPE resources
		spe_image_close(hGenerate);
		spe_image_close(hHash);
		spe_image_close(hTest);
	}
	catch(std::string err)
	{
		cerr << "ERROR: " << err.c_str() << endl;
		exit(1);
	}
	
	//cout << "----------" << endl;

	THREAD_RETURN(0);
}

void *ppu_pthread_function(void *arg)
{
	unsigned int entry = SPE_DEFAULT_ENTRY;
	spe_context_run(reinterpret_cast<spe_context*>(arg),&entry,0,NULL,NULL,NULL);
	pthread_exit(NULL);
}