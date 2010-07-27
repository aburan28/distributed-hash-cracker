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
 * MD4.h - declaration of the MD4 class												*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/15/2008		A. Zonenberg		Created initial version						*
 * 01/19/2009		A. Zonenberg		Added FLOP counter							*
 ************************************************************************************
 */
#pragma once

class MD4 : public HashingAlgorithm
{
public:
	virtual char* GetName();		//Returns a statically allocated buffer
	virtual bool RequiresSalt();	//Returns true if we need a salt
	virtual int GetHashLength();	//Returns hash length in bytes
	
	virtual void Hash(				//Calculates four hashes simultaneously, in a thread-safe manner.
		unsigned char** in,			//Salt must be null if we don't use a salt,
		unsigned char** out,		//and must point to a two-byte array if we do.
		unsigned char** salt,
		int len);

	virtual int GetFLOPs();			//return FLOPs per hashing operation
};