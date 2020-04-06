// FtpToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FtpTool.h"
#include "FtpToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CFtpToolDlg dialog

CFtpToolDlg::CFtpToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFtpToolDlg::IDD, pParent)
	, m_strStatus(_T(""))
	, m_strProcess(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFtpToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTENT, m_listContent);
	DDX_Text(pDX, IDC_STATIC_STATUS, m_strStatus);
	DDX_Control(pDX, IDC_RICHEDIT2_MESSAGE, m_editMessage);
	DDX_Text(pDX, IDC_STATIC_PROCESS, m_strProcess);
}

BEGIN_MESSAGE_MAP(CFtpToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_OPTION_CONFIG, &CFtpToolDlg::OnOptionConfig)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CFtpToolDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDCANCEL, &CFtpToolDlg::OnBnClickedCancel)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CONTENT, &CFtpToolDlg::OnNMRclickListContent)
	ON_COMMAND(ID_OPERATION_UPONELEVEL, &CFtpToolDlg::OnOperationUponelevel)
	ON_COMMAND(ID_OPERATION_OPEN, &CFtpToolDlg::OnOperationOpen)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CONTENT, &CFtpToolDlg::OnNMDblclkListContent)
	ON_COMMAND(ID_OPERATION_TOROOTDIRECTORY, &CFtpToolDlg::OnOperationTorootdirectory)
	ON_COMMAND(ID_OPERATION_DOWNLOAD, &CFtpToolDlg::OnOperationDownload)
	ON_COMMAND(ID_OPERATION_UPLOAD, &CFtpToolDlg::OnOperationUpload)
	ON_COMMAND(ID_OPERATION_REFRESH, &CFtpToolDlg::OnOperationRefresh)
	ON_COMMAND(ID_OPERATION_DELETE, &CFtpToolDlg::OnOperationDelete)
	ON_MESSAGE(UM_NETWORK_MESSAGE, OnFTPMessage)
	ON_MESSAGE(UM_NETWORK_PROCESS, OnFTPProcess)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST_CONTENT, &CFtpToolDlg::OnLvnEndlabeleditListContent)
	ON_COMMAND(ID_OPERATION_NEWFOLDER, &CFtpToolDlg::OnOperationNewfolder)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_CONTENT, &CFtpToolDlg::OnLvnColumnclickListContent)
	ON_COMMAND(ID_LOG_TODAY, &CFtpToolDlg::OnLogToday)
	ON_COMMAND(ID_LOG_MORE, &CFtpToolDlg::OnLogMore)
	ON_COMMAND(ID_SKIN_AQUAOS, &CFtpToolDlg::OnSkinAquaos)
	ON_COMMAND(ID_SKIN_ROYALE, &CFtpToolDlg::OnSkinRoyale)
	ON_COMMAND(ID_SKIN_XPCORONA, &CFtpToolDlg::OnSkinXpcorona)
	ON_COMMAND(ID_SKIN_NONE, &CFtpToolDlg::OnSkinNone)
	ON_COMMAND(ID_SKIN_MAC, &CFtpToolDlg::OnSkinMac)
	ON_COMMAND(ID_SKIN_OTHER, &CFtpToolDlg::OnSkinOther)
	ON_COMMAND(ID_SKIN_GETMORE, &CFtpToolDlg::OnSkinGetmore)
	ON_COMMAND(ID_SKIN_XPMETALLIC, &CFtpToolDlg::OnSkinXpmetallic)
	ON_COMMAND(ID_OPTION_ABOUT, &CFtpToolDlg::OnOptionAbout)
END_MESSAGE_MAP()


// CFtpToolDlg message handlers

