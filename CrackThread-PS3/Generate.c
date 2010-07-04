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
 * Generate.c - Cell SPU thread for generating candidate values						*
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

#define unroll_4x(s) s s s s

#define PROF_SPEED 79800000

int main(unsigned long long speid)
{
	unsigned int prevptr[2];		//Pointer to previous node's output
	int blocksize;					//Number of hashes per pipeline block
	unsigned char* outbuf[2];		//Output buffers
	int csize;						//Character set size
	volatile unsigned char __attribute__((aligned(16))) charset[257];
	int tag = 1;					//DMA request tag
	unsigned int rptr;				//Generic remote pointer
	int guesslen;					//Length of our guesses
	volatile unsigned char __attribute__((aligned(16))) sspace[128];		//Our search space (start=0...63; end=64...128)
	unsigned int pRemoteInputReady;	//We set this to true when we have input ready for the next pipeline stage
	unsigned int i,j;				//Generic loop counters
	int iCurrentBuf = 0;			//Current buffer we're working with
	unsigned int __attribute__((aligned(16))) one = 1;			//The value 1, used for DMA sync
	unsigned int __attribute__((aligned(16))) two= 2;			//The value 2, used for DMA sync
	
	volatile int __attribute__((aligned(16))) bInputReady = 0;			//Set to true by the previous pipeline stage when
											//it has data ready for us to pick up by DMA
	volatile int __attribute__((aligned(16))) bNextStageDone = 1;		//Set to true by the next pipeline stage when it's
											//done with the block (i.e. we can now write to it)
	int bDone = 0;
	volatile unsigned char* start = &sspace[0];
	volatile unsigned char* end = &sspace[64];
	int bmax = 0;
	int guessbound;	//guesslen - 1
	int blockcount;
	int iBlock;
	int bDidLast=0;
	
	int dclen;
	char* tbl = NULL;
	
#ifdef PROFILING
	//Profiling stuff
	long long profStartVal = 1000000000;
	long long profEndVal;
	float tGenerate=0;
	float tWait=0;
	float tDMA=0;
#endif
	
	//Get our block size
	blocksize = spu_read_in_mbox();
	
	//Allocate output buffers
	outbuf[0] = (unsigned char*)aligned_malloc(blocksize * 32,16);
	outbuf[1] = (unsigned char*)aligned_malloc(blocksize * 32,16);
	
	//Send output pointers to PPE
	spu_write_out_mbox((unsigned int)outbuf[0]);
	spu_write_out_mbox((unsigned int)outbuf[1]);
	
	//Send status pointers to PPE
	spu_write_out_mbox((unsigned int)&bInputReady);
	spu_write_out_mbox((unsigned int)&bNextStageDone);
	
	//Get block count and character set size
	blockcount = spu_read_in_mbox();
	csize = spu_read_in_mbox();
	
	//Read entire an 256-byte block for charset, regardless of actual size
	rptr = spu_read_in_mbox();
	mfc_get(charset, rptr, 256, tag, 0, 0);
	
	//Get guess length, must be <=16
	guesslen = spu_read_in_mbox();
	if(guesslen > 16)
		abort();
	
	//Read search space
	rptr = spu_read_in_mbox();
	mfc_get(sspace, rptr, 128, tag, 0, 0);
	
	//Wait until DMA reads are done
	mfc_write_tag_mask(1 << tag);
	mfc_read_tag_status_all();
	
	//Get remote input-ready pointer
	pRemoteInputReady = spu_read_in_mbox();
	
	//Generate lookup table
	dclen = 2 * csize;
	tbl = aligned_malloc(csize * csize * 2,16);
	for(i=0;i<csize;i++)
	{
		char* row = tbl + (dclen * i);
		char ch = charset[i];
		for(j=0;j<csize;j++)
		{
			row[2*j] = ch;
			row[2*j + 1] = charset[j];
		}
	}
	
	//Loop while we haven't finished the WU
	bmax = blocksize* guesslen;
	guessbound = guesslen - 1;
	iBlock = 0;
	while(iBlock < blockcount && !bDidLast)
	{
		if(bInputReady)
		{
			printf("SPE: Received termination signal");
			break;
		}
	
		//Get the current buffer
		unsigned char* pBuf = outbuf[iCurrentBuf];
	
		//Generate a block of data
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		int base=0,k;
		do
		{
			//Generate guesses
			for(k=0;k<guesslen;k++)
				pBuf[base+k] = charset[start[k]];
			pBuf[base+guesslen]='\0';
					
			if(!bDone)
			{
				if(iBlock >= blockcount)
					bDidLast=1;
			
				start[guessbound]++;
				for(k=guessbound;k>=0;k--)
				{
					if(start[k] != csize)
						break;
				
					start[k] = 0;
					
					if(k>0)
						start[k-1]++;
					else
						bDone = 1;
				}
			}
			base+=guesslen;			
		} while(base < bmax);
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tGenerate += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
				
		//Wait until the next stage is ready for more work
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		while(!bNextStageDone)
		{}
		bNextStageDone = 0;
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tWait += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
			
		//Tell the next stage work is on the way
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		mfc_put((void*)&one, pRemoteInputReady, 4, tag, 0, 0);
		mfc_write_tag_mask(1 << tag);
		mfc_read_tag_status_all();
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tDMA += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
				
		//Switch buffers. At this point, the old buffer is
		//being DMAed to the next pipe stage
		iCurrentBuf = 1-iCurrentBuf;
		
		//Bump block counter
		iBlock ++;
	}
	
#ifdef PROFILING
	printf("GENERATION:\nWork = %f\nStall = %f\nDMA=%f\n\n",tGenerate,tWait,tDMA);
#endif
	
	//Tell the hash to quit
	mfc_put((void*)&two, pRemoteInputReady, 4, tag, 0, 0);
	
	//Barrier until everyone is finished
	spu_write_out_mbox(1);
	spu_read_in_mbox();
	
	//Clean up
	aligned_free(tbl);
	aligned_free(outbuf[0]);
	aligned_free(outbuf[1]);
	return 0;
} 
