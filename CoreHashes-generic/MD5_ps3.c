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
 * MD5_ps3.c - Cell SPU thread for MD5 hash											*
 *																					*
 ************************************************************************************
 */

#include <stdio.h>
#include <spu_mfcio.h>
#include <spu_intrinsics.h>

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

unsigned int ByteSwapDword(unsigned int i)
{
	unsigned char c1 = i & 255,
		c2 = (i >> 8) & 255,
		c3 = (i >> 16) & 255,
		c4 = (i >> 24) & 255;
	return ((unsigned int)c1 << 24) + ((unsigned int)c2 << 16) + ((unsigned int)c3 << 8) + c4;
}

#define PROF_SPEED 79800000

//This is where the fun stuff starts...
//We have to emulate this since it doesn't exist in the instruction set...
#define spu_not(x) spu_xor(x, allones);

#define VAR(num,v) v##num

//#define F(X,Y,Z) (((X) & (Y)) | (~(X) & (Z)))
#define md5round_f(a,b,c,d,index,shamt,t,num) \
	VAR(num,temp3) = VAR(num,x)[index];					/* prefetch x[k] */ \
	VAR(num,temp1) = spu_and(VAR(num,b),VAR(num,c)); \
	VAR(num,temp2) = spu_not(VAR(num,b)); \
	VAR(num,temp2) = spu_and(VAR(num,temp2), VAR(num,d)); \
	VAR(num,temp1) = spu_or(VAR(num,temp1) , VAR(num,temp2)); \
	md5round(VAR(num,a),VAR(num,b),index,shamt,t,num);

//#define G(X,Y,Z) (((X) & (Z)) | ((Y) & ~(Z)))	
#define md5round_g(a,b,c,d,index,shamt,t,num) \
	VAR(num,temp3) = VAR(num,x)[index];					/* prefetch x[k] */ \
	VAR(num,temp1) = spu_and(VAR(num,b),VAR(num,d)); \
	VAR(num,temp2) = spu_not(VAR(num,d)); \
	VAR(num,temp2) = spu_and(VAR(num,temp2), VAR(num,c)); \
	VAR(num,temp1) = spu_or(VAR(num,temp1), VAR(num,temp2)); \
	md5round(VAR(num,a),VAR(num,b),index,shamt,t,num);

//#define H(X,Y,Z) ((X) ^ (Y) ^ (Z))
#define md5round_h(a,b,c,d,index,shamt,t,num) \
	VAR(num,temp3) = VAR(num,x)[index];					/* prefetch x[k] */ \
	VAR(num,temp1) = spu_xor(VAR(num,b),VAR(num,c)); \
	VAR(num,temp1) = spu_xor(VAR(num,temp1),VAR(num,d)); \
	md5round(VAR(num,a),VAR(num,b),index,shamt,t,num);

//#define I(X,Y,Z) ((Y) ^ ((X) | ~(Z)))
#define md5round_i(a,b,c,d,index,shamt,t,num) \
	VAR(num,temp3) = VAR(num,x)[index];					/* prefetch x[k] */ \
	VAR(num,temp1) = spu_not(VAR(num,d)); \
	VAR(num,temp1) = spu_or(VAR(num,b),VAR(num,temp1)); \
	VAR(num,temp1) = spu_xor(VAR(num,temp1), VAR(num,c)); \
	md5round(VAR(num,a),VAR(num,b),index,shamt,t,num);

//#define md5round(a,b,c,d,k,s,i,f) a = ROTL(a + f(b,c,d) + x[k] + t[i],s) + b;
#define md5round(a,b,index,shamt,t,num) \
	VAR(num,temp1) = spu_add(a, VAR(num,temp1));			/* VAR(num,temp1) = a + f(b,c,d) */ \
	VAR(num,temp2) = spu_add(VAR(num,temp3), t);			/* VAR(num,temp2) = x[k] + t[i] */ \
	VAR(num,temp1) = spu_add(VAR(num,temp1), VAR(num,temp2));		/* VAR(num,temp1) = a + f(b,c,d) + x[k] + t[i] */ \
	VAR(num,temp1) = spu_rl(VAR(num,temp1), shamt);		/* VAR(num,temp1) = ROTL(a + f(b,c,d) + x[k] + t[i],s) */ \
	a = spu_add(VAR(num,temp1), b);

void print_vector(char *var, vector unsigned int val)
{
	printf("%s = {%08x, %08x, %08x, %08x}\n", var, spu_extract(val, 0), spu_extract(val, 1), spu_extract(val, 2), spu_extract(val, 3));
}

