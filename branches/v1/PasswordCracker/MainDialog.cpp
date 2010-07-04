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
 * MainDialog.cpp - implementation of the MainDialog class							*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/05/2008		A. Zonenberg		Created initial version						*
 * 12/06/2008		A. Zonenberg		Continued basic implementation				*
 * 12/07/2008		A. Zonenberg		Continued basic implementation				*
 * 12/08/2008		A. Zonenberg		GUI tweaks									*
 * 12/09/2008		A. Zonenberg		More GUI tweaks								*
 * 12/10/2008		A. Zonenberg		Added crack speed benchmark					*
 * 12/12/2008		A. Zonenberg		Added parallel hashing						*
 * 12/13/2008		A. Zonenberg		Added port number selection					*
 * 12/15/2008		A. Zonenberg		Changed default crack location to "farm"	*
 * 12/19/2008		A. Zonenberg		Fixed bug in port number parsing			*
 ************************************************************************************
 */

#include "stdafx.h"
#include "resource.h"
#include "MainDialog.h"
#include "..\CrackThread\CrackThread.h"
#include "..\CrackServer\CrackServer.h"
#include "NodeManager.h"

static char* charsets[]=
{
	"0123456789",
	"abcdefghijklmnopqrstuvwxyz",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	"0123456789abcdefABCDEF-",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/=",
	"0123456789abcdefghijklmnopqrstuvwxyz",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz !@#$%^&*()_+-=~`[]\\{}|;':\",./<>?",
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\x20\x21\x22\x23\x24\x25\x26\x27\x28\x29\x2a"
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
		"\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
		"\x2b\x2c\x2d\x2e\x2f\x3a\x3b\x3c\x3d\x3e\x3f\x40\x5b\x5c\x5d\x5e\x5f\x60"
		"\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c"
		"\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e"
		"\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0"
		"\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2"
		"\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4"
		"\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6"
		"\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8"
		"\xf9\xfa\xfb\xfc\xfd\xfe\xff"
};

BEGIN_MESSAGE_MAP(MainDialog,CDialog)
	ON_BN_CLICKED(IDC_START,&MainDialog::OnStart)
	ON_BN_CLICKED(IDC_STOP,&MainDialog::OnStop)
	ON_BN_CLICKED(IDC_MANAGE_NODES,&MainDialog::OnManageNodes)
END_MESSAGE_MAP()

MainDialog::MainDialog()
: CDialog(IDD_MAIN_DIALOG)
, m_hashalg("")
, m_encoding(HEX_ENCODING)
, m_guesstype(BOTH_ATTACKS)
, m_charset(1)
, m_crackon(CRACK_ON_FARM)
, m_minlen(3)
, m_maxlen(6)
, m_target(_T("a920d3b22d35e528e4b52a244cc00328"))		//md5("rpisec")
, m_crackEvent(NULL)
, m_bStopping(false)
{
	//charsets[10] has to be everything printable
	/*
	CString str;
	for(int i=32;i<=255;i++)
	{
		CString s;
		if(isalnum(i))
			str+=(char)i;
		else
		{
			s.Format(_T("\\x%02x"),i);
			str += s;
		}
	}
	int q=0;*/
}

MainDialog::~MainDialog()
{
	//Clean up algorithms
	POSITION p=m_algorithms.GetStartPosition();
	CString str;
	HashingAlgorithm* alg;
	while(p)
	{
		m_algorithms.GetNextAssoc(p,str,alg);
		delete alg;
	}
	m_algorithms.RemoveAll();

	//Free our DLLs
	for(int i=0;i<m_dlls.GetCount();i++)
		FreeLibrary(m_dlls[i]);
	m_dlls.RemoveAll();
}

void MainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_ALGORITHM_LIST,m_algList);
	DDX_Text(pDX,IDC_ALGORITHM_LIST,m_hashalg);
	DDX_CBIndex(pDX,IDC_ENCODING,m_encoding);
	DDX_CBIndex(pDX,IDC_GUESS_TYPE,m_guesstype);
	DDX_CBIndex(pDX,IDC_CHARSET,m_charset);
	DDX_CBIndex(pDX,IDC_CRACK_ON,m_crackon);
	DDX_Text(pDX,IDC_MINLEN,m_minlen);
	DDX_Text(pDX,IDC_MAXLEN,m_maxlen);
	DDX_Text(pDX,IDC_TARGET_HASH,m_target);
	DDX_Control(pDX,IDC_CRACK_PROGRESS,m_progress);
	DDX_Control(pDX,IDC_STATUS_LIST,m_statusList);
}

