// FtpToolDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "OptionDlg.h"
#include "afxwin.h"

// CFtpToolDlg dialog
class CFtpToolDlg : public CDialog
{
// Construction
public:
	CFtpToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FTPTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnOptionConfig();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnNMRclickListContent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOperationUponelevel();
	afx_msg void OnOperationOpen();
	afx_msg void OnNMDblclkListContent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOperationTorootdirectory();
	afx_msg void OnOperationDownload();
	afx_msg void OnOperationUpload();
	afx_msg void OnOperationRefresh();
	afx_msg void OnOperationDelete();
	afx_msg LRESULT OnFTPMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFTPProcess(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnEndlabeleditListContent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOperationNewfolder();
	afx_msg void OnLvnColumnclickListContent(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLogToday();
	afx_msg void OnLogMore();
	afx_msg void OnSkinAquaos();
	afx_msg void OnSkinRoyale();
	afx_msg void OnSkinXpcorona();
	afx_msg void OnSkinNone();
	afx_msg void OnSkinMac();
	afx_msg void OnSkinOther();
	afx_msg void OnSkinGetmore();
	afx_msg void OnSkinXpmetallic();
	afx_msg void OnOptionAbout();

public:
	void Initialize(void);
	void UpdateList(void);
	static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
 
public:
	CImageList m_listImage;
	CListCtrl m_listContent;
	CRichEditCtrl m_editMessage;	

	COptionDlg m_dlgOption;
	CString m_strStatus;
	CString m_strProcess;

	CFTPClient * pFTPClient;
};
