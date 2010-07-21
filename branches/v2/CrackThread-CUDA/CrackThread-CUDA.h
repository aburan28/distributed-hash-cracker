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
* CrackThread-CUDA.h - general useful stuff                                   *
*                                                                             *
******************************************************************************/

#ifndef CRACKTHREADCUDA_H
#define CRACKTHREADCUDA_H

#include "config.h"

#if WINDOWS
#include <windows.h>
#endif

#if UNIX
#include <sys/time.h>
#endif

#include <iostream>
#include <string>
#include <stdlib.h>
#include <memory.h>
#include <fstream>
#include "../Cracker-common/Thread.h"

//Special export modifiers for various platforms
#ifdef _MSC_VER
#define ZEXPORT extern "C" __declspec(dllexport)
#else
#define ZEXPORT extern "C"
#endif

//Turn off stupid VC++ warnings for sscanf on %x: can't possibly overflow
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

ZEXPORT bool Initialize();
ZEXPORT void Cleanup();
ZEXPORT int GetMaxThreads();
ZEXPORT int GetCrackType();
ZEXPORT ZTHREADPROC GetComputeThread();
THREAD_PROTOTYPE(internalComputeThreadProc,_pData);

#endif