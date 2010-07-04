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
* Plugin.cpp - cross-platform DLL/SO/DYLIB library                            *
*                                                                             *
******************************************************************************/

#include "Plugin.h"
#include <string>
using namespace std;

Plugin::Plugin(const char* fname)
{
#if WINDOWS

	m_hMod = LoadLibraryA(fname);
	if(!m_hMod)
		throw string("Failed to load DLL ") + fname;

#elif UNIX

	m_hMod = dlopen(fname,RTLD_LAZY);
	if(!m_hMod)
	{
		string s("./");
		s+=fname;
		m_hMod=dlopen(s.c_str(),RTLD_LAZY);
	}

	if(!m_hMod)
		throw string("Failed to load dynamic library: ") + dlerror();
#endif
}

Plugin::~Plugin()
{
#if WINDOWS
	FreeLibrary(m_hMod);
#elif UNIX
	dlclose(m_hMod);
#endif
}

PLUGINPROC Plugin::GetFunctionPtr(const char* func)
{
#if WINDOWS
	FARPROC proc = GetProcAddress(m_hMod,func);
#elif UNIX
	void *proc = dlsym(m_hMod, func);
#endif

	return reinterpret_cast<PLUGINPROC>(proc);
}