BOOL MainDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	ShowWindow(SW_SHOWNORMAL);
	CenterWindow();
	
	CWaitCursor wait;

	m_progress.EnableWindow(FALSE);
	m_progress.SetRange(0,100);
	m_progress.SetPos(0);

	LoadDictionary();

	LoadPlugins();
	PopulateAlgorithmList();

	m_statusList.DeleteAllItems();
	m_statusList.InsertColumn(0,_T("Host"),LVCFMT_LEFT,150,0);
	m_statusList.InsertColumn(1,_T("Cores"),LVCFMT_LEFT,50,1);
	m_statusList.InsertColumn(2,_T("Speed"),LVCFMT_LEFT,50,2);
	m_statusList.InsertColumn(3,_T("Status"),LVCFMT_LEFT,100,3);

	return TRUE;
}

void MainDialog::LoadDictionary()
{
	//Open it
	CStdioFile f;
	if(!f.Open(_T("dictionary.txt"),CFile::modeRead|CFile::typeText))
	{
		AfxMessageBox(_T("Failed to load dictionary"),MB_OK|MB_ICONSTOP);
		return;
	}

	//Load it forward and backwards
	ULONGLONG len=f.GetLength();
	CString str;
	int i=0;
	while(f.ReadString(str))
	{
		i++;
		m_dictionary.Add(str);
		CString s;
		for(int j=str.GetLength()-1;j>=0;j--)
			s+=str[j];
		if(s != str)
			m_dictionary.Add(s);

		if(i % 1000 == 0)
		{
			ULONGLONG pos=f.GetPosition();
			DWORD pct=static_cast<DWORD>(pos * 100 / len);
			CString s;
			s.Format(_T("Loading dictionary: %d%%"),pct);
			SetDlgItemText(IDC_CRACK_STATUS,s);
			m_progress.SetPos(pct);
		}
	}

	//Add every 1-2 char combination to the dictionary
	for(int i=32;i<255;i++)
	{
		CString str;
		str.Format(_T("%c"),i & 0xFF);
		m_dictionary.Add(str);

		for(int j=32;j<255;j++)
		{
			str.Format(_T("%c%c"),i & 0xFF, j & 0xFF);
			m_dictionary.Add(str);
		}
	}

	SetDlgItemText(IDC_CRACK_STATUS,_T("Ready"));
	m_progress.SetPos(0);
}

void MainDialog::LoadPlugins()
{
	CString search=_T("*.*");

	//Find all plugins
	CStringArray fs;
	CFileFind finder;
	if(!finder.FindFile(search))
	{
		AfxMessageBox(_T("No hashing algorithms found!"),MB_OK|MB_ICONSTOP);
		return;
	}
	while(finder.FindNextFile())
	{
		CString name=finder.GetFileName();
		if(name.Find(_T(".dll"),0)>0)
			fs.Add(name);
	}

	//Load them
	for(int i=0;i<fs.GetCount();i++)
	{
		HMODULE h=LoadLibrary(fs[i]);
		if(!h)
		{
			CString str;
			str.Format(_T("Cannot load hash plugin %s"),fs[i]);
			AfxMessageBox(str,MB_OK|MB_ICONSTOP);
			continue;
		}

		//Make sure it's a valid plugin DLL
		if(GetProcAddress(h,"GetAlgCount") == NULL)
		{
			FreeLibrary(h);
			continue;
		}

		m_dlls.Add(h);
	}
}