int main(unsigned long long speid)
{
	int blocksize;					//Number of hashes per pipeline block
	int hashsize = 16;				//Size of our hash
	unsigned char* outbuf[2];		//Output buffers
	unsigned char* inbuf[2];		//Input buffers
	unsigned char* inbufcpy[2];		//Copy of input buffers
	int guesslen;					//Length of guesses we're getting
	unsigned int rinbufs[2];		//Remote copies of our input buffers
	unsigned int pNextInputReady;	//We set this to true to tell the next pipeline stage we're ready for data
	unsigned int pBlockDone;		//We set this to true to tell the previous pipeline stage our DMA is done
	unsigned int iCurrentBuf = 0;	//Current buffer to work with
	int __attribute__((aligned(16))) one = 1;					//The value 1
	int tag = 1;					//DMA tag
	int i,j,k;						//generic loop counters
	int memsize;
	
#ifdef PROFILING
	//Profiling stuff
	long long profStartVal = 1000000000;
	long long profEndVal;
	float tHash=0;
	float tWait=0;
	float tDMA=0;
#endif
	
	//External data
	volatile int __attribute__((aligned(16))) bInputReady = 0;			//Set to true by the previous pipeline stage when
											//it has data ready for us to pick up by DMA
	volatile int __attribute__((aligned(16))) bNextStageDone = 1;		//Set to true by the next pipeline stage when it's
											//done with the block (i.e. we can now write to it)
	
	//Get our block size
	blocksize = spu_read_in_mbox();
	
	//Allocate input buffers
	int imemsize = (blocksize + 4) * 32 + 16;
	inbuf[0] = (unsigned char*)aligned_malloc(imemsize,16);
	inbuf[1] = (unsigned char*)aligned_malloc(imemsize,16);
	
	//Allocate output buffers
	memsize = (blocksize+4) * (32 + hashsize);
	while(memsize % 16 != 0)
		memsize++;
	outbuf[0] = (unsigned char*)aligned_malloc(memsize,16);
	outbuf[1] = (unsigned char*)aligned_malloc(memsize,16);
	inbufcpy[0] = outbuf[0] + (blocksize*hashsize);
	inbufcpy[1] = outbuf[1] + (blocksize*hashsize);
	
	//Send output pointers to PPE
	spu_write_out_mbox((unsigned int)outbuf[0]);
	spu_write_out_mbox((unsigned int)outbuf[1]);
	
	//Send status pointers to PPE
	spu_write_out_mbox((unsigned int)&bInputReady);
	spu_write_out_mbox((unsigned int)&bNextStageDone);
	
	//Get guess length
	guesslen = spu_read_in_mbox();
	
	//Get input buffer pointers
	rinbufs[0] = spu_read_in_mbox();
	rinbufs[1] = spu_read_in_mbox();
	
	//Get the next stage's block-done pointer
	pBlockDone = spu_read_in_mbox();
	
	//Get the next stage's data-ready pointer
	pNextInputReady = spu_read_in_mbox();
	
	//Time to start work! Wait until our first data blob is ready
	while(0 == bInputReady)
	{}
	bInputReady=0;	//Clear the flag
	mfc_get(inbuf[iCurrentBuf], rinbufs[iCurrentBuf], blocksize * 32, tag, 0, 0);
	
	//Loop until we get a terminate message
	while(bInputReady != 2)
	{
		unsigned char* buf = inbuf[iCurrentBuf];
		unsigned char* out = outbuf[iCurrentBuf];
	
		//If the previous DMA operation isn't done, wait until it is.
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		mfc_write_tag_mask(1 << tag);
		mfc_read_tag_status_all();
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tDMA += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
		
		//Tell the generation node we got it
		mfc_put(&one, pBlockDone, 4, tag, 0, 0);
	
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
			mfc_get(inbuf[1-iCurrentBuf], rinbufs[1-iCurrentBuf], blocksize * 32, tag, 0, 0);
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
		for(i=0;i<blocksize;i+=16)
		{
			char* pin = buf + (guesslen*i);
			char* pout = out + (hashsize*i);
			
			//Base (vector number) and offset (position inside vector) of padding
			int base=guesslen & 0xFC;
			int off=guesslen-base;
			unsigned int itemp1;
			
			//MD5 buffer - default to zero
			vec_uint4
				x1[16] = { {0,0,0,0} },
				x2[16] = { {0,0,0,0} },
				x3[16] = { {0,0,0,0} },
				x4[16] = { {0,0,0,0} };
				
			vec_uint4* px[] = { &x1[0], &x2[0], &x3[0], &x4[0] };
			
			//MD5 variables
			vec_uint4 
				A1 = {0x67452301, 0x67452301, 0x67452301, 0x67452301}, 
				A2 = {0x67452301, 0x67452301, 0x67452301, 0x67452301}, 
				A3 = {0x67452301, 0x67452301, 0x67452301, 0x67452301},
				A4 = {0x67452301, 0x67452301, 0x67452301, 0x67452301};
			vec_uint4
				B1 = {0xefcdab89, 0xefcdab89, 0xefcdab89, 0xefcdab89}, 
				B2 = {0xefcdab89, 0xefcdab89, 0xefcdab89, 0xefcdab89}, 
				B3 = {0xefcdab89, 0xefcdab89, 0xefcdab89, 0xefcdab89}, 
				B4 = {0xefcdab89, 0xefcdab89, 0xefcdab89, 0xefcdab89};
			vec_uint4
				C1 = {0x98badcfe, 0x98badcfe, 0x98badcfe, 0x98badcfe},
				C2 = {0x98badcfe, 0x98badcfe, 0x98badcfe, 0x98badcfe},
				C3 = {0x98badcfe, 0x98badcfe, 0x98badcfe, 0x98badcfe},
				C4 = {0x98badcfe, 0x98badcfe, 0x98badcfe, 0x98badcfe};
			vec_uint4
				D1 = {0x10325476, 0x10325476, 0x10325476, 0x10325476},
				D2 = {0x10325476, 0x10325476, 0x10325476, 0x10325476},
				D3 = {0x10325476, 0x10325476, 0x10325476, 0x10325476},
				D4 = {0x10325476, 0x10325476, 0x10325476, 0x10325476};
			
			//Used due to lack of a native spu_not instruction
			vec_uint4 allones = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
			
			//Temporary values used by md5 rounds
			vec_uint4
				temp11, temp12, temp13, temp14,
				temp21, temp22, temp23, temp24,
				temp31, temp32, temp33, temp34;
			
			//Copy and interleave data
			for(j=0;j<4;j++)
			{
				vec_uint4* x = px[j];
				
				char* in1 = pin + (j*4*guesslen);
				char* in2 = in1 + guesslen;
				char* in3 = in2 + guesslen;
				char* in4 = in3 + guesslen;
				
				for(k=0; k<guesslen; k+=4)
				{
					//Copy a dword of each input at a time, byte-swapping as it comes in
					int q = k/4;
					vec_uint4 tx = x[q];
					tx = spu_insert( (in1[3] << 24) | (in1[2] << 16) | (in1[1] << 8) | in1[0], tx, 0);
					tx = spu_insert( (in2[3] << 24) | (in2[2] << 16) | (in2[1] << 8) | in2[0], tx, 1);
					tx = spu_insert( (in3[3] << 24) | (in3[2] << 16) | (in3[1] << 8) | in3[0], tx, 2);
					x[q] = spu_insert( (in4[3] << 24) | (in4[2] << 16) | (in4[1] << 8) | in4[0], tx, 3);
									
					//Bump pointers
					in1 += 4;
					in2 += 4;
					in3 += 4;
					in4 += 4;
				}
			}
			
			
			//Zero out extra stuff
			base /= 4;
			itemp1 = ~ (0xFFFFFFFF << (8 * off));	//0s in all bits we want zeroed out
			temp11 = spu_splats( itemp1 );			//Vectorize it
			x1[base] = spu_and(x1[base], temp11);	//Merge it in
			x2[base] = spu_and(x2[base], temp11);	//Merge it in
			x3[base] = spu_and(x3[base], temp11);	//Merge it in
			x4[base] = spu_and(x4[base], temp11);	//Merge it in
								
			//Append padding bits
			itemp1 = 1 << (7 + 8*off);				//A single "1" bit at the correct position
			temp11 = spu_splats( itemp1 );			//Vectorize it
			x1[base] = spu_or(x1[base], temp11);		//Merge it in
			x2[base] = spu_or(x2[base], temp11);		//Merge it in
			x3[base] = spu_or(x3[base], temp11);		//Merge it in
			x4[base] = spu_or(x4[base], temp11);		//Merge it in
				
			//Append length
			itemp1 = guesslen * 8;					//Guess length in bits
			x1[14] = spu_splats( itemp1 );			//Vectorize it	
			x2[14] = spu_splats( itemp1 );			//Vectorize it	
			x3[14] = spu_splats( itemp1 );			//Vectorize it	
			x4[14] = spu_splats( itemp1 );			//Vectorize it	
			
			//Round 1
			md5round_f(A,B,C,D,0,7,0xd76aa478,1);	md5round_f(A,B,C,D,0,7,0xd76aa478,2);	md5round_f(A,B,C,D,0,7,0xd76aa478,3);	md5round_f(A,B,C,D,0,7,0xd76aa478,4);
				md5round_f(D,A,B,C,1,12,0xe8c7b756,1);	md5round_f(D,A,B,C,1,12,0xe8c7b756,2);	md5round_f(D,A,B,C,1,12,0xe8c7b756,3);	md5round_f(D,A,B,C,1,12,0xe8c7b756,4);
				md5round_f(C,D,A,B,2,17,0x242070db,1);	md5round_f(C,D,A,B,2,17,0x242070db,2);	md5round_f(C,D,A,B,2,17,0x242070db,3);	md5round_f(C,D,A,B,2,17,0x242070db,4);
				md5round_f(B,C,D,A,3,22,0xc1bdceee,1);	md5round_f(B,C,D,A,3,22,0xc1bdceee,2);	md5round_f(B,C,D,A,3,22,0xc1bdceee,3);	md5round_f(B,C,D,A,3,22,0xc1bdceee,4);
			md5round_f(A,B,C,D,4,7,0xf57c0faf,1);	md5round_f(A,B,C,D,4,7,0xf57c0faf,2);	md5round_f(A,B,C,D,4,7,0xf57c0faf,3);	md5round_f(A,B,C,D,4,7,0xf57c0faf,4);
				md5round_f(D,A,B,C,5,12,0x4787c62a,1);	md5round_f(D,A,B,C,5,12,0x4787c62a,2);	md5round_f(D,A,B,C,5,12,0x4787c62a,3);	md5round_f(D,A,B,C,5,12,0x4787c62a,4);
				md5round_f(C,D,A,B,6,17,0xa8304613,1);	md5round_f(C,D,A,B,6,17,0xa8304613,2);	md5round_f(C,D,A,B,6,17,0xa8304613,3);	md5round_f(C,D,A,B,6,17,0xa8304613,4);
				md5round_f(B,C,D,A,7,22,0xfd469501,1);	md5round_f(B,C,D,A,7,22,0xfd469501,2);	md5round_f(B,C,D,A,7,22,0xfd469501,3);	md5round_f(B,C,D,A,7,22,0xfd469501,4);
			md5round_f(A,B,C,D,8,7,0x698098d8,1);	md5round_f(A,B,C,D,8,7,0x698098d8,2);	md5round_f(A,B,C,D,8,7,0x698098d8,3);	md5round_f(A,B,C,D,8,7,0x698098d8,4);
				md5round_f(D,A,B,C,9,12,0x8b44f7af,1);	md5round_f(D,A,B,C,9,12,0x8b44f7af,2);	md5round_f(D,A,B,C,9,12,0x8b44f7af,3);	md5round_f(D,A,B,C,9,12,0x8b44f7af,4);
				md5round_f(C,D,A,B,10,17,0xffff5bb1,1);	md5round_f(C,D,A,B,10,17,0xffff5bb1,2);	md5round_f(C,D,A,B,10,17,0xffff5bb1,3);	md5round_f(C,D,A,B,10,17,0xffff5bb1,4);
				md5round_f(B,C,D,A,11,22,0x895cd7be,1);	md5round_f(B,C,D,A,11,22,0x895cd7be,2);	md5round_f(B,C,D,A,11,22,0x895cd7be,3);	md5round_f(B,C,D,A,11,22,0x895cd7be,4);
			md5round_f(A,B,C,D,12,7,0x6b901122,1);	md5round_f(A,B,C,D,12,7,0x6b901122,2);	md5round_f(A,B,C,D,12,7,0x6b901122,3);	md5round_f(A,B,C,D,12,7,0x6b901122,4);
				md5round_f(D,A,B,C,13,12,0xfd987193,1);	md5round_f(D,A,B,C,13,12,0xfd987193,2);	md5round_f(D,A,B,C,13,12,0xfd987193,3);	md5round_f(D,A,B,C,13,12,0xfd987193,4);
				md5round_f(C,D,A,B,14,17,0xa679438e,1);	md5round_f(C,D,A,B,14,17,0xa679438e,2);	md5round_f(C,D,A,B,14,17,0xa679438e,3);	md5round_f(C,D,A,B,14,17,0xa679438e,4);
				md5round_f(B,C,D,A,15,22,0x49b40821,1);	md5round_f(B,C,D,A,15,22,0x49b40821,2);	md5round_f(B,C,D,A,15,22,0x49b40821,3);	md5round_f(B,C,D,A,15,22,0x49b40821,4);
			
			//Round 2
			md5round_g(A,B,C,D,1,5,0xf61e2562,1);	md5round_g(A,B,C,D,1,5,0xf61e2562,2);	md5round_g(A,B,C,D,1,5,0xf61e2562,3);	md5round_g(A,B,C,D,1,5,0xf61e2562,4);	
				md5round_g(D,A,B,C,6,9,0xc040b340,1);	md5round_g(D,A,B,C,6,9,0xc040b340,2);	md5round_g(D,A,B,C,6,9,0xc040b340,3);	md5round_g(D,A,B,C,6,9,0xc040b340,4);
				md5round_g(C,D,A,B,11,14,0x265e5a51,1);	md5round_g(C,D,A,B,11,14,0x265e5a51,2);	md5round_g(C,D,A,B,11,14,0x265e5a51,3);	md5round_g(C,D,A,B,11,14,0x265e5a51,4);
				md5round_g(B,C,D,A,0,20,0xe9b6c7aa,1);	md5round_g(B,C,D,A,0,20,0xe9b6c7aa,2);	md5round_g(B,C,D,A,0,20,0xe9b6c7aa,3);	md5round_g(B,C,D,A,0,20,0xe9b6c7aa,4);
			md5round_g(A,B,C,D,5,5,0xd62f105d,1);	md5round_g(A,B,C,D,5,5,0xd62f105d,2);	md5round_g(A,B,C,D,5,5,0xd62f105d,3);	md5round_g(A,B,C,D,5,5,0xd62f105d,4);
				md5round_g(D,A,B,C,10,9,0x02441453,1);	md5round_g(D,A,B,C,10,9,0x02441453,2);	md5round_g(D,A,B,C,10,9,0x02441453,3);	md5round_g(D,A,B,C,10,9,0x02441453,4);
				md5round_g(C,D,A,B,15,14,0xd8a1e681,1);	md5round_g(C,D,A,B,15,14,0xd8a1e681,2);	md5round_g(C,D,A,B,15,14,0xd8a1e681,3);	md5round_g(C,D,A,B,15,14,0xd8a1e681,4);
				md5round_g(B,C,D,A,4,20,0xe7d3fbc8,1);	md5round_g(B,C,D,A,4,20,0xe7d3fbc8,2);	md5round_g(B,C,D,A,4,20,0xe7d3fbc8,3);	md5round_g(B,C,D,A,4,20,0xe7d3fbc8,4);
			md5round_g(A,B,C,D,9,5,0x21e1cde6,1);	md5round_g(A,B,C,D,9,5,0x21e1cde6,2);	md5round_g(A,B,C,D,9,5,0x21e1cde6,3);	md5round_g(A,B,C,D,9,5,0x21e1cde6,4);	
				md5round_g(D,A,B,C,14,9,0xc33707d6,1);	md5round_g(D,A,B,C,14,9,0xc33707d6,2);	md5round_g(D,A,B,C,14,9,0xc33707d6,3);	md5round_g(D,A,B,C,14,9,0xc33707d6,4);
				md5round_g(C,D,A,B,3,14,0xf4d50d87,1);	md5round_g(C,D,A,B,3,14,0xf4d50d87,2);	md5round_g(C,D,A,B,3,14,0xf4d50d87,3);	md5round_g(C,D,A,B,3,14,0xf4d50d87,4);
				md5round_g(B,C,D,A,8,20,0x455a14ed,1);	md5round_g(B,C,D,A,8,20,0x455a14ed,2);	md5round_g(B,C,D,A,8,20,0x455a14ed,3);	md5round_g(B,C,D,A,8,20,0x455a14ed,4);
			md5round_g(A,B,C,D,13,5,0xa9e3e905,1);	md5round_g(A,B,C,D,13,5,0xa9e3e905,2);	md5round_g(A,B,C,D,13,5,0xa9e3e905,3);	md5round_g(A,B,C,D,13,5,0xa9e3e905,4);	
				md5round_g(D,A,B,C,2,9,0xfcefa3f8,1);	md5round_g(D,A,B,C,2,9,0xfcefa3f8,2);	md5round_g(D,A,B,C,2,9,0xfcefa3f8,3);	md5round_g(D,A,B,C,2,9,0xfcefa3f8,4);
				md5round_g(C,D,A,B,7,14,0x676f02d9,1);	md5round_g(C,D,A,B,7,14,0x676f02d9,2);	md5round_g(C,D,A,B,7,14,0x676f02d9,3);	md5round_g(C,D,A,B,7,14,0x676f02d9,4);
				md5round_g(B,C,D,A,12,20,0x8d2a4c8a,1);	md5round_g(B,C,D,A,12,20,0x8d2a4c8a,2);	md5round_g(B,C,D,A,12,20,0x8d2a4c8a,3);	md5round_g(B,C,D,A,12,20,0x8d2a4c8a,4);
			
			//Round 3
			md5round_h(A,B,C,D,5,4,0xfffa3942,1);	md5round_h(A,B,C,D,5,4,0xfffa3942,2);	md5round_h(A,B,C,D,5,4,0xfffa3942,3);	md5round_h(A,B,C,D,5,4,0xfffa3942,4);
				md5round_h(D,A,B,C,8,11,0x8771f681,1);	md5round_h(D,A,B,C,8,11,0x8771f681,2);	md5round_h(D,A,B,C,8,11,0x8771f681,3);	md5round_h(D,A,B,C,8,11,0x8771f681,4);
				md5round_h(C,D,A,B,11,16,0x6d9d6122,1);	md5round_h(C,D,A,B,11,16,0x6d9d6122,2);	md5round_h(C,D,A,B,11,16,0x6d9d6122,3);	md5round_h(C,D,A,B,11,16,0x6d9d6122,4);
				md5round_h(B,C,D,A,14,23,0xfde5380c,1);	md5round_h(B,C,D,A,14,23,0xfde5380c,2);	md5round_h(B,C,D,A,14,23,0xfde5380c,3);	md5round_h(B,C,D,A,14,23,0xfde5380c,4);
			md5round_h(A,B,C,D,1,4,0xa4beea44,1);	md5round_h(A,B,C,D,1,4,0xa4beea44,2);	md5round_h(A,B,C,D,1,4,0xa4beea44,3);	md5round_h(A,B,C,D,1,4,0xa4beea44,4);
				md5round_h(D,A,B,C,4,11,0x4bdecfa9,1);	md5round_h(D,A,B,C,4,11,0x4bdecfa9,2);	md5round_h(D,A,B,C,4,11,0x4bdecfa9,3);	md5round_h(D,A,B,C,4,11,0x4bdecfa9,4);
				md5round_h(C,D,A,B,7,16,0xf6bb4b60,1);	md5round_h(C,D,A,B,7,16,0xf6bb4b60,2);	md5round_h(C,D,A,B,7,16,0xf6bb4b60,3);	md5round_h(C,D,A,B,7,16,0xf6bb4b60,4);
				md5round_h(B,C,D,A,10,23,0xbebfbc70,1);	md5round_h(B,C,D,A,10,23,0xbebfbc70,2);	md5round_h(B,C,D,A,10,23,0xbebfbc70,3);	md5round_h(B,C,D,A,10,23,0xbebfbc70,4);
			md5round_h(A,B,C,D,13,4,0x289b7ec6,1);	md5round_h(A,B,C,D,13,4,0x289b7ec6,2);	md5round_h(A,B,C,D,13,4,0x289b7ec6,3);	md5round_h(A,B,C,D,13,4,0x289b7ec6,4);
				md5round_h(D,A,B,C,0,11,0xeaa127fa,1);	md5round_h(D,A,B,C,0,11,0xeaa127fa,2);	md5round_h(D,A,B,C,0,11,0xeaa127fa,3);	md5round_h(D,A,B,C,0,11,0xeaa127fa,4);
				md5round_h(C,D,A,B,3,16,0xd4ef3085,1);	md5round_h(C,D,A,B,3,16,0xd4ef3085,2);	md5round_h(C,D,A,B,3,16,0xd4ef3085,3);	md5round_h(C,D,A,B,3,16,0xd4ef3085,4);
				md5round_h(B,C,D,A,6,23,0x04881d05,1);	md5round_h(B,C,D,A,6,23,0x04881d05,2);	md5round_h(B,C,D,A,6,23,0x04881d05,3);	md5round_h(B,C,D,A,6,23,0x04881d05,4);
			md5round_h(A,B,C,D,9,4,0xd9d4d039,1);	md5round_h(A,B,C,D,9,4,0xd9d4d039,2);	md5round_h(A,B,C,D,9,4,0xd9d4d039,3);	md5round_h(A,B,C,D,9,4,0xd9d4d039,4);
				md5round_h(D,A,B,C,12,11,0xe6db99e5,1);	md5round_h(D,A,B,C,12,11,0xe6db99e5,2);	md5round_h(D,A,B,C,12,11,0xe6db99e5,3);	md5round_h(D,A,B,C,12,11,0xe6db99e5,4);
				md5round_h(C,D,A,B,15,16,0x1fa27cf8,1);	md5round_h(C,D,A,B,15,16,0x1fa27cf8,2);	md5round_h(C,D,A,B,15,16,0x1fa27cf8,3);	md5round_h(C,D,A,B,15,16,0x1fa27cf8,4);
				md5round_h(B,C,D,A,2,23,0xc4ac5665,1);	md5round_h(B,C,D,A,2,23,0xc4ac5665,2);	md5round_h(B,C,D,A,2,23,0xc4ac5665,3);	md5round_h(B,C,D,A,2,23,0xc4ac5665,4);
	
			//Round 4
			md5round_i(A,B,C,D,0,6,0xf4292244,1);	md5round_i(A,B,C,D,0,6,0xf4292244,2);	md5round_i(A,B,C,D,0,6,0xf4292244,3);	md5round_i(A,B,C,D,0,6,0xf4292244,4);
				md5round_i(D,A,B,C,7,10,0x432aff97,1);	md5round_i(D,A,B,C,7,10,0x432aff97,2);	md5round_i(D,A,B,C,7,10,0x432aff97,3);	md5round_i(D,A,B,C,7,10,0x432aff97,4);
				md5round_i(C,D,A,B,14,15,0xab9423a7,1);	md5round_i(C,D,A,B,14,15,0xab9423a7,2);	md5round_i(C,D,A,B,14,15,0xab9423a7,3);	md5round_i(C,D,A,B,14,15,0xab9423a7,4);
				md5round_i(B,C,D,A,5,21,0xfc93a039,1);	md5round_i(B,C,D,A,5,21,0xfc93a039,2);	md5round_i(B,C,D,A,5,21,0xfc93a039,3);	md5round_i(B,C,D,A,5,21,0xfc93a039,4);
			md5round_i(A,B,C,D,12,6,0x655b59c3,1);	md5round_i(A,B,C,D,12,6,0x655b59c3,2);	md5round_i(A,B,C,D,12,6,0x655b59c3,3);	md5round_i(A,B,C,D,12,6,0x655b59c3,4);
				md5round_i(D,A,B,C,3,10,0x8f0ccc92,1);	md5round_i(D,A,B,C,3,10,0x8f0ccc92,2);	md5round_i(D,A,B,C,3,10,0x8f0ccc92,3);	md5round_i(D,A,B,C,3,10,0x8f0ccc92,4);
				md5round_i(C,D,A,B,10,15,0xffeff47d,1);	md5round_i(C,D,A,B,10,15,0xffeff47d,2);	md5round_i(C,D,A,B,10,15,0xffeff47d,3);	md5round_i(C,D,A,B,10,15,0xffeff47d,4);
				md5round_i(B,C,D,A,1,21,0x85845dd1,1);	md5round_i(B,C,D,A,1,21,0x85845dd1,2);	md5round_i(B,C,D,A,1,21,0x85845dd1,3);	md5round_i(B,C,D,A,1,21,0x85845dd1,4);
			md5round_i(A,B,C,D,8,6,0x6fa87e4f,1);	md5round_i(A,B,C,D,8,6,0x6fa87e4f,2);	md5round_i(A,B,C,D,8,6,0x6fa87e4f,3);	md5round_i(A,B,C,D,8,6,0x6fa87e4f,4);	
				md5round_i(D,A,B,C,15,10,0xfe2ce6e0,1);	md5round_i(D,A,B,C,15,10,0xfe2ce6e0,2);	md5round_i(D,A,B,C,15,10,0xfe2ce6e0,3);	md5round_i(D,A,B,C,15,10,0xfe2ce6e0,4);
				md5round_i(C,D,A,B,6,15,0xa3014314,1);	md5round_i(C,D,A,B,6,15,0xa3014314,2);	md5round_i(C,D,A,B,6,15,0xa3014314,3);	md5round_i(C,D,A,B,6,15,0xa3014314,4);
				md5round_i(B,C,D,A,13,21,0x4e0811a1,1);	md5round_i(B,C,D,A,13,21,0x4e0811a1,2);	md5round_i(B,C,D,A,13,21,0x4e0811a1,3);	md5round_i(B,C,D,A,13,21,0x4e0811a1,4);
			md5round_i(A,B,C,D,4,6,0xf7537e82,1);	md5round_i(A,B,C,D,4,6,0xf7537e82,2);	md5round_i(A,B,C,D,4,6,0xf7537e82,3);	md5round_i(A,B,C,D,4,6,0xf7537e82,4);
				md5round_i(D,A,B,C,11,10,0xbd3af235,1);	md5round_i(D,A,B,C,11,10,0xbd3af235,2);	md5round_i(D,A,B,C,11,10,0xbd3af235,3);	md5round_i(D,A,B,C,11,10,0xbd3af235,4);
				md5round_i(C,D,A,B,2,15,0x2ad7d2bb,1);	md5round_i(C,D,A,B,2,15,0x2ad7d2bb,2);	md5round_i(C,D,A,B,2,15,0x2ad7d2bb,3);	md5round_i(C,D,A,B,2,15,0x2ad7d2bb,4);
				md5round_i(B,C,D,A,9,21,0xeb86d391,1);	md5round_i(B,C,D,A,9,21,0xeb86d391,2);	md5round_i(B,C,D,A,9,21,0xeb86d391,3);	md5round_i(B,C,D,A,9,21,0xeb86d391,4);

			//Add in the original values
			A1=spu_add(A1,0x67452301);	A2=spu_add(A2,0x67452301);	A3=spu_add(A3,0x67452301);	A4=spu_add(A4,0x67452301);
			B1=spu_add(B1,0xefcdab89);	B2=spu_add(B2,0xefcdab89);	B3=spu_add(B3,0xefcdab89);	B4=spu_add(B4,0xefcdab89);
			C1=spu_add(C1,0x98badcfe);	C2=spu_add(C2,0x98badcfe);	C3=spu_add(C3,0x98badcfe);	C4=spu_add(C4,0x98badcfe);
			D1=spu_add(D1,0x10325476);	D2=spu_add(D2,0x10325476);	D3=spu_add(D3,0x10325476);	D4=spu_add(D4,0x10325476);
					
			//and copy the output
			for(j=0;j<4;j++)
			{
				unsigned int* p = (unsigned int*)(pout + 16*j);
				p[0] = ByteSwapDword(spu_extract(A1,j));
				p[1] = ByteSwapDword(spu_extract(B1,j));
				p[2] = ByteSwapDword(spu_extract(C1,j));
				p[3] = ByteSwapDword(spu_extract(D1,j));
				
				p += 16;
				p[0] = ByteSwapDword(spu_extract(A2,j));
				p[1] = ByteSwapDword(spu_extract(B2,j));
				p[2] = ByteSwapDword(spu_extract(C2,j));
				p[3] = ByteSwapDword(spu_extract(D2,j));
				
				p += 16;
				p[0] = ByteSwapDword(spu_extract(A3,j));
				p[1] = ByteSwapDword(spu_extract(B3,j));
				p[2] = ByteSwapDword(spu_extract(C3,j));
				p[3] = ByteSwapDword(spu_extract(D3,j));
				
				p += 16;
				p[0] = ByteSwapDword(spu_extract(A4,j));
				p[1] = ByteSwapDword(spu_extract(B4,j));
				p[2] = ByteSwapDword(spu_extract(C4,j));
				p[3] = ByteSwapDword(spu_extract(D4,j));
			}
		}
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tHash += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
		
		//Copy the input buffer to the output copy.
		//This may overwrite a few "extra" hashes but we don't need them anyway
		memcpy(inbufcpy[iCurrentBuf],buf,blocksize*32);
		
		//Make sure we're not stepping on the next stage's toes, then
		//tell it we have a little something ready for processing
#ifdef PROFILING
		spu_write_decrementer(profStartVal);
#endif
		while(!bNextStageDone)
		{}
		bNextStageDone = 0;
		mfc_put(&one, pNextInputReady, 4, tag, 0, 0);
#ifdef PROFILING
		profEndVal = spu_read_decrementer();
		tWait += (((float)(profStartVal - profEndVal)) / PROF_SPEED);
#endif
		
		//Time to quit?
		if(bInputReady == 2)
		{
			//Wait until the DMA stuff is done
			mfc_write_tag_mask(1 << tag);
			mfc_read_tag_status_all();
			
			//Tell it to stop		
			mfc_put(&bInputReady, pNextInputReady, 4, tag, 0, 0);
			
			//Quit
			break;
		}
		
		//Swap buffers
		iCurrentBuf = 1-iCurrentBuf;
	}
	
#ifdef PROFILING
	printf("HASHING:\nWork = %f\nStall = %f\nDMA=%f\n\n",tHash,tWait,tDMA);
#endif
	
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
