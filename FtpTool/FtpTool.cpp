// FtpTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FtpTool.h"
#include "FtpToolDlg.h"

#include "SkinPPWTL/SkinPPWTL.h" //skin
#pragma comment(lib, "SkinPPWTL/SkinPPWTL.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFtpToolApp

BEGIN_MESSAGE_MAP(CFtpToolApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFtpToolApp construction

CFtpToolApp::CFtpToolApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	AfxInitRichEdit2();
}


// The one and only CFtpToolApp object

CFtpToolApp theApp;


// CFtpToolApp initialization

BOOL CFtpToolApp::InitInstance()
{
	/*if(!AfxInitRichEdit2()) return FALSE;*/
	if(!this->LoadSkinConfig())
	{
		if(!this->SaveSkinConfig())
		{
			AfxMessageBox("Please update skin config!");
			return FALSE;
		}
	}

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CFtpToolDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CFtpToolApp::ExitInstance()
{
	// TODO: Add your specialized code here and/or call the base class
	if(!this->SaveSkinConfig())
	{
		AfxMessageBox("Failed to save config");
		return 0;
	}
		
	if(this->m_bRestart)
	{
		HINSTANCE hi = ShellExecute(NULL, "open", theApp.m_pszAppName, NULL, theApp.GetAppPath(), SW_SHOWNORMAL);
		if(hi < HINSTANCE(32)) AfxMessageBox("Failed to restart application!");

		this->m_bRestart = false;
	}

	return CWinApp::ExitInstance();
}

bool CFtpToolApp::LoadSkinConfig(void)
{
	CRegKey regConf;
	char * pszSkin = NULL;
	CString strSkinPath = "";
	ULONG nSize = MAX_PATH;
	
	if(ERROR_SUCCESS != regConf.Create(HKEY_CURRENT_USER, "Software\\FTPTool\\Skin")) return false;
	if(ERROR_SUCCESS != regConf.QueryDWORDValue("Skin", (DWORD&)this->m_sk.nName)) return false;
	
	strSkinPath = theApp.GetAppPath();
	switch(this->m_sk.nName)
	{
		case SN_AQUAOS:
			strSkinPath.Append("\\Skins\\AquaOS.ssk");
			break;
		case SN_MAC:
			strSkinPath.Append("\\Skins\\MAC.ssk");
			break;
		case SN_ROYALE:
			strSkinPath.Append("\\Skins\\Royale.ssk");
			break;
		case SN_XPCORONA:
			strSkinPath.Append("\\Skins\\xp_corona.ssk");
			break;
		case SN_XPMETALLIC:
			strSkinPath.Append("\\Skins\\XP-Metallic.ssk");
			break;
		case SN_OTHER:
			if(ERROR_SUCCESS != regConf.QueryStringValue("Other",this->m_sk.strName.GetBuffer(nSize), &nSize)) 
			{
				this->m_sk.strName.ReleaseBuffer(nSize);
				return false;
			}
			this->m_sk.strName.ReleaseBuffer(nSize);
			strSkinPath = this->m_sk.strName;
			break;
		default:
			break;
	}

	pszSkin = strSkinPath.GetBuffer();
	strSkinPath.ReleaseBuffer();
	regConf.Close();

	if(!skinppLoadSkin(pszSkin)) return false;

	return true;
}

bool CFtpToolApp::SaveSkinConfig(void)
{
	CRegKey regConf;
	if(ERROR_SUCCESS != regConf.Create(HKEY_CURRENT_USER, "Software\\FTPTool\\Skin")) return false;
	if(ERROR_SUCCESS != regConf.SetDWORDValue("Skin", this->m_sk.nName)) return false;
	if(ERROR_SUCCESS != regConf.SetStringValue("Other", this->m_sk.strName)) return false;
	regConf.Close();
	if(!skinppExitSkin()) return false;
	return true;
}

CString CFtpToolApp::GetAppPath(void)
{
	CString strAppPath = "";
	::GetModuleFileName(NULL, strAppPath.GetBuffer(MAX_PATH), MAX_PATH);
	strAppPath.ReleaseBuffer(MAX_PATH);
	strAppPath = strAppPath.Left(strAppPath.ReverseFind('\\'));

	return strAppPath;
}