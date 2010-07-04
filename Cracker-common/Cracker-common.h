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
* Cracker-common.h - common stuff for the cracker                             *
*                                                                             *
******************************************************************************/

#ifndef CRACKER_COMMON_H
#define CRACKER_COMMON_H

#include "config.h"

#ifdef _MSC_VER
# ifdef CRACKERCOMMON_EXPORTS
#  define CRACKER_COMMON_EXPORT __declspec(dllexport)
#  define CRACKER_COMMON_EXPORT_FUNC extern "C" __declspec(dllexport)
# else
#  define CRACKER_COMMON_EXPORT __declspec(dllimport)
#  define CRACKER_COMMON_EXPORT_FUNC extern "C" __declspec(dllimport)
# endif
#else //#ifdef _MSC_VER
# define CRACKER_COMMON_EXPORT
# define CRACKER_COMMON_EXPORT_FUNC
#endif

//Sanity check
#ifdef X86
#elif defined(AMD64)
#elif defined(PS3)
#else
#error Unknown platform - please define X86, AMD64, or PS3
#endif

#endif