void MainDialog::PopulateAlgorithmList()
{
	//Load algorithms for each DLL
	GETALGCOUNT GetAlgCount=NULL;
	GETALGPTR GetAlgPtr=NULL;
	for(int i=0;i<m_dlls.GetCount();i++)
	{
		//Get the number of algorithms
		GetAlgCount=reinterpret_cast<GETALGCOUNT>(GetProcAddress(m_dlls[i],"GetAlgCount"));
		if(GetAlgCount==NULL)
		{
			CString err;
			err.Format(_T("Cannot find GetAlgCount export in plugin DLL %d"),i);
			AfxMessageBox(err,MB_OK|MB_ICONSTOP);
			continue;
		}
		int c=GetAlgCount();

		//Get the initialization function
		GetAlgPtr=reinterpret_cast<GETALGPTR>(GetProcAddress(m_dlls[i],"GetAlgPtr"));
		if(GetAlgPtr==NULL)
		{
			CString err;
			err.Format(_T("Cannot find GetAlgPtr export in plugin DLL %d"),i);
			AfxMessageBox(err,MB_OK|MB_ICONSTOP);
			continue;
		}

		//Create instances of each algorithm
		for(int alg=0;alg<c;alg++)
		{
			//Create it
			HashingAlgorithm* pAlg=GetAlgPtr(alg);
			if(pAlg==NULL)
			{
				CString err;
				err.Format(_T("Failed to instantiate algorithm %d in plugin DLL %d"),alg,i);
				AfxMessageBox(err,MB_OK|MB_ICONSTOP);
				continue;
			}
			
			//Add it to our list
			CString name(pAlg->GetName());
			m_algorithms[name]=pAlg;

			//Add it to the dialog
			m_algList.AddString(name);
		}
	}

	m_algList.SelectString(0,_T("MD5"));
}

void MainDialog::OnStart()
{
	SetCrackState(true);

	//Read all of our dialog data
	UpdateData();

	//Try a dictionary attack first
	if(m_guesstype == DICTIONARY_ATTACK || m_guesstype == BOTH_ATTACKS)
	{
		if(DictionaryAttack())
		{
			//Success? We're done
			SetCrackState(false);
			return;
		}

		//No go on the dictionary. Should we stop now?
		if(m_guesstype == DICTIONARY_ATTACK)
		{
			AfxMessageBox(_T("Failed to find password"),MB_OK|MB_ICONINFORMATION);
			SetCrackState(false);
			return;
		}
		else
		{
			AfxMessageBox(_T("Password is not in dictionary, trying brute force attack"),
				MB_OK|MB_ICONINFORMATION);
		}
	}

	//Dictionary attack, if requested, has failed. Go for brute force.
	if(!BruteForceAttack())
		AfxMessageBox(_T("Failed to find password"),MB_OK|MB_ICONINFORMATION);
	
	SetCrackState(false);
}

bool MainDialog::DictionaryAttack()
{
	//Get the target string
	unsigned char target[128]={0};
	DecodeTarget(target,128);

	//Get the hashing algorithm to use
	HashingAlgorithm* pAlg=NULL;
	if(!m_algorithms.Lookup(m_hashalg,pAlg))
	{
		CString str;
		str.Format(_T("Failed to find hashing algorithm %s"),m_hashalg);
		AfxMessageBox(str,MB_OK|MB_ICONSTOP);
		return false;
	}

	//Cache some data
	int len=m_dictionary.GetCount();
	unsigned char* hashbuf[4]={NULL};
	for(int i=0;i<4;i++)
		hashbuf[i]=new unsigned char[128];
	int hashlen=pAlg->GetHashLength();

	//Try the dictionary
	bool ok=false;
	for(int i=0;i<len;i++)
	{
		//We need to waste some time here by hashing the same word 4 times.
		//Unfortunately SSE hashes can't work with less than 4 inputs.
		CString word=m_dictionary[i];
		int wlen=word.GetLength();
		unsigned char* buf=reinterpret_cast<unsigned char*>(word.GetBuffer(wlen));
		unsigned char* inbuf[4]=
		{
			buf,
			buf,
			buf,
			buf
		};

		//Hash the word
		pAlg->Hash(
			inbuf,
			&hashbuf[0],
			NULL,
			wlen);

		//See if we found it
		ok=true;
		for(int j=0;j<hashlen;j++)
		{
			if(hashbuf[0][j] != target[j])
			{
				ok=false;
				break;
			}
		}
		if(ok)
		{
			CString str;
			str.Format(_T("CRACKED: %s"),word);
			SetDlgItemText(IDC_CRACK_STATUS,str);
			break;
		}

		if(i % 1000 == 0)
		{
			CString str;
			str.Format(_T("Dictionary attack: %d%%"),i*100/len);
			SetDlgItemText(IDC_CRACK_STATUS,str);
		}
	}

	for(int i=0;i<4;i++)
		delete hashbuf[i];

	return ok;
}

