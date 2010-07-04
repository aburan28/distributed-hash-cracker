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
* MD5crypt.cpp - CPU-based MD5 algorithm                                      *
*                                                                             *
******************************************************************************/

#include "CoreHashes-generic.h"
#include "../Cracker-common/HashingAlgorithm.h"
#include "MD5.h"
#include "MD5crypt.h"
#include <memory.h>
#include <string>
using namespace std;

const char* md5crypt_name = "md5crypt";

///////////////////////////////////////////////////////////////////////////////
//MD5Crypt_Portable: Generic C++ implementation
MD5crypt_Portable::MD5crypt_Portable()
{
	Hash = &MD5crypt_Portable::DoHash;
}

MD5crypt_Portable::~MD5crypt_Portable()
{
}

//Informational callbacks
int MD5crypt_Portable::GetHashLength()
{
	return MD5CRYPT_HASHSIZE;
}

int MD5crypt_Portable::GetMaxInputSize()
{
	return MD5CRYPT_MAXINPUT;
}

const char* MD5crypt_Portable::GetName()
{
	return md5crypt_name;
}

int MD5crypt_Portable::GetVectorSize()
{
	//No parallelism at all. What do you expect from generic, portable, unoptimized code?
	return 1;
}

bool MD5crypt_Portable::IsGPUBased()
{
	return false;
}

#define encode(x) charset[x];

void MD5crypt_Portable::DoHash(unsigned char *in, unsigned char *out, int len,unsigned char* salt, int saltlen)
{
	//REFERENCE: http://www.users.zetnet.co.uk/hopwood/crypto/scan/algs/md5crypt.txt
	unsigned char charset[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	
	int inlen = len;
	int slen = saltlen;
	
	//|| is defined as concatenation
	//foo := MD5(password || salt || password);
	unsigned char foo[512];
	int foolen=0;
	for(int i=0; i<inlen; i++)
		foo[foolen+i] = in[i];
	foolen += inlen;
	for(int i=0; i<slen; i++)
		foo[foolen+i] = salt[i];
	foolen += slen;
	for(int i=0; i<inlen; i++)
		foo[foolen+i] = in[i];
	foolen += inlen;
	MD5_Portable::DoHash(foo,foo,foolen,NULL,0);
	
	// extend(str, len) is str repeated to fill len bytes.
	//  bar := password || "$1$" || salt || extend(foo, length(password));
	unsigned char bar[512];
	int barlen = 0;
	for(int i=0; i<inlen; i++)
		bar[barlen+i] = in[i];
	barlen += inlen;
	bar[barlen++]='$';
	bar[barlen++]='1';
	bar[barlen++]='$';
	for(int i=0; i<slen; i++)
		bar[barlen+i] = salt[i];
	barlen += slen;
	for(int i=0; i<inlen; i++)		//extend(foo, inlen)
		bar[barlen+i] = foo[i%foolen];
	barlen += inlen;
	
	/*
	i := length(password);
	while (i > 0) {
		if (i mod 2 != 0) {
			bar := bar || 0;
		} else {
		bar := bar || password[0];
		}
		i := i >>> 1;
	}
	*/
	int i = inlen;	
	while(i > 0)
	{
		if(i % 2)
			bar[barlen++] = '\0';	//append a null, not an ascii '0' or nothing at all!
		else
			bar[barlen++] = in[0];
		i >>= 1;
	}
	
	//baz := MD5(bar);  // [*]
	unsigned char baz[512];
	MD5_Portable::DoHash(bar,baz,barlen,NULL,0);
	
	/*
  	 ifnz(i, s) = "", if i == 0
            = s,  otherwise
	*/
	
	unsigned char tbaz[512];
	i=0;
	while(i<1000)
	{
		//baz := MD5(baz || ifnz(i mod 3, salt) || ifnz(i mod 7, password) || password);
		int tbazlen=0;
		for(int j=0; j<16; j++)
			tbaz[tbazlen+j] = baz[j];
		tbazlen += 16;
		if(i%3 != 0)
		{
			for(int j=0; j<slen; j++)
				tbaz[tbazlen+j] = salt[j];
			tbazlen += slen;
		}
		if(i%7 != 0)
		{
			for(int j=0; j<inlen; j++)
				tbaz[tbazlen+j] = in[j];
			tbazlen += inlen;
		}
		for(int j=0; j<inlen; j++)
			tbaz[tbazlen+j] = in[j];
		tbazlen += inlen;
		MD5_Portable::DoHash(tbaz,baz,tbazlen,NULL,0);
		
		i++;
		
		//baz := MD5(password || ifnz(i mod 3, salt) || ifnz(i mod 7, password) || baz);
		tbazlen=0;
		for(int j=0; j<inlen; j++)
			tbaz[tbazlen+j] = in[j];
		tbazlen += inlen;
		if(i%3 != 0)
		{
			for(int j=0; j<slen; j++)
				tbaz[tbazlen+j] = salt[j];
			tbazlen += slen;
		}
		if(i%7 != 0)
		{
			for(int j=0; j<inlen; j++)
				tbaz[tbazlen+j] = in[j];
			tbazlen += inlen;
		}
		for(int j=0; j<16; j++)
			tbaz[tbazlen+j] = baz[j];
		tbazlen += 16;
		MD5_Portable::DoHash(tbaz,baz,tbazlen,NULL,0);
		
		i++;
	}
	
	// encode(x) = ("./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" ||
	//              "abcdefghijklmnopqrstuvwxyz")[x]

	// output := "$1$" || salt || "$" || 
	//This gets trimmed off before we're called

	int outlen = 0;
	out[outlen++] = encode(                    baz[12]       & 0x3F );
	out[outlen++] = encode((baz[12] >> 6) | ((baz[ 6] << 2) & 0x3F));
	out[outlen++] = encode((baz[ 6] >> 4) | ((baz[ 0] << 4) & 0x3F));
	out[outlen++] = encode( baz[ 0] >> 2                           );
	out[outlen++] = encode(                    baz[13]       & 0x3F );
	out[outlen++] = encode((baz[13] >> 6) | ((baz[ 7] << 2) & 0x3F));
	out[outlen++] = encode((baz[ 7] >> 4) | ((baz[ 1] << 4) & 0x3F));
	out[outlen++] = encode( baz[ 1] >> 2                           );
	out[outlen++] = encode(                    baz[14]       & 0x3F );
	out[outlen++] = encode((baz[14] >> 6) | ((baz[ 8] << 2) & 0x3F));
	out[outlen++] = encode((baz[ 8] >> 4) | ((baz[ 2] << 4) & 0x3F));
	out[outlen++] = encode( baz[ 2] >> 2                           );
	out[outlen++] = encode(                    baz[15]       & 0x3F );
	out[outlen++] = encode((baz[15] >> 6) | ((baz[ 9] << 2) & 0x3F));
	out[outlen++] = encode((baz[ 9] >> 4) | ((baz[ 3] << 4) & 0x3F));
	out[outlen++] = encode( baz[ 3] >> 2                           );
	out[outlen++] = encode(                    baz[ 5]       & 0x3F );
	out[outlen++] = encode((baz[ 5] >> 6) | ((baz[10] << 2) & 0x3F));
	out[outlen++] = encode((baz[10] >> 4) | ((baz[ 4] << 4) & 0x3F));
	out[outlen++] = encode( baz[ 4] >> 2                           );
	out[outlen++] = encode(                    baz[11]       & 0x3F );
	out[outlen++] = encode( baz[11] >> 6                           );
	out[outlen]='\0';
}