BOOL CFtpToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	this->Initialize();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFtpToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFtpToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFtpToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFtpToolDlg::Initialize(void)
{
	DWORD dwStyle;

	dwStyle = m_listContent.GetExtendedStyle();
	dwStyle |= /*LVS_EX_FULLROWSELECT |*/ LVS_EX_GRIDLINES | LVS_EDITLABELS | LVS_EX_INFOTIP; /* | LVS_SHOWSELALWAYS*/;
	this->m_listContent.SetExtendedStyle(dwStyle);
	this->m_listContent.InsertColumn(0, "Name", LVCFMT_LEFT, 300);
	this->m_listContent.InsertColumn(1, "Size", LVCFMT_CENTER, 100);
	this->m_listContent.InsertColumn(2, "Modified Time", LVCFMT_CENTER, 200);
	this->m_listImage.Create(16, 16, ILC_COLORDDB||ILC_MASK, 2, 1);
	this->m_listImage.Add(AfxGetApp()->LoadIconA(IDI_ICON_FOLDER));
	this->m_listImage.Add(AfxGetApp()->LoadIconA(IDI_ICON_FILE));
	this->m_listContent.SetImageList(&m_listImage, LVSIL_SMALL);
	this->m_listContent.EnableToolTips(TRUE);

	this->m_strStatus = "Disconnected";
	this->m_strProcess = "";
	
	pFTPClient = NULL;

	switch(theApp.m_sk.nName)
	{
		case SN_AQUAOS:
			this->GetMenu()->CheckMenuItem(ID_SKIN_AQUAOS, MF_CHECKED );
			break;
		case SN_MAC:
			this->GetMenu()->CheckMenuItem(ID_SKIN_MAC, MF_CHECKED );
			break;
		case SN_ROYALE:
			this->GetMenu()->CheckMenuItem(ID_SKIN_ROYALE, MF_CHECKED );
			break;
		case SN_XPCORONA:
			this->GetMenu()->CheckMenuItem(ID_SKIN_XPCORONA, MF_CHECKED );
			break;			
		case SN_XPMETALLIC:
			this->GetMenu()->CheckMenuItem(ID_SKIN_XPMETALLIC, MF_CHECKED );
			break;
		case SN_OTHER:
			this->GetMenu()->CheckMenuItem(ID_SKIN_OTHER, MF_CHECKED );
			break;
		default:
			this->GetMenu()->CheckMenuItem(ID_SKIN_NONE, MF_CHECKED );
			break;
	}

	this->UpdateData(FALSE);
}

void CFtpToolDlg::OnOptionConfig()
{
	// TODO: Add your command handler code here
	m_dlgOption.DoModal();
}

void CFtpToolDlg::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
	CString strTitle = "";
	this->GetDlgItemTextA(IDC_BUTTON_CONNECT, strTitle);

	if("CONNECT" == strTitle.MakeUpper())
	{
		this->m_editMessage.SetWindowTextA("");

		if(pFTPClient)
		{
			delete pFTPClient;
			pFTPClient = NULL;
		}

		pFTPClient =  new CFTPClient(this->m_dlgOption.m_strFTPAddr, this->m_dlgOption.m_strFTPUser, this->m_dlgOption.m_strFTPPwd, this->m_dlgOption.m_nFTPPort, this->m_hWnd);
		pFTPClient->m_bRecordLog = (TRUE == this->m_dlgOption.m_bRecordLog);
		pFTPClient->SetBufferSize(1024*10);		

		if(this->m_dlgOption.m_nProxyType < 3)
		{
			if(!pFTPClient->SetProxy((PROXY_TYPE)this->m_dlgOption.m_nProxyType, this->m_dlgOption.m_strProxyAddr, this->m_dlgOption.m_nProxyPort, this->m_dlgOption.m_strProxyUser, this->m_dlgOption.m_strProxyPwd))
			{
				AfxMessageBox("Failed to set proxy!");
				return;
			}
		}

		if(!pFTPClient->Login())
		{
			return;
		}

		pFTPClient->SetPassiveMode(1 == m_dlgOption.m_bPassive);

		this->UpdateList();
		
		this->SetDlgItemTextA(IDC_BUTTON_CONNECT, "Disconnect");

		this->m_strStatus = "Connected: " + this->m_dlgOption.m_strFTPAddr;
		if(this->m_dlgOption.m_nProxyType<3) this->m_strStatus += "\tProxy: " + this->m_dlgOption.m_strProxyAddr;

		UpdateData(FALSE);
	}
	else if("DISCONNECT" == strTitle.MakeUpper())
	{
		this->pFTPClient->Logout();
		delete pFTPClient;
		pFTPClient = NULL;

		this->m_listContent.DeleteAllItems();
		
		this->m_strStatus = "Disconnected";
		UpdateData(FALSE);
		
		this->SetDlgItemTextA(IDC_BUTTON_CONNECT, "Connect");
	}
}

void CFtpToolDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	if(pFTPClient) 
	{
		delete pFTPClient;
		pFTPClient = NULL;
	}
	OnCancel();
}

void CFtpToolDlg::UpdateList(void)
{
	CStringArray strNameA;
	CStringArray strSizeA;
	CStringArray strTimeA;

	if(!this->pFTPClient->FolderList("", strNameA, strSizeA, strTimeA)) //folder name is "", will list current directory
	{
		AfxMessageBox("Failed to list directory!");
		return;
	}

	this->m_listContent.SetRedraw(FALSE);
	this->m_listContent.DeleteAllItems();
	for(int n = 0; n < strNameA.GetCount(); n++)
	{
		if("DIR" == strSizeA.GetAt(n))
			this->m_listContent.InsertItem(n, strNameA.GetAt(n), 0);
		else this->m_listContent.InsertItem(n, strNameA.GetAt(n), 1);

		this->m_listContent.SetItemText(n, 0, strNameA.GetAt(n));
		this->m_listContent.SetItemText(n, 1, strSizeA.GetAt(n));
		this->m_listContent.SetItemText(n, 2, strTimeA.GetAt(n));

		/*this->m_listContent.SetHotItem(n);
		this->m_listContent.EnsureVisible(n, true);*/
	}
	this->m_listContent.SetRedraw(TRUE);
}

int CALLBACK CFtpToolDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CString & str1 = *((CString*)lParam1);
	CString & str2 = *((CString*)lParam2);
	bool & bAscend = *((bool*)lParamSort); //sort descend or ascend

	if(bAscend)
	{
		return str1.CompareNoCase(str2);
	}
	else
	{
		return str2.CompareNoCase(str1);
	}
}

void CFtpToolDlg::OnNMRclickListContent(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	
	CMenu menu;
	CMenu* popup = NULL;
	CPoint ptcurrent; 

	menu.LoadMenuA(IDR_MENU_OPERATION);
	popup = menu.GetSubMenu(0);
	GetCursorPos(&ptcurrent); 
	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptcurrent.x, ptcurrent.y, this);
}

void CFtpToolDlg::OnOperationUponelevel()
{
	// TODO: Add your command handler code here
	if(NULL != pFTPClient) 
	{
		if(pFTPClient->FolderOpen("..")) this->UpdateList();
	}
}

void CFtpToolDlg::OnOperationOpen()
{
	// TODO: Add your command handler code here
	POSITION posSel = m_listContent.GetFirstSelectedItemPosition();
	int nIndex = m_listContent.GetNextSelectedItem(posSel);

	if("DIR" == this->m_listContent.GetItemText(nIndex, 1) && NULL != pFTPClient )
	{
		CString strFolderName = this->m_listContent.GetItemText(nIndex, 0);
		if(pFTPClient->FolderOpen(strFolderName))
		{
			this->UpdateList();
		}
	}
}

void CFtpToolDlg::OnNMDblclkListContent(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	*pResult = 0;
	this->OnOperationOpen();
}

void CFtpToolDlg::OnOperationTorootdirectory()
{
	// TODO: Add your command handler code here
	if(NULL != pFTPClient) 
	{
		this->pFTPClient->DirectoryOpen("/");
		this->UpdateList();
	}
}

void CFtpToolDlg::OnOperationDownload()
{
	// TODO: Add your command handler code here
	POSITION posSel = m_listContent.GetFirstSelectedItemPosition();
	int nIndex = m_listContent.GetNextSelectedItem(posSel);

	if("DIR" != this->m_listContent.GetItemText(nIndex, 1) && NULL != pFTPClient)
	{
		CString strFileName = this->m_listContent.GetItemText(nIndex, 0);
		unsigned long long nDownload = pFTPClient->FileDownload(strFileName, "");

		if(nDownload == pFTPClient->FileSize(strFileName)) AfxMessageBox("Download ok!");
		else AfxMessageBox("Download failed!");
	}
}

