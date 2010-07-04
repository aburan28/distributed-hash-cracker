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
 * CoreHashes.cpp - implementation of export table									*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/06/2008		A. Zonenberg		Created initial version						*
 * 12/12/2008		A. Zonenberg		Removed XOR128								*
 * 12/15/2008		A. Zonenberg		Added MD4									*
 * 12/24/2008		A. Zonenberg		Added double MD5							*
 ************************************************************************************
 */

#include "stdafx.h"
#include "hashexports.h"
#include "MD4.h"
#include "MD5.h"
#include "DoubleMD5.h"

#define ALG_COUNT 2

//Exports
extern "C" __declspec(dllexport) int GetAlgCount();
extern "C" __declspec(dllexport) HashingAlgorithm* GetAlgPtr(int i);

int GetAlgCount()
{
	return ALG_COUNT;
}

HashingAlgorithm* GetAlgPtr(int i)
{
	switch(i)
	{
	case 0:
		return new MD5;
	case 1:
		return new MD4;
	/*case 2:
		return new DoubleMD5;*/
	default:
		return NULL;
	}
}