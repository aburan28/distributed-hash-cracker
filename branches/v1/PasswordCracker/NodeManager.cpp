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
 * NodeManager.cpp - implementation of the NodeManager class						*
 *																					*
 * REVISION HISTORY																	*
 * Date				Programmer			Description									*
 * 12/07/2008		A. Zonenberg		Created initial version						*
 * 12/13/2008		A. Zonenberg		Added port number changing					*
 ************************************************************************************
 */

#include "stdafx.h"
#include "resource.h"
#include "NodeManager.h"

BEGIN_MESSAGE_MAP(NodeManager,CDialog)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_ADD, &NodeManager::OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, &NodeManager::OnRemove)
END_MESSAGE_MAP()

NodeManager::NodeManager()
: CDialog(IDD_MANAGE_NODES)
{
}

BOOL NodeManager::OnInitDialog()
{
	if(!CDialog::OnInitDialog())
		return FALSE;

	//Populate the list with data from our parent
	int c=m_pHosts->GetCount();
	for(int i=0;i<c;i++)
		m_list.AddString((*m_pHosts)[i]);

	SetDlgItemText(IDC_PORTNUM,_T("21219"));

	return TRUE;
}

void NodeManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_NODE_LIST,m_list);
}

void NodeManager::OnClose()
{
	CDialog::OnClose();

	//Empty our host list
	m_pHosts->RemoveAll();

	//Populate it with dialog data
	int c=m_list.GetCount();
	for(int i=0;i<c;i++)
	{
		CString str;
		m_list.GetText(i,str);
		m_pHosts->Add(str);
	}
}

void NodeManager::OnAdd()
{
	//Get the IP
	CString ip;
	GetDlgItemText(IDC_SERVER_IP,ip);

	//Get the port
	CString port;
	GetDlgItemText(IDC_PORTNUM,port);
	
	//Format it
	CString addr;
	addr.Format(_T("%s:%s"),ip,port);

	//Add it
	m_list.AddString(addr);

	//Clear the control
	SetDlgItemText(IDC_SERVER_IP,_T(""));
	SetDlgItemText(IDC_PORTNUM,_T("21219"));
}

void NodeManager::OnRemove()
{
	//Delete it
	UINT i=m_list.GetCurSel();
	m_list.DeleteString(i);
}