void CFtpToolDlg::OnOperationUpload()
{
	// TODO: Add your command handler code here
	if(NULL != pFTPClient)
	{
		this->pFTPClient->FileUpload("", "");
		this->UpdateList();
	}
}

void CFtpToolDlg::OnOperationRefresh()
{
	// TODO: Add your command handler code here
	if(NULL != pFTPClient)
	{
		this->UpdateList();
	}
}

void CFtpToolDlg::OnOperationDelete()
{
	// TODO: Add your command handler code here
	POSITION posSel = m_listContent.GetFirstSelectedItemPosition();
	int nIndex = m_listContent.GetNextSelectedItem(posSel);
	CString strName = this->m_listContent.GetItemText(nIndex, 0);	
	bool bRet = false;

	if(NULL != pFTPClient)
	{
		if("DIR" == this->m_listContent.GetItemText(nIndex, 1))
		{
			bRet = this->pFTPClient->FolderDelete(strName);
		}
		else
		{
			bRet = this->pFTPClient->FileDelete(strName);
		}

		if(bRet) this->UpdateList();
	}
}


LRESULT CFtpToolDlg::OnFTPMessage(WPARAM wParam, LPARAM lParam)
{
	char * pszMsg = (char *)wParam;
	CString strMsg = "";

	strMsg.Format("%s\r\n", pszMsg);
	
	m_editMessage.SetSel(0xFFFF, 0xFFFF);
	m_editMessage.HideSelection(FALSE, TRUE);
	m_editMessage.ReplaceSel(strMsg);
	m_editMessage.HideSelection(TRUE, TRUE);

	return 0;
}

LRESULT CFtpToolDlg::OnFTPProcess(WPARAM wParam, LPARAM lParam)
{
	if(lParam >= 1024*1024) //MB
	{this->m_strProcess.Format("%d%%\t%.2f MB/s", wParam, float(lParam)/1024/1024);}
	else //KB
	{this->m_strProcess.Format("%d%%\t%.2f KB/s", wParam, float(lParam)/1024);}
	UpdateData(FALSE);
	return 0;
}

void CFtpToolDlg::OnLvnEndlabeleditListContent(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	POSITION posSel = m_listContent.GetFirstSelectedItemPosition();
	int nIndex = m_listContent.GetNextSelectedItem(posSel);
	CString strName = this->m_listContent.GetItemText(nIndex, 0);	

	if(NULL != this->pFTPClient && NULL != pDispInfo->item.pszText)
	{
		if(this->pFTPClient->FileRename(strName, pDispInfo->item.pszText))
		{
			*pResult = 1; //set to 1 to enable edit
			return;
		}
	}

	*pResult = 0;
}

void CFtpToolDlg::OnOperationNewfolder()
{
	// TODO: Add your command handler code here
	if(NULL != pFTPClient)
	{
		if(this->pFTPClient->FolderCreate("New Folder"))
		{
			this->UpdateList();
		}
	}
}

void CFtpToolDlg::OnLvnColumnclickListContent(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	
	static int nSubItem = 0; //clicked sub item index
	static bool bAscend = true; //sort as descend or ascend
	
	int nCount = m_listContent.GetItemCount();
	CStringArray strArray;
	
	if(nSubItem == pNMLV->iSubItem)
	{
		bAscend = (false == bAscend);
	}
	else
	{
		bAscend = true;
		nSubItem = pNMLV->iSubItem;
	}

	// Set the data of each item
	strArray.SetSize(nCount);
	for (int nIndex=0; nIndex < nCount; nIndex++)
	{
		strArray[nIndex] = this->m_listContent.GetItemText(nIndex, nSubItem);
	   this->m_listContent.SetItemData(nIndex, (DWORD_PTR)&strArray[nIndex]); //set sort value
	}

	this->m_listContent.SortItems(CompareFunc, (DWORD_PTR)&bAscend);

	*pResult = 0;
}

void CFtpToolDlg::OnLogToday()
{
	// TODO: Add your command handler code here
	CTime tmNow = CTime::GetCurrentTime();
	CString strLogFile = "";
	HINSTANCE hi = 0;

 	strLogFile.Format("%s\\Log\\FTPClient%s.log", theApp.GetAppPath(), tmNow.Format("%Y-%m-%d"));
	if(!PathFileExists(strLogFile)) AfxMessageBox("No log!");
	else
	{
		hi = ShellExecute(this->m_hWnd, "open", "notepad", strLogFile, NULL, SW_SHOWNORMAL);
		if(hi < HINSTANCE(32)) AfxMessageBox("Failed to view log!");
	}
}

void CFtpToolDlg::OnLogMore()
{
	// TODO: Add your command handler code here
	CString strAppPath = theApp.GetAppPath();
	strAppPath.Append("\\Log");

	CFileDialog fdOpen(TRUE);
	fdOpen.m_ofn.lStructSize = sizeof(OPENFILENAME);
	fdOpen.m_ofn.lpstrTitle = "Please choose the log file";
	fdOpen.m_ofn.lpstrFilter = "log files(*.log)\0*.log\0\0";
	fdOpen.m_ofn.lpstrInitialDir = strAppPath;
	if(IDOK ==	fdOpen.DoModal())
	{
		HINSTANCE hi = ShellExecute(this->m_hWnd, "open", "notepad", fdOpen.GetPathName(), NULL, SW_SHOWNORMAL);
		if(hi < HINSTANCE(32)) AfxMessageBox("Failed to view log!");
	}
}

void CFtpToolDlg::OnSkinAquaos()
{
	// TODO: Add your command handler code here
	if(SN_AQUAOS != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_AQUAOS;
}

void CFtpToolDlg::OnSkinRoyale()
{
	// TODO: Add your command handler code here
	if(SN_ROYALE != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_ROYALE;
}

void CFtpToolDlg::OnSkinXpcorona()
{
	// TODO: Add your command handler code here
	if(SN_XPCORONA != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_XPCORONA;
}

void CFtpToolDlg::OnSkinNone()
{
	// TODO: Add your command handler code here
	if(SN_NONE != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_NONE;
}
void CFtpToolDlg::OnSkinMac()
{
	// TODO: Add your command handler code here
	if(SN_MAC != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_MAC;
}

void CFtpToolDlg::OnSkinXpmetallic()
{
	// TODO: Add your command handler code here
	if(SN_XPMETALLIC != theApp.m_sk.nName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
	{
		theApp.m_bRestart = true;
		this->OnCancel();
	}
	theApp.m_sk.nName = SN_XPMETALLIC;
}

void CFtpToolDlg::OnSkinOther()
{
	// TODO: Add your command handler code here
	CFileDialog fdOpen(TRUE);
	fdOpen.m_ofn.lStructSize = sizeof(OPENFILENAME);
	fdOpen.m_ofn.lpstrTitle = "Please choose a skin file";
	fdOpen.m_ofn.lpstrFilter = "skin files(*.ssk)\0*.ssk\0\0";
	if(IDOK ==	fdOpen.DoModal())
	{
		if( fdOpen.GetPathName() != theApp.m_sk.strName && IDYES == AfxMessageBox("Restart the application now?", MB_YESNO | MB_ICONQUESTION))
		{
			theApp.m_bRestart = true;
			this->OnCancel();
		}
		theApp.m_sk.strName = fdOpen.GetPathName();
		theApp.m_sk.nName = SN_OTHER;
	}
}

void CFtpToolDlg::OnSkinGetmore()
{
	// TODO: Add your command handler code here
	//http://www.uipower.com/SkinList.php
	//HINSTANCE hi = ShellExecute(this->m_hWnd, "open", "iexplore.exe", "http://www.uipower.com/SkinList.php", NULL, SW_SHOWNORMAL);
	HINSTANCE hi = ShellExecute(this->m_hWnd, "open", "http://www.uipower.com/SkinList.php", NULL, NULL, SW_SHOWNORMAL); //call default exlpore
	if(hi < HINSTANCE(32)) AfxMessageBox("Failed to get skin!");
}

void CFtpToolDlg::OnOptionAbout()
{
	// TODO: Add your command handler code here
	CFTPClient::About();
}