bool MainDialog::BruteForceAttack()
{	
	if(m_crackon == CRACK_ON_LOCALHOST)
		return LocalBruteForceAttack();
	else
		return DistributedBruteForceAttack();
}

void MainDialog::DecodeTarget(unsigned char* out,int len)
{
	switch(m_encoding)
	{
	case RAW_ENCODING:
		//Enforce null padding of shorter-than-desired strings
		strcpy_s((char*)out,len,m_target.GetBuffer(len));
		for(int i=m_target.GetLength();i<len;i++)
			out[i]='\0';
		break;
	case HEX_ENCODING:
		if(m_target.GetLength() % 2 != 0)
		{
			AfxMessageBox(_T("Hex data length must be a multiple of two characters"),MB_OK|MB_ICONINFORMATION);
			return;
		}

		for(int i=0;i<m_target.GetLength();i+=2)
		{
			char temp[]=
			{
				'0',
				'x',
				m_target[i],
				m_target[i+1],
				'\0'
			};
			int q;
			sscanf_s(temp,"%x",&q);
			out[i/2]=q;
		}
		break;
	default:
		//Not implemented
		AfxMessageBox(_T("Unsupported encoding"),MB_OK|MB_ICONINFORMATION);
	}
}

void MainDialog::SetCrackState(bool cracking)
{
	GetDlgItem(IDC_START)->EnableWindow(!cracking);
	GetDlgItem(IDC_STOP)->EnableWindow(cracking);
}

void MainDialog::OnStop()
{
	if(m_crackon == CRACK_ON_LOCALHOST)
	{
		if(m_crackEvent != NULL)
			SetEvent(m_crackEvent);
	}
	else
	{
		m_bStopping=true;
	}
}

void MainDialog::OnManageNodes()
{
	NodeManager dlg;
	dlg.m_pHosts=&m_nodes;
	dlg.DoModal();
}

