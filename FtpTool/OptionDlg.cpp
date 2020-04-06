// OptionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FtpTool.h"
#include "OptionDlg.h"


// COptionDlg dialog

IMPLEMENT_DYNAMIC(COptionDlg, CDialog)

COptionDlg::COptionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COptionDlg::IDD, pParent)
	, m_strFTPAddr(_T(""))
	, m_nFTPPort(21)
	, m_strFTPUser(_T(""))
	, m_strFTPPwd(_T(""))
	, m_nProxyPort(0)
	, m_strProxyAddr(_T(""))
	, m_strProxyUser(_T(""))
	, m_strProxyPwd(_T(""))
	, m_nProxyType(3)
	, m_bPassive(FALSE)
	, m_bRecordLog(TRUE)
{
	this->LoadConfig();
}

COptionDlg::~COptionDlg()
{
}

void COptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FTPADDR, m_strFTPAddr);
	DDX_Text(pDX, IDC_EDIT_FTPPORT, m_nFTPPort);
	DDX_Text(pDX, IDC_EDIT_FTPUSER, m_strFTPUser);
	DDX_Text(pDX, IDC_EDIT_FTPPASS, m_strFTPPwd);
	DDX_Text(pDX, IDC_EDIT_PROXYPORT, m_nProxyPort);
	DDX_Text(pDX, IDC_EDIT_PROXYADDR, m_strProxyAddr);
	DDX_Text(pDX, IDC_EDIT_PROXYUSER, m_strProxyUser);
	DDX_Text(pDX, IDC_EDIT_PROXYPASS, m_strProxyPwd);
	DDX_CBIndex(pDX, IDC_COMBO_PROXYTYPE, m_nProxyType);
	DDX_Check(pDX, IDC_CHECK_PASSIVE, m_bPassive);
	DDX_Check(pDX, IDC_CHECK_RECORD_LOG, m_bRecordLog);
}


BEGIN_MESSAGE_MAP(COptionDlg, CDialog)
	ON_BN_CLICKED(IDOK, &COptionDlg::OnBnClickedOk)
	ON_CBN_SELCHANGE(IDC_COMBO_PROXYTYPE, &COptionDlg::OnCbnSelchangeComboProxytype)
END_MESSAGE_MAP()


// COptionDlg message handlers

void COptionDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(!this->SaveConfig())
	{
		AfxMessageBox("Failed to save config!");
		return;
	}
	OnOK();
}

void COptionDlg::OnCbnSelchangeComboProxytype()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if(3 == this->m_nProxyType)
	{
		this->GetDlgItem(IDC_EDIT_PROXYADDR)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYPORT)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYUSER)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYPASS)->EnableWindow(FALSE);

		this->m_strProxyAddr = "";
		this->m_nProxyPort = 0;
		this->m_strProxyUser = "";
		this->m_strProxyPwd = "";
	}
	else
	{
		if(2 == this->m_nProxyType)
		{
			this->m_nProxyPort = 1080;
		}
		else if(1 == this->m_nProxyType)
		{
			this->m_nProxyPort = 8080;
		}
		else
		{
			this->m_nProxyPort = 3128;
		}

		this->GetDlgItem(IDC_EDIT_PROXYADDR)->EnableWindow(TRUE);
		this->GetDlgItem(IDC_EDIT_PROXYPORT)->EnableWindow(TRUE);
		this->GetDlgItem(IDC_EDIT_PROXYUSER)->EnableWindow(TRUE);
		this->GetDlgItem(IDC_EDIT_PROXYPASS)->EnableWindow(TRUE);
	}
	UpdateData(FALSE);
}

BOOL COptionDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	if(!this->LoadConfig()) 
	{
		if(!this->SaveConfig())
		{
			AfxMessageBox("Please update config!");
			return FALSE;
		}
	}

	if(3 == this->m_nProxyType)
	{
		this->GetDlgItem(IDC_EDIT_PROXYADDR)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYPORT)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYUSER)->EnableWindow(FALSE);
		this->GetDlgItem(IDC_EDIT_PROXYPASS)->EnableWindow(FALSE);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

bool COptionDlg::LoadConfig(void)
{
	CRegKey regConf;
	ULONG nSize = MAX_PATH;
	char szBuf[MAX_PATH] = "";
	
	if(ERROR_SUCCESS != regConf.Create(HKEY_CURRENT_USER, "Software\\FTPTool\\Config")) return false;
	
	if(ERROR_SUCCESS != regConf.QueryStringValue("FTP Server", szBuf, &nSize)) return false;
	this->m_strFTPAddr = szBuf;

	nSize = MAX_PATH;
	memset(szBuf, 0, MAX_PATH);
	if(ERROR_SUCCESS != regConf.QueryStringValue("FTP User", szBuf, &nSize)) return false;
	this->m_strFTPUser = szBuf;

	nSize = MAX_PATH;
	memset(szBuf, 0, MAX_PATH);
	if(ERROR_SUCCESS != regConf.QueryStringValue("FTP Password", szBuf, &nSize)) return false;
	this->m_strFTPPwd = szBuf;

	if(ERROR_SUCCESS != regConf.QueryDWORDValue("FTP Port", (DWORD&)this->m_nFTPPort)) return false;

	nSize = MAX_PATH;
	memset(szBuf, 0, MAX_PATH);
	if(ERROR_SUCCESS != regConf.QueryStringValue("Proxy Server", szBuf, &nSize)) return false;
	this->m_strProxyAddr = szBuf;

	nSize = MAX_PATH;
	memset(szBuf, 0, MAX_PATH);
	if(ERROR_SUCCESS != regConf.QueryStringValue("Proxy User", szBuf, &nSize)) return false;
	this->m_strProxyUser = szBuf;

	nSize = MAX_PATH;
	memset(szBuf, 0, MAX_PATH);
	if(ERROR_SUCCESS != regConf.QueryStringValue("Proxy Password", szBuf, &nSize)) return false;
	this->m_strProxyPwd = szBuf;

	if(ERROR_SUCCESS != regConf.QueryDWORDValue("Proxy Port", (DWORD&)this->m_nProxyPort)) return false;

	if(ERROR_SUCCESS != regConf.QueryDWORDValue("Proxy Type", (DWORD&)this->m_nProxyType)) return false;

	if(ERROR_SUCCESS != regConf.QueryDWORDValue("Passive Mode", (DWORD&)this->m_bPassive)) return false;

	if(ERROR_SUCCESS != regConf.QueryDWORDValue("Record Log", (DWORD&)this->m_bRecordLog)) return false;
	
	return true;
}

bool COptionDlg::SaveConfig(void)
{
	CRegKey regConf;
	if(ERROR_SUCCESS != regConf.Create(HKEY_CURRENT_USER, "Software\\FTPTool\\Config")) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("FTP Server", this->m_strFTPAddr)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("FTP User", this->m_strFTPUser)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("FTP Password", this->m_strFTPPwd)) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("FTP Port", this->m_nFTPPort)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("Proxy Server", this->m_strProxyAddr)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("Proxy User", this->m_strProxyUser)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("Proxy Password", this->m_strProxyPwd)) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("Proxy Port", this->m_nProxyPort)) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("Proxy Type", this->m_nProxyType)) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("Passive Mode", this->m_bPassive)) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("Record Log", this->m_bRecordLog)) return false;
	return true;
}