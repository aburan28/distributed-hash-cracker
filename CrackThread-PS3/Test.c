/************************************************************************************
 * Distributed Hash Cracker	v2.0													*
 *																					*
 * Copyright (c) 2009 RPISEC														*
 * All rights reserved.																*
 *																					*
 * Redistribution and use in source and binary forms, with or without				*
 * modification, are permitted provided that the following conditions are met:		*
 *     *Redistributions of source code must retain the above copyright				*
 *      notice, this list of conditions and the following disclaimer.				*
 *     *Redistributions in binary form must reproduce the above copyright			*
 *      notice, this list of conditions and the following disclaimer in the			*
 *      documentation and/or other materials provided with the distribution.		*
 *     *Neither the name of RPISEC nor the											*
 *      names of its contributors may be used to endorse or promote products		*
 *      derived from this software without specific prior written permission.		*
 *																					*
 * THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY								*
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED		*
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE			*
 * DISCLAIMED. IN NO EVENT SHALL RPISEC BE LIABLE FOR ANY							*
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES		*
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;		*
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND		*
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT		*
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS	*
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.						*
 ************************************************************************************
 *																					*
 * Test.c - Cell SPU thread for testing hashes against the target					*
 *																					*
 ************************************************************************************
 */

#include <stdio.h>
#include <spu_mfcio.h>

//#define PROFILING

void *aligned_malloc(size_t size, size_t align_size)
{
  char *ptr,*ptr2,*aligned_ptr;
  int align_mask = align_size - 1;

  ptr=(char *)malloc(size + align_size + sizeof(int));
  if(ptr==NULL) return(NULL);

  ptr2 = ptr + sizeof(int);
  aligned_ptr = ptr2 + (align_size - ((size_t)ptr2 & align_mask));


  ptr2 = aligned_ptr - sizeof(int);
  *((int *)ptr2)=(int)(aligned_ptr - ptr);

  return(aligned_ptr);
}

void aligned_free(void *ptr)
{
  int *ptr2=(int *)ptr - 1;
  ptr -= *ptr2;
  free(ptr);
}

#define PROF_SPEED 79800000

int main(unsigned long long speid)
{
	unsigned int prevptr;			//Pointer to previous node's output
	int blocksize;					//Number of hashes per pipeline block
	unsigned char* outbuf[2];		//Output buffers
	unsigned char* inbuf[2];		//Input buffers
	unsigned int hashsize;			//Size of each hash
	volatile unsigned char __attribute__((aligned(16))) target[128];	//Our target hash
	unsigned int rptr;				//General remote pointer
	int tag = 1;
	unsigned int rinbufs[2];		//Remote input buffers
	unsigned int pBlockDone;		//Write true to this once DMA transfer is done
	int iCurrentBuf = 0;
	int i;							//Generic loop counter
	int __attribute__((aligned(16))) one = 1;	//The value 1
	
	volatile int __attribute__((aligned(16))) bInputReady = 0;			//Set to true by the previous pipeline stage when
											//it has data ready for us to pick up by DMA
	volatile int __attribute__((aligned(16))) bNextStageDone = 1;		//Set to true by the next pipeline stage when it's
											//done with the block (i.e. we can now write to it)
	unsigned int pout;				//Output pointer
	char __attribute__((aligned(16))) outstr[64];
											
	int guesslen;
	int memsize;
	int found = 0;
								
#ifdef PROFILING
	//Profiling stuff
	long long profStartVal = 1000000000;
	long long profEndVal;
	float tTest=0;
	float tWait=0;
	float tDMA=0;
#endif
	
	//Get our block size
	blocksize = spu_read_in_mbox();
	
	//Allocate output buffer
	//outbuf[0] = (unsigned char*)aligned_malloc(blocksize * 1, 16);
	//outbuf[1] = (unsigned char*)aligned_malloc(blocksize * 1, 16);
	outbuf[0] = (unsigned char*)aligned_malloc(64, 16);
	outbuf[1] = (unsigned char*)aligned_malloc(64, 16);
	
	//Allocate input buffer
	memsize = blocksize * (32 + hashsize);
	while(memsize % 16 != 0)
		memsize++;
	inbuf[0] = (unsigned char*)aligned_malloc(memsize, 16);
	inbuf[1] = (unsigned char*)aligned_malloc(memsize, 16);
	
	//Send output pointers to PPE
	spu_write_out_mbox((unsigned int)outbuf[0]);
	spu_write_out_mbox((unsigned int)outbuf[1]);
	
	//Send status pointers to PPE
	spu_write_out_mbox((unsigned int)&bInputReady);
	spu_write_out_mbox((unsigned int)&bNextStageDone);
	
	//Get guess length
	guesslen = spu_read_in_mbox();
	
	//Get hash size
	hashsize = spu_read_in_mbox();
		
	//Get target hash
	rptr = spu_read_in_mbox();
	mfc_get(target, rptr, 128, tag, 0, 0);
	
	//Get output buffer
	pout = spu_read_in_mbox();
	
	//Wait until DMA reads are done
	mfc_write_tag_mask(1 << tag);
	mfc_read_tag_status_all();
	
	//Read remote input buffer pointers
	rinbufs[0] = spu_read_in_mbox();
	rinbufs[1] = spu_read_in_mbox();
	
	pBlockDone = spu_read_in_mbox();
	
	//Time to start work! Wait until our first data blob is ready
	while(0 == bInputReady)
	{}
	bInputReady=0;	//Clear the flag
	mfc_get(inbuf[iCurrentBuf], rinbufs[iCurrentBuf], memsize , tag, 0, 0);
	
	//Actual test loop
	while(bInputReady != 2)
	{
		unsigned char* buf = inbuf[iCurrentBuf];
		
		//If the previous DMA operation isn't done, wait until it is.
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		mfc_write_tag_mask(1 << tag);
		mfc_read_tag_status_all();
				
		//Tell the hash node we got it
		mfc_put(&one, pBlockDone, 4, tag, 0, 0);
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tDMA += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
	
		//Stall until we have input
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		while(0 == bInputReady)
		{}
		if(bInputReady != 2)
		{			
			//We have work. Clear the flag and start DMAing it in.
			bInputReady = 0;
			mfc_get(inbuf[1-iCurrentBuf], rinbufs[1-iCurrentBuf], memsize, tag, 0, 0);
		}
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tWait += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
		
		//At this point, we have a work unit in local storage and one in the pipe.
		//Process the one in local storage.
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		for(i=0;i<blocksize;i++)
		{
			if(0 == memcmp(buf + hashsize*i, target, hashsize) )
			{
				char* str = buf + (hashsize * blocksize) + (guesslen * i);
				str[guesslen]='\0';
				
				//DMA the cracked hash to the PPE
				strcpy(outstr,str);
				mfc_put(outstr, pout, 64, tag, 0, 0);
				mfc_write_tag_mask(1 << tag);
				mfc_read_tag_status_all();
							
				bInputReady = 2;	//tell the loop to quit
				found = 1;
				break;
			}
		}
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tTest += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
		
		//Swap buffers
		iCurrentBuf = 1-iCurrentBuf;
	}
	
#ifdef PROFILING
	printf("TESTING:\nWork = %f\nStall = %f\nDMA=%f\n\n",tTest,tWait,tDMA);
#endif
	
	//If we did NOT get a termination message from the other pipeline,
	//send termination status back to the master
	//0 = normal termination, didn't find
	//1 = found it
	spu_write_out_mbox(found);
	
	//Barrier until everyone is finished
	spu_write_out_mbox(1);
	spu_read_in_mbox();

	//Clean up
	for(i=0;i<2;i++)
	{
		aligned_free(inbuf[i]);
		aligned_free(outbuf[i]);
	}
	
	return 0;
} 