bool MainDialog::LocalBruteForceAttack()
{
	//Get our target character set
	char* target_charset = charsets[m_charset];
	int charset_size = strlen(target_charset);

	//Get the target string
	unsigned char target[128]={0};
	DecodeTarget(target,128);

	//Output variables
	char crackedhash[32];
	bool found=false;

	DWORD start=GetTickCount();

	m_progress.EnableWindow(TRUE);
	m_progress.SetRange(0,100);
	m_progress.SetPos(0);

	//We don't want to waste time on excessive context switching
	//so only create as many threads as we have CPUs.
	SYSTEM_INFO info;
	GetSystemInfo(&info);

	//This event will be set to signalled when it's time to quit.
	m_crackEvent=CreateEvent(NULL,TRUE,FALSE,NULL);

	//Get the hashing algorithm to use
	HashingAlgorithm* pHashingAlg=NULL;
	if(!m_algorithms.Lookup(m_hashalg,pHashingAlg))
	{
		CString str;
		str.Format(_T("Failed to find hashing algorithm %s"),m_hashalg);
		AfxMessageBox(str,MB_OK|MB_ICONSTOP);
		return false;
	}

	//Divide starting-character ranges among threads.
	//If the size doesn't divide evenly the last thread will pick up the slack.
	int thread_chars = charset_size / info.dwNumberOfProcessors;
	for(int start=0;start<charset_size;start += thread_chars + 1)
	{
		//Set up the thread params
		CrackThreadData* dat=new CrackThreadData;
		strcpy_s(dat->charset,256,target_charset);
		dat->charset_len=charset_size;
		dat->startchar=start;
		dat->endchar=start + thread_chars;
		dat->startchar2=0;
		dat->endchar2=charset_size-1;
		if(start + thread_chars*2 > charset_size)
			dat->endchar = charset_size-1;
		dat->minlen=m_minlen;
		dat->maxlen=m_maxlen;
		dat->progress=0;
		dat->hQuitEvent=m_crackEvent;
		dat->pAlg=pHashingAlg;
		dat->found=false;
		memset(dat->crackedhash,0,32);
		memcpy(dat->target,target,128);
		m_crackdat.Add(dat);

		//Actually start the crack
		CWinThread* pThread = AfxBeginThread(CrackThreadProc,dat);
		m_crackthreads.Add(pThread->m_hThread);

		//Time to stop if we're at the end
		if(start + 2*thread_chars > charset_size)
			break;
	}

	//Wait for the cracking to finish
	bool bQuitting=false;
	while(WaitForMultipleObjects(m_crackthreads.GetCount(),m_crackthreads.GetData(),TRUE,100)==WAIT_TIMEOUT)
	{
		//Process messages
		MSG msg;
		while(PeekMessage(&msg,0,0,0,TRUE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Update progress
		if(!bQuitting)
		{
			int prog=0;
			for(int i=0;i<m_crackdat.GetCount();i++)
			{
				CrackThreadData* dat=m_crackdat[i];
				if(dat->found)
				{
					memcpy(crackedhash,dat->crackedhash,32);
					found=true;
					bQuitting=true;
					SetEvent(m_crackEvent);
					break;
				}

				prog += dat->progress;
			}
			prog /= m_crackdat.GetCount();
			m_progress.SetPos(prog);

			CString str;
			str.Format(_T("Brute force attack : %d%%"),prog);
			SetDlgItemText(IDC_CRACK_STATUS,str);
		}
	}
	SetEvent(m_crackEvent);

	//Wait for all threads to terminate
	Sleep(100);

	//Collect some stats
	DWORD dt=GetTickCount() - start;
	__int64 tried=0;
	for(int i=0;i<m_crackdat.GetCount();i++)
		tried+=m_crackdat[i]->testcount;
	float sec = static_cast<float>(dt)/1000;
	float hashspeed = static_cast<float>(tried) / sec;

	//See if we found it
	if(!found)
	{
		for(int i=0;i<m_crackdat.GetCount();i++)
		{
			CrackThreadData* dat=m_crackdat[i];
			if(dat->found)
			{
				memcpy(crackedhash,dat->crackedhash,32);
				found=true;
				break;
			}
		}
	}

	//Clean up
	m_crackthreads.RemoveAll();
	for(int i=0;i<m_crackdat.GetCount();i++)
		delete m_crackdat[i];
	m_crackdat.RemoveAll();
	CloseHandle(m_crackEvent);
	m_crackEvent=NULL;

	/*
	//Show stats
	CString str;
	str.Format(_T("Tried %llu hashes in %.2f sec (%.3f MHashes/sec)"),tried,sec,hashspeed / 1000000);
	AfxMessageBox(str,MB_OK|MB_ICONINFORMATION);*/

	//Print status
	if(found)
	{
		CString str;
		str.Format(_T("CRACKED (in %.2f sec, %.2f M/sec): %s"),sec,hashspeed/1000000,crackedhash);
		SetDlgItemText(IDC_CRACK_STATUS,str);
	}
	
	//and reset controls
	m_progress.EnableWindow(FALSE);
	m_progress.SetRange(0,100);
	m_progress.SetPos(0);

	return found;
}

bool MainDialog::DistributedBruteForceAttack()
{
	m_statusList.DeleteAllItems();

	//Get our target character set
	char* target_charset = charsets[m_charset];
	int charset_size = strlen(target_charset);

	//Get the target string
	unsigned char target[128]={0};
	DecodeTarget(target,128);
	
	char crackhost[128];
	char crackedhash[32];

	//Set progress to zero
	m_progress.EnableWindow(TRUE);
	m_progress.SetRange(0,100);
	m_progress.SetPos(0);

	DWORD starttime=GetTickCount();

	//Get the hashing algorithm to use
	HashingAlgorithm* pHashingAlg=NULL;
	if(!m_algorithms.Lookup(m_hashalg,pHashingAlg))
	{
		CString str;
		str.Format(_T("Failed to find hashing algorithm %s"),m_hashalg);
		AfxMessageBox(str,MB_OK|MB_ICONSTOP);
		return false;
	}
	int flops=pHashingAlg->GetFLOPs();
	CString algname=pHashingAlg->GetName();

	//Connect to each node
	CArray<ConnectionInfo> connections;
	for(int i=0;i<m_nodes.GetCount();i++)
	{
		//Create the socket
		SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(!sock)
		{
			AfxMessageBox(_T("Failed to create a socket!"),MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		ConnectionInfo info;
		info.sock=sock;

		//Read the address
		unsigned short port = 21219;
		CString add = m_nodes[i];
		char hn[64];
		char p[15];
		int k=0;
		for(;k<60 && k<add.GetLength();k++)
		{
			if(add[k]==':')
				break;
			hn[k]=add[k];
		}
		hn[k]='\0';
		int j=++k;
		for(;j<k+10 && j<add.GetLength();j++)
			p[j-k]=add[j];
		p[j-k]='\0';
		port=atoi(p);

		//Format the address structure
		sockaddr_in host;
		host.sin_family=AF_INET;
		host.sin_port=htons(port);
		host.sin_addr.S_un.S_addr=inet_addr(hn);

		//Try connecting to it
		if(0!=connect(sock,reinterpret_cast<sockaddr*>(&host),sizeof(host)))
		{
			CString str;
			str.Format(_T("Failed to connect to %s:%d!"),hn,port);
			AfxMessageBox(str,MB_OK|MB_ICONINFORMATION);
			closesocket(sock);
			continue;
		}
		Sleep(50);

		//Get the banner
		char banner[256];
		int len=recv(sock,banner,256,0);
		banner[len]=0;

		//Do an info query
		char query[]="STATS";
		send(sock,query,sizeof(query),0);
		char response[1024];
		len=recv(sock,response,1024,0);
		if(len > 1000)
		{
			AfxMessageBox(_T("Invalid response length"),MB_OK|MB_ICONINFORMATION);
			closesocket(sock);
			continue;
		}
		response[len]='\0';

		//Parse it
		sscanf_s(
			response,
			"Host %s has %d CPUs (crack speed=%d)",
			info.hostname,
			sizeof(info.hostname),
			&info.cpucount,
			&info.crackspeed);

		//Add to list
		int n=m_statusList.InsertItem(m_statusList.GetItemCount(),info.hostname);
		CString cpc;
		cpc.Format(_T("%d"),info.cpucount);
		m_statusList.SetItemText(n,1,cpc);
		m_statusList.SetItemText(n,3,_T("Initializing"));
		CString cps;
		cps.Format(_T("%d"),info.crackspeed);
		m_statusList.SetItemText(n,2,cps);

		//Save the socket
		connections.Add(info);
	}
	if(connections.GetCount()==0)
	{
		AfxMessageBox(_T("A distributed crack requires at least one operational compute node."),
			MB_OK|MB_ICONINFORMATION);
		return false;
	}

	m_bStopping=false;

	//Sum up the available CPUs
	int total_cpus=0;
	for(int i=0;i<connections.GetCount();i++)
		total_cpus += connections[i].cpucount;

	//Sum up available computing power
	int total_speed=0;
	for(int i=0;i<connections.GetCount();i++)
		total_speed += connections[i].cpucount * connections[i].crackspeed;

	//////////////////////////////////////////////////////////////////////////////////
	//Divide starting character ranges evenly among nodes.
	//If it doesn't work out exactly, the last node will pick up the slack.

	//This is the total number of two-letter combinations we can start with.
	int range_size = charset_size * charset_size;

	//This is the number of combinations we need to allocate for each unit of total_speed.
	float delta = static_cast<float>(range_size) / total_speed;

	//TODO: Error check here!!!

	//Start allocating characters
	int sa=0,sb=0;
	for(int i=0;i<connections.GetCount();i++)
	{
		//Start at (sa,sb)
		//TRACE("%c%c -> ",target_charset[sa],target_charset[sb]);
		
		//Allocate this many second-level combinations
		int ea=sa,eb=sb;
		int num_chars = delta * connections[i].cpucount * connections[i].crackspeed;
		eb += num_chars;
		while(eb >= charset_size)
		{
			eb-=charset_size;
			ea++;
		}

		//End at (ea,eb)
		if(i==connections.GetUpperBound())
		{
			ea=charset_size-1;
			eb=charset_size-1;
		}
		//TRACE("%c%c\n",target_charset[ea],target_charset[eb]);

		//Send the command
		char command[]="CRACK";
		send(connections[i].sock,command,sizeof(command),0);
		char response;
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//Send the charset
		send(connections[i].sock,target_charset,charset_size+1,0);
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//Send the crack range
		char crange[128];
		sprintf_s(crange,128,"%d %d - %d %d",sa,sb,ea,eb);
		send(connections[i].sock,crange,strlen(crange)+1,0);
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//Send the length range
		sprintf_s(crange,128,"%d-%d",m_minlen,m_maxlen);
		send(connections[i].sock,crange,strlen(crange)+1,0);
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//Send the hashing algorithm
		char name[128];
		strcpy_s(name,128,pHashingAlg->GetName());
		send(connections[i].sock,name,strlen(name)+1,0);
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//Send the target hash
		send(connections[i].sock,reinterpret_cast<char*>(&target[0]),128,0);
		recv(connections[i].sock,&response,1,0);
		if(response != 1)
		{
			//todo: handle error
		}

		//At this point the server will start cracking!
		//Bump the indices for next iteration.
		sa = ea;
		sb = eb + 1;
		while(sb >= charset_size)
		{
			sb-=charset_size;
			sa++;
		}
	}


	//////////////////////////////////////////////////////////////////////////////////
	//Sit around and pick up SITREPs
	bool done=false;
	bool found=false;
	while(!done)
	{
		if(m_bStopping)
		{
			//Tell the compute nodes to quit
			char q=CRACK_QUERY_ABORT;
			for(int i=0;i<connections.GetCount();i++)
				send(connections[i].sock,&q,1,0);
			done=true;
			break;
		}

		//Process messages
		MSG msg;
		while(PeekMessage(&msg,0,0,0,TRUE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Ask each server for a SITREP
		int prog=0;
		__int64 tried=0;
		bool done = true;
		for(int i=0;i<connections.GetCount();i++)
		{
			//Send the query
			char q=CRACK_QUERY_PROGRESS;
			send(connections[i].sock,&q,1,0);

			//Get the response
			bool err=false;
			char pct;
			if(1!=recv(connections[i].sock,&pct,1,0))
				err=true;
			__int64 t;
			if(8!=recv(connections[i].sock,reinterpret_cast<char*>(&t),8,0))
				err=true;
			tried+=t;

			if(err)
			{
				CString str;
				str.Format(_T("Compute node %s has stopped responding. Aborting crack."),
					connections[i].hostname);
				AfxMessageBox(str,MB_OK|MB_ICONSTOP);

				//Abort the crack
				OnStop();
				break;
			}

			//See if it found the target
			q=CRACK_QUERY_ISDONE;
			send(connections[i].sock,&q,1,0);
			char isdone;
			recv(connections[i].sock,&isdone,1,0);

			//Get the target
			if(isdone)
			{
				m_statusList.SetItemText(i,2,_T("Done"));

				//Tell the compute nodes to quit
				m_bStopping=true;

				//Get the value
				q=CRACK_QUERY_GETHASH;
				send(connections[i].sock,&q,1,0);
				ZeroMemory(crackedhash,32);
				int len=recv(connections[i].sock,crackedhash,31,0);
				crackedhash[len+1]='\0';

				strcpy_s(crackhost,32,connections[i].hostname);

				found=true;
			}
			else
			{
				CString p;
				p.Format(_T("%d%%"),pct);
				m_statusList.SetItemText(i,3,p);
				done=false;
			}

			pct=min(pct,100);
			prog += pct;
		}
		prog /= connections.GetCount();

		//Calculate elapsed time and hashing speed
		float dt = static_cast<float>(GetTickCount() - starttime) / 1000;
		float hashspeed = static_cast<float>(tried) / dt;

		//Calculate gigaflops throughput
		float gflops = (hashspeed * flops) / 1E9;

		//Update status		
		m_progress.SetPos(prog);
		CString str;
		str.Format(_T("Brute force attack (%d CPUs, %.2fM/sec, %.2f GFlops) : %d%%"),
			total_cpus,
			hashspeed / 1000000,
			gflops,
			prog);
		SetDlgItemText(IDC_CRACK_STATUS,str);

		Sleep(100);
	}

	//Print status
	if(found)
	{
		CString str;
		float t=static_cast<float>(GetTickCount() - starttime) / 1000;
		str.Format(_T("CRACKED (by %s in %.2f seconds): %s"),crackhost,t,crackedhash);
		SetDlgItemText(IDC_CRACK_STATUS,str);
	}

	//Disconnect
	for(int i=0;i<connections.GetCount();i++)
		closesocket(connections[i].sock);
	connections.RemoveAll();

	//and reset controls
	m_progress.EnableWindow(FALSE);
	m_progress.SetRange(0,100);
	m_progress.SetPos(0);

	return found;
}