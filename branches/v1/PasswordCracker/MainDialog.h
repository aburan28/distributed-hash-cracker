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
 * MainDialog.h - declaration of the MainDialog class								*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/05/2008		A. Zonenberg		Created initial version						*
 * 12/06/2008		A. Zonenberg		Continued basic implementation				*
 * 12/07/2008		A. Zonenberg		Continued basic implementation				*
 * 12/09/2008		A. Zonenberg		Added list control							*
 * 12/10/2008		A. Zonenberg		Added crack-speed benchmark					*
 ************************************************************************************
 */

#include "..\CoreHashes\hashexports.h"
#include "..\CrackThread\CrackThread.h"

struct ConnectionInfo
{
	SOCKET sock;
	char hostname[128];
	int cpucount;
	int crackspeed;
};

class MainDialog : public CDialog
{
public:
	MainDialog();
	~MainDialog();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	//Dialog data
	CComboBox m_algList;
	CString m_hashalg;
	int m_encoding;
	int m_guesstype;
	int m_charset;
	int m_crackon;
	int m_minlen;
	int m_maxlen;
	CString m_target;
	CProgressCtrl m_progress;
	CListCtrl m_statusList;

	enum crackons
	{
		CRACK_ON_LOCALHOST,
		CRACK_ON_FARM,
	};

	enum attacks
	{
		DICTIONARY_ATTACK,
		BRUTE_FORCE_ATTACK,
		BOTH_ATTACKS
	};

	enum encodings
	{
		RAW_ENCODING,
		HEX_ENCODING,
		BASE64_ENCODING,
		CRYPT_ENCODING
	};

	//Plugins
	CArray<HMODULE,HMODULE> m_dlls;
	CMap<CString,LPCTSTR,HashingAlgorithm*,HashingAlgorithm*> m_algorithms;

	//Notification handlers
	void OnStart();
	void OnStop();
	void SetCrackState(bool cracking);
	void OnManageNodes();

	//Cracking
	bool DictionaryAttack();
	bool BruteForceAttack();
	bool LocalBruteForceAttack();
	bool DistributedBruteForceAttack();
	void DecodeTarget(unsigned char* out,int len);

	//Dictionary cracking data
	void LoadDictionary();
	CStringArray m_dictionary;

	//Local cracking data
	HANDLE m_crackEvent;
	CArray<HANDLE,HANDLE> m_crackthreads;
	CArray<CrackThreadData*> m_crackdat;

	//Distributed cracking data
	bool m_bStopping;
	CStringArray m_nodes;

	//Initialization
	void LoadPlugins();
	void PopulateAlgorithmList();

protected:
	DECLARE_MESSAGE_MAP();
};