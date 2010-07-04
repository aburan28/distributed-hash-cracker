/************************************************************************************
 * Password Cracker																	*
 *																					*
 * Copyright (c) 2008 RPISec														*
 * All rights reserved.																*
 *																					*
 * Redistribution and use in source and binary forms, with or without				*
 * modification, are permitted provided that the following conditions are met:		*
 *     *Redistributions of source code must retain the above copyright				*
 *      notice, this list of conditions and the following disclaimer.				*
 *     *Redistributions in binary form must reproduce the above copyright			*
 *      notice, this list of conditions and the following disclaimer in the			*
 *      documentation and/or other materials provided with the distribution.		*
 *     *Neither the name of RPISec nor the											*
 *      names of its contributors may be used to endorse or promote products		*
 *      derived from this software without specific prior written permission.		*
 *																					*
 * THIS SOFTWARE IS PROVIDED BY RPISec "AS IS" AND ANY								*
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
 * CrackThread.h - declaration of cracking thread									*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/07/2008		A. Zonenberg		Created initial version						*
 * 12/10/2008		A. Zonenberg		Added multi-char specifications				*
 * 12/12/2008		A. Zonenberg		Added test count							*
 ************************************************************************************
 */

#pragma once

#include "..\CoreHashes\hashexports.h"

struct CrackThreadData
{
	//INPUT DATA
	char charset[256];				//Character set to try
	unsigned char charset_len;		//The desired character set
	
	unsigned char target[128];		//The target hash (must be under 1024 bits).
									//Note: This is raw hash output, not hex or base64.

	int startchar;					//Start and end values (inclusive) for the first character
	int endchar;
	int startchar2;					//Start and end values (inclusive) for the second character
	int endchar2;

	int minlen;						//Min and max length passwords to try
	int maxlen;
	HashingAlgorithm* pAlg;			//The hashing algorithm to use

	HANDLE hQuitEvent;				//Set to signalled if we should quit

	//OUTPUT DATA
	unsigned int progress;			//Percentage complete
	bool found;						//TRUE if we've found a hash
	char crackedhash[32];			//The input producing our hash
	__int64 testcount;
};

#ifdef CRACK_CPP
extern "C" UINT __declspec(dllexport) CrackThreadProc(LPVOID pData);
#else
extern "C" UINT __declspec(dllimport) CrackThreadProc(LPVOID pData);
#endif