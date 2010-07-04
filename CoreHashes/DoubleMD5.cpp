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
 * DoubleMD5.cpp - implementation of the DoubleMD5 class							*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/24/2008		A. Zonenberg		Created initial version						*
 ************************************************************************************
 */

#include "stdafx.h"
#include "hashexports.h"
#include "DoubleMD5.h"

char* DoubleMD5::GetName()
{
	static char name[]="Double MD5";
	return name;
}

bool DoubleMD5::RequiresSalt()
{
	return false;
}

int DoubleMD5::GetHashLength()
{
	return 16;
}

void DoubleMD5::Hash(
		unsigned char** inbuf,
		unsigned char** outbuf,
		unsigned char** salt,
		int len)
{
	//Not yet implemented...
}