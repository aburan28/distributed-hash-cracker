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
 * PasswordCracker.cpp - implementation of application class						*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/05/2008		A. Zonenberg		Created initial version						*
 * 12/07/2008		A. Zonenberg		Added Winsock init / cleanup				*
 ************************************************************************************
 */

#include "stdafx.h"
#include "PasswordCracker.h"
#include "MainDialog.h"

PasswordCrackerApp theApp;

BOOL PasswordCrackerApp::InitInstance()
{
	//Initialize Winsock
	WSADATA wsdat;
	if(0!=WSAStartup(MAKEWORD(2,2),&wsdat))
	{
		MessageBox(NULL,_T("Winsock init failed"),_T("Error"),MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	if(!CWinApp::InitInstance())
		return FALSE;

	//Create and show the main window
	MainDialog dlg;
	dlg.DoModal();

	//Clean up Winsock
	WSACleanup();

	//don't enter main message loop
	return FALSE;
}