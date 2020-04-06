/********************************
 Scope:		CFTPClient
 Author:	Walnut
 Updated:	2012-08-05
********************************/
#pragma once
#include <afxsock.h>
#include "network.h"

enum TRANSFER_TYPE
{
	TT_ASCII,
	TT_BINARY
	/*	TT_EBCDIC,
	TT_LOCAL*/
};

enum PROXY_TYPE
{
	PROXY_HTTP11_GET,
	PROXY_HTTP11_CONNECT,
	PROXY_SOCKS5,
	PROXY_NONE = -1
};

typedef struct FTP_PROFILE
{
	bool bSupportMLSD;
	bool bSupportCompress;
	bool bUnix;
	bool bWin;

	FTP_PROFILE()
	{
		bSupportMLSD = false;
		bSupportCompress = false;
		bUnix = false;
		bWin = false;
	}
} * ftp_profile;

class CFTPClient : private CNetwork
{
public:
	CFTPClient(LPCTSTR lpszFTPServer, LPCTSTR lpszFTPUser = NULL, LPCTSTR lpszFTPPassword = NULL, UINT nFTPPort = 21, HWND hWnd = NULL);
	~CFTPClient(void);

	bool Login(void);
	void Logout(void);

	bool SetProxy(PROXY_TYPE nProxyType, LPCTSTR lpszProxyServer, UINT nProxyPort, LPCTSTR lpszProxyUser = NULL, LPCTSTR lpszProxyPassword = NULL);
	void SetPassiveMode(bool bPassive);
	void SetBufferSize(int nBytes);

	//file operation function (both absolute and relative path are work)
	unsigned long long FileDownload(LPCTSTR lpszFTPFile, LPCTSTR lpszLocalStorePath = NULL, TRANSFER_TYPE nTranserType = TT_BINARY);
	unsigned long long FileUpload(LPCTSTR lpszFTPPath, LPCTSTR lpszLocalFile = NULL, TRANSFER_TYPE nTranserType = TT_BINARY);
	unsigned long long FileSize(LPCTSTR lpszFTPFile);
	bool FileDelete(LPCTSTR lpszFTPFile);
	bool FileRename(LPCTSTR lpszFTPFile, LPCTSTR lpszNewName);

	//absolute path operation function
	bool DirectoryList(LPCTSTR lpszFTPDirectory, CString & strOut);
	bool DirectoryList(LPCTSTR lpszFTPDirectory, CStringArray & strOutNameArray, CStringArray & strOutSizeArray, CStringArray & strOutMdTimeArray);
	bool DirectoryOpen(LPCTSTR lpszFTPDirectory);
	bool DirectoryCreate(LPCTSTR lpszFTPDirectory);
	bool DirectoryDelete(LPCTSTR lpszFTPDirectory);
	
	//relative path operation function (in current working directory)
	bool FolderList(LPCTSTR lpszFolderName, CString & strOut);
	bool FolderList(LPCTSTR lpszFolderName, CStringArray & strOutNameArray, CStringArray & strOutSizeArray, CStringArray & strOutMdTimeArray);
	bool FolderOpen(LPCTSTR lpszFolderName);
	bool FolderCreate(LPCTSTR lpszFolderName);
	bool FolderDelete(LPCTSTR lpszFolderName);
	
	//get file or folder modified time
	CString GetModifiedTime(LPCTSTR lpszFTPObject);

	//get current working directory
	CString GetWorkingDirectory(void);

	//static functions
	static void About(void);
	static CString Version(void);
	static bool SaveLog(LPCTSTR lpszLog);

private:
	void CheckFTPProfile(void);

	bool SendDataPort(UINT nPort);
	bool GetDataPort(UINT & nPort);

public:
	bool m_bRecordLog;

private:
	CString m_strFTPServer;
	CString m_strFTPUser;
	CString m_strFTPPassword;
	UINT m_nFTPPort;

	CString m_strProxyAuthorization;
	CString m_strProxyServer;
	UINT m_nProxyPort;

	CString m_strServerReply;

	CString m_strWorkingPath;	
	bool m_bPassiveMode;
	PROXY_TYPE m_nProxyType;
	ftp_profile m_pProfile;

	HWND m_hWnd;
};