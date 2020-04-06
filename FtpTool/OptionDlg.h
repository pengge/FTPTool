#pragma once
#include "afxwin.h"


// COptionDlg dialog

class COptionDlg : public CDialog
{
	DECLARE_DYNAMIC(COptionDlg)

public:
	COptionDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COptionDlg();

// Dialog Data
	enum { IDD = IDD_OPTION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnCbnSelchangeComboProxytype();
	virtual BOOL OnInitDialog();

	bool LoadConfig(void);
	bool SaveConfig(void);

public:
	CString m_strFTPAddr;
	UINT m_nFTPPort;
	CString m_strFTPUser;
	CString m_strFTPPwd;

	UINT m_nProxyPort;
	CString m_strProxyAddr;
	CString m_strProxyUser;
	CString m_strProxyPwd;
	int m_nProxyType;

	BOOL m_bPassive;
	BOOL m_bRecordLog;
};
