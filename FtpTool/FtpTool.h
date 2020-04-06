// FtpTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CFtpToolApp:
// See FtpTool.cpp for the implementation of this class
//

enum SKIN_NAME
{
	SN_AQUAOS,
	SN_MAC,
	SN_ROYALE,
	SN_XPCORONA,
	SN_XPMETALLIC,
	SN_OTHER,
	SN_NONE = 0xffffffff
};

typedef struct _SKIN_CONFIG
{
	SKIN_NAME nName;
	CString strName;

	_SKIN_CONFIG()
	{
		nName = SN_NONE;
		strName = "";
	}
} SKIN_CONFIG;

class CFtpToolApp : public CWinApp
{
public:
	CFtpToolApp();

// Overrides
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// Implementation

	DECLARE_MESSAGE_MAP()

private:
	bool LoadSkinConfig(void);
	bool SaveSkinConfig(void);

public:
	CString GetAppPath(void);

public:
	SKIN_CONFIG m_sk;
	bool m_bRestart;
};

extern CFtpToolApp theApp;