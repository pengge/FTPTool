/********************************
 Scope:		CNetwork
 Author:	Walnut
 Updated:	2012-07-25
********************************/

#pragma once
#include <afxsock.h>

#define UM_NETWORK_MESSAGE	 WM_USER + 100 //WPARAM is the string message received from network
#define UM_NETWORK_PROCESS	 WM_USER + 101 //WPARAM is the finished percent, LPARAM is the average B/S speed

class CNetwork;

class CSocketObject : public CSocket
{
	DECLARE_DYNAMIC(CSocketObject)

public:
	CSocketObject(CNetwork * pNetwork);
	virtual ~CSocketObject();

public:
	virtual void OnAccept(int nErrorCode); //non blocking mode
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);

private:
	CNetwork * m_pNetwork;
};

class CNetwork
{
public:
	CNetwork(HWND hWnd = NULL);
	~CNetwork(void);

	bool Connect(LPCTSTR lpszAddress, UINT nPort);
	void Disconnect(void);

	bool StartServer(UINT nPort);
	void StopServer(void);
	bool AcceptConnection(void);
	int GetConnectionCount(void);
	CSocket * GetConnection(int nIndex) const;
	void RemoveConnection(int nIndex);

	bool SendSimpleData(LPCTSTR lpszData); //send a line through archive
	bool ReceiveSimpleData(CString & strReceived); //receive a line through archive
	
	bool SendRawData(LPCTSTR lpszData, UINT nDataLen = 0); //send raw data
	bool SendRawData(CSocket * pSocket, LPCTSTR lpszData, UINT nDataLen = 0);

	bool ReceivedRawData(CSocket * pSocket, LPTSTR lpszBuffer, UINT nBufferSize, int & nReceivedLen);

	bool ReceiveRawData(CString & strReceived); //receive all message in the socket buffer
	bool ReceiveRawData(CSocket * pSocket, CString & strReceived);

	int ReceiveRawData(CString & strReceived, int nExpectLen);
	int ReceiveRawData(CSocket * pSocket, CString & strReceived, int nExpectLen); //want to receive data that length is nExpectLen at least 

	unsigned __int64 SendFileData(CSocket * pSocket,  LPCTSTR lpszFileName); //send all data of a file
	unsigned __int64 ReceiveFileData(CSocket * pSocket, LPCTSTR lpszFileName, unsigned __int64 nExpectLen, bool bOverwtire = true); //receive data and store to a local file
	
	UINT GetSocketPort(void) const;
	UINT GetSocketPort(CSocket * pSocket) const;

	bool SetSocketBufferSize(UINT nSendSize_Byte, UINT nReceiveSize_Byte);
	bool SetSocketBufferSize(CSocket * pSocket, UINT nSendSize_Byte, UINT nReceiveSize_Byte);
	bool GetSocketBufferSize(UINT & nSendSize_Byte, UINT & nReceiveSize_Byte);
	bool GetSocketBufferSize(CSocket * pSocket, UINT & pnSendSize_Byte, UINT & nReceiveSize_Byte);

	bool SetSocketTimeout(int nSendTimeout, int nReceiveTimeout);
	bool SetSocketTimeout(CSocket * pSocket, int nSendTimeout, int nReceiveTimeout);

	CString GetLocalIPAddress(void) const;
	CString GetLocalHostName(void) const;

	void SetDataBufferSize(int nBytes);
	int GetDataBufferSize(void);

private:
	bool CreateClientObject(void);
	void ReleaseClientObject(void);

private:
	HWND m_hWnd;

	CSocket * m_psockClient;
	CSocketFile * m_pfileCommand;
	CArchive * m_parchReceiver;
	CArchive * m_parchTransmiter;

	//char * m_pszReceiveBuffer;
	int m_nDataBufferSize;

	CSocketObject * m_psockListen;
	CArray<CSocket *> m_psockConnections;
};
