/********************************
 Scope:		CNetwork
 Author:	Walnut
 Updated:	2012-08-08
********************************/

#include "StdAfx.h"
#include "Network.h"
#include "Timer.h"

IMPLEMENT_DYNAMIC(CSocketObject, CSocket)

CSocketObject::CSocketObject(CNetwork * pNetwork)
{
	VERIFY(NULL != pNetwork);
	this->m_pNetwork = pNetwork;
}

CSocketObject::~CSocketObject()
{
}

void CSocketObject::OnAccept(int nErrorCode)
{
	switch(nErrorCode)
	{
	case 0:
		this->m_pNetwork->AcceptConnection();
		break;
	case WSAENETDOWN:
		AfxMessageBox("The network subsystem failed!");
		break;
	default:
		break;
	}
	
	CSocket::OnAccept(nErrorCode);
}

void CSocketObject::OnClose(int nErrorCode)
{
	switch(nErrorCode)
	{
	case 0:
		//detect which client is closed
		break;
	case WSAENETDOWN:
		AfxMessageBox("The network subsystem failed!");
		break;
	case WSAECONNRESET:
		AfxMessageBox("The connection was reset by the remote side!");
		break;
	case WSAECONNABORTED:
		AfxMessageBox("The connection was aborted due to timeout or other failure!");
		break;
	default:
		break;
	}
	CSocket::OnClose(nErrorCode);
}

void CSocketObject::OnReceive(int nErrorCode)
{
	DWORD nValue;

	switch(nErrorCode)
	{
	case 0:
		if(FALSE == this->IOCtl(FIONREAD, &nValue))
		{AfxMessageBox("Failed to get data size!");}
		break;
	case WSAENETDOWN:
		AfxMessageBox("The network subsystem failed!");
		break;
	case WSAECONNRESET:
		AfxMessageBox("The connection was reset by the remote side!");
		break;
	case WSAECONNABORTED:
		AfxMessageBox("The connection was aborted due to timeout or other failure!");
		break;
	default:
		break;
	}
	
	CSocket::OnReceive(nErrorCode);
}

void CSocketObject::OnSend(int nErrorCode)
{

	CSocket::OnSend(nErrorCode);
}

CNetwork::CNetwork(HWND hWnd):
				 m_hWnd(hWnd)
				,m_psockClient(NULL)
				,m_pfileCommand(NULL)
				,m_parchReceiver(NULL)
				,m_parchTransmiter(NULL)
				,m_nDataBufferSize(8688)
{
	if(!AfxSocketInit(NULL)) 
	{
		AfxMessageBox("Failed to initialize socket!");
		return;
	}

	this->m_psockClient =new CSocket;
	VERIFY(NULL != this->m_psockClient);
	
	this->m_psockListen = new CSocketObject(this);
	VERIFY(NULL != this->m_psockListen);

	/*this->m_pszReceiveBuffer = new char[m_nDataBufferSize];
	VERIFY(NULL != this->m_pszReceiveBuffer);
	memset(m_pszReceiveBuffer, 0, m_nDataBufferSize);*/
}

CNetwork::~CNetwork(void)
{
	this->ReleaseClientObject();

	if(this->m_psockClient)
	{
		this->m_psockClient->Close();
		delete this->m_psockClient;
		this->m_psockClient = NULL;
	}

	if(NULL != this->m_psockListen)
	{
		this->m_psockListen->Close();
		delete this->m_psockListen;
		this->m_psockListen = NULL;
	}

	for(int n = 0; n < this->m_psockConnections.GetCount(); n ++)
	{
		if(NULL != this->m_psockConnections[n])
		{
			this->m_psockConnections[n]->Close();
			delete this->m_psockConnections[n];
			this->m_psockConnections[n] = NULL;
		}
	}
}

bool CNetwork::CreateClientObject(void)
{
	BOOL bKeep = TRUE;

	this->ReleaseClientObject();

 	if(!this->m_psockClient->Create()) //port set to 0, system will allocate an unused port
	{
		AfxMessageBox("Failed to create socket!");
		return false;
	}
	if(!this->m_psockClient->SetSockOpt(SO_KEEPALIVE, &bKeep, sizeof(BOOL)))
	{
		AfxMessageBox("Failed to set option for network socket!");
		return false;
	}

	this->m_pfileCommand = new CSocketFile(m_psockClient);
	this->m_parchReceiver = new CArchive(m_pfileCommand, CArchive::load);
	this->m_parchTransmiter = new CArchive(m_pfileCommand, CArchive::store);

	VERIFY(NULL != this->m_pfileCommand);
	VERIFY(NULL != this->m_parchReceiver);
	VERIFY(NULL != this->m_parchTransmiter);

	return true;
}

void CNetwork::ReleaseClientObject(void)
{
	if(this->m_parchReceiver)
	{
		this->m_parchReceiver->Close();
		delete this->m_parchReceiver;
		this->m_parchReceiver = NULL;
	}

	if(this->m_parchTransmiter)
	{
		this->m_parchTransmiter->Close();
		delete this->m_parchTransmiter;
		this->m_parchTransmiter = NULL;
	}

	if(this->m_pfileCommand)
	{
		this->m_pfileCommand->Close();
		delete this->m_pfileCommand;
		this->m_pfileCommand = NULL;
	}

	this->m_psockClient->Close();
}

bool CNetwork::Connect(LPCTSTR lpszAddress, UINT nPort)
{
	if(!this->CreateClientObject())
	{
		AfxMessageBox("Failed to create network resources!");
		return false;
	}

	if(!this->m_psockClient->Connect(lpszAddress, nPort))
	{
		AfxMessageBox("Failed to connect to remote server!");
		return false;
	}
	return true;
}

void CNetwork::Disconnect(void)
{
	ReleaseClientObject();
	this->m_psockClient->Close();
}

bool CNetwork::StartServer(UINT nPort)
{
	this->m_psockListen->Close();
	if(FALSE == this->m_psockListen->Create(nPort))
	{
		AfxMessageBox("Failed to create listen socket!");
		return false;
	}
	
	//non blocking mode
	if(FALSE == this->m_psockListen->AsyncSelect()) 
	{
		AfxMessageBox("Failed to asyncselect!");
		return false;
	}

	if(FALSE == m_psockListen->Listen())
	{
		AfxMessageBox("Failed to listen!");
		return false;
	}

	return true;
}

void CNetwork::StopServer(void)
{
	this->m_psockListen->Close();
}

bool CNetwork::AcceptConnection(void)
{
	CSocket * psockClient = new CSocket;
	VERIFY(NULL != psockClient);

	if(FALSE == this->m_psockListen->Accept(*psockClient))
	{
		AfxMessageBox("Failed to accept!");
		return false;
	}
	
	//debug
	psockClient->Send("Hello", 5);

	this->m_psockConnections.Add(psockClient);

	return true;
}

int CNetwork::GetConnectionCount(void)
{
	return (int)this->m_psockConnections.GetCount();
}

CSocket * CNetwork::GetConnection(int nIndex) const
{
	if(nIndex < 0 || nIndex >= this->m_psockConnections.GetCount())
	{return NULL;}
	else
	{return this->m_psockConnections.GetAt(nIndex);}
}

void CNetwork::RemoveConnection(int nIndex)
{
	if(nIndex < 0 || nIndex >= this->m_psockConnections.GetCount())
	{return;}
	else
	{return this->m_psockConnections.RemoveAt(nIndex);}
}

bool CNetwork::SendSimpleData(LPCTSTR lpszData)
{
	try
	{
		this->m_parchTransmiter->WriteString(lpszData);
		this->m_parchTransmiter->Flush();
		Sleep(10);
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
		return false;
	}
	return true;
}

bool CNetwork::ReceiveSimpleData(CString & strReceived)
{
	try
	{
		if(FALSE == this->m_parchReceiver->ReadString(strReceived))
		{
			AfxMessageBox("Failed to get archive message!");
			return false;
		}
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
		return false;
	}

	if(this->m_hWnd)
	{
		SendMessage(this->m_hWnd, UM_NETWORK_MESSAGE, (WPARAM)strReceived.GetString(), 0);
	}

	return true;
}

bool CNetwork::SendRawData(LPCTSTR lpszData, UINT nDataLen)
{
	if(0 == nDataLen)
	{nDataLen = (UINT)strlen(lpszData);}
	if(0 == nDataLen)
	{return false;}

	try
	{
		this->m_parchTransmiter->Write(lpszData, nDataLen);
		this->m_parchTransmiter->Flush();
		Sleep(10);
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
		return false;
	}
	return true;
	//return this->SendRawData(this->m_psockClient, lpszData, nDataLen);
}

bool CNetwork::SendRawData(CSocket * pSocket, LPCTSTR lpszData, UINT nDataLen)
{
	VERIFY(NULL != pSocket);

	//if(NULL == lpszData) return 0;
	//int nSendBytes = -1;
	//CString strMsg;

	CSocketFile * psockFile = NULL;
	CArchive * parchTransmiter = NULL;
	
	if(0 == nDataLen) 
	{nDataLen = (UINT)strlen(lpszData);}
	if(0 == nDataLen)
	{return false;}

	try
	{	
		psockFile = new CSocketFile(pSocket);
		VERIFY(NULL != psockFile);
		parchTransmiter = new CArchive(psockFile, CArchive::store);
		VERIFY(NULL != parchTransmiter);

		parchTransmiter->Write(lpszData, nDataLen);
		parchTransmiter->Flush();
		Sleep(50);

		parchTransmiter ->Close();
		delete parchTransmiter;
		parchTransmiter = NULL;
		
		psockFile->Close();
		delete psockFile;
		psockFile = NULL;
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
		return false;
	}
	return true;

	/*try
	{
		nSendBytes = pSocket->Send(lpszData, nDataLen);

		if(SOCKET_ERROR == nSendBytes || nDataLen != nSendBytes)
		{
			strMsg.Format("Error occured when send socket data!\nError code: %d\nExpect send bytes:%d, Actual send byes: %d", GetLastError(), nDataLen, nSendBytes);
			AfxMessageBox(strMsg);
			return false;
		}
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
	}

	return true;*/
}

bool CNetwork::ReceivedRawData(CSocket * pSocket, LPTSTR lpszBuffer, UINT nBufferSize, int & nReceivedLen)
{
	try
	{
		nReceivedLen = pSocket->Receive(lpszBuffer, nBufferSize);
	}
	catch(CException * e)
	{
		e->ReportError();
		e->Delete();
		return false;
	}

	if(SOCKET_ERROR == nReceivedLen)
	{
		CString strMsg;
		strMsg.Format("Error occured when receive socket data!\nError code: %d", GetLastError());
		AfxMessageBox(strMsg);
		return false;
	}

	return true;
}

bool CNetwork::ReceiveRawData(CString & strReceived)
{
	//UINT n  = 0;
	//do{
	//	try
	//	{
	//		
	//		n = this->m_pfileCommand->GetLength();
	//		//this->m_parchReceiver->ReadString(this->m_pszReceiveBuffer, this->m_nDataBufferSize - 1);
	//		//this->ReceiveSimpleData(strReceived);
	//		//memset(this->m_pszReceiveBuffer, 0, this->m_nDataBufferSize);
	//		//UINT nRead = this->m_parchReceiver->Read(this->m_pszReceiveBuffer, 1);//this->m_nDataBufferSize - 1);
	//		//{
	//		//	AfxMessageBox("Failed to get archive message!");
	//		//	return false;
	//		//}
	//		//memcpy_s(strReceived
	//	}
	//	catch(CException * e)
	//	{
	//		e->ReportError();
	//		e->Delete();
	//		return false;
	//	}
	//}
	//while(FALSE == this->m_parchReceiver->IsBufferEmpty());
	//return true;
	return this->ReceiveRawData(this->m_psockClient, strReceived);
}

bool CNetwork::ReceiveRawData(CSocket * pSocket, CString & strReceived)
{
	VERIFY(NULL != pSocket);

	char * pDataBuffer = NULL;
	int nReadBytes = -1;
	CString strMsg= "";
	bool bRet = true;
	
	strReceived.Empty();
	pDataBuffer = new char[this->m_nDataBufferSize + 1];

	while(true)
	{
		memset(pDataBuffer, 0, m_nDataBufferSize + 1);

		bRet = this->ReceivedRawData(pSocket, pDataBuffer, m_nDataBufferSize, nReadBytes);
		if(!bRet || 0 == nReadBytes)
		{break;}

		strReceived.Append(pDataBuffer);
		if(nReadBytes < m_nDataBufferSize && pSocket == this->m_psockClient) 
		{break;}

		Sleep(10);
	}

	delete[] pDataBuffer;
	
	if(this->m_hWnd)
	{
		SendMessage(this->m_hWnd, UM_NETWORK_MESSAGE, (WPARAM)strReceived.GetString(), 0);
	}

	return bRet;
}

int CNetwork::ReceiveRawData(CString & strReceived, int nExpectLen)
{
	//char * pszData = new char[nExpectLen + 1];
	//int nDataLen = this->ReceiveRawData(this->m_psockClient, pszData, nExpectLen);
	
	//if(0 != memcpy_s(strReceived.GetBuffer(nExpectLen), nExpectLen, pszData, nExpectLen))
	//{nDataLen = 0;}
	//strReceived.ReleaseBuffer(nExpectLen);

	//return nDataLen;
	return this->ReceiveRawData(this->m_psockClient, strReceived, nExpectLen);
}

int CNetwork::ReceiveRawData(CSocket * pSocket, CString & strReceived, int nExpectLen)
{
	VERIFY(NULL != pSocket/* && NULL != lpszBuffer*/);

	char * pDataBuffer = NULL;
	int nReadBytes = -1;
	CString strMsg= "";
	int nTotalReceived = 0;
	bool bRet = true;
	
	strReceived.Empty();
	pDataBuffer = new char[this->m_nDataBufferSize + 1];

	while(true)
	{
		memset(pDataBuffer, 0, m_nDataBufferSize + 1);

		bRet = this->ReceivedRawData(pSocket, pDataBuffer, m_nDataBufferSize, nReadBytes);
		if(!bRet || 0 == nReadBytes)
		{break;}
		
		nTotalReceived += nReadBytes;
		strReceived.GetBuffer(nTotalReceived);
		strReceived.Append(pDataBuffer);
		strReceived.ReleaseBuffer(nTotalReceived);

		if(nTotalReceived >= nExpectLen) 
		{break;}

		Sleep(10);
	}
	delete[] pDataBuffer;

	if(this->m_hWnd)
	{SendMessage(this->m_hWnd, UM_NETWORK_MESSAGE, (WPARAM)strReceived.GetString(), 0);	}

	return nTotalReceived;
}

unsigned __int64 CNetwork::SendFileData(CSocket * pSocket,  LPCTSTR lpszFileName)
{
	VERIFY(NULL != pSocket/* && NULL != lpszFileName*/);
	
	CFile file;
	unsigned __int64 nFileSize = 0;
	unsigned __int64 nTotalSendBytes = 0;
	int nReadBytes = 0;
	int nSendBytes = 0;
	char * pDataBuffer = NULL;

	CTimer tm;
	double fElapsedSeconds;

	if(!file.Open(lpszFileName, CFile::modeRead | CFile::shareExclusive)) 
	{
		AfxMessageBox("Failed to open file to read!");
		return 0;
	}

	nFileSize = file.GetLength();
	if(0 == nFileSize)
	{
		AfxMessageBox("File size is 0!");
		return 0;
	}

	if(this->m_hWnd) 
	{SendMessage(this->m_hWnd, UM_NETWORK_PROCESS, 0, 0);}

	tm.Start();
	pDataBuffer = new char[this->m_nDataBufferSize + 1];

	while(true)
	{
		//tm.Reset();
		memset(pDataBuffer, 0, this->m_nDataBufferSize + 1);

		nReadBytes = file.Read(pDataBuffer, this->m_nDataBufferSize);
		if(nReadBytes <= 0)
		{break;}

		try
		{
			nSendBytes = pSocket->Send(pDataBuffer, nReadBytes);
		}
		catch(CException * e)
		{
			e->ReportError();
			e->Delete();
			break;
		}

		if(SOCKET_ERROR == nSendBytes || nSendBytes != nReadBytes)
		{
			AfxMessageBox("Error occured when upload data!");
			break;
		}

		nTotalSendBytes += nSendBytes;
		fElapsedSeconds = tm.GetElapsedSeconds();

		if(this->m_hWnd) 
		{SendMessage(this->m_hWnd, UM_NETWORK_PROCESS, int(100*nTotalSendBytes/nFileSize), LONG(nTotalSendBytes/fElapsedSeconds));}
	}

	delete[] pDataBuffer;
	file.Close();
	tm.Stop();

	return nTotalSendBytes;
}

unsigned __int64 CNetwork::ReceiveFileData(CSocket * pSocket, LPCTSTR lpszFileName, unsigned __int64 nExpectLen, bool bOverwtire)
{
	VERIFY(NULL != pSocket/* && NULL != lpszFileName*/);
	
	CFile file;
	int nReadBytes = -1;
	unsigned __int64 nTotalReceived = 0;
	char * pDataBuffer = NULL;
	UINT nOpenFlag;
	bool bRet = true;

	CTimer tm;
	double fElapsedSeconds;

	if(bOverwtire)
	{nOpenFlag = CFile::modeCreate | CFile::modeWrite | CFile::typeBinary;}
	else
	{nOpenFlag = CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeBinary;};

	if(!file.Open(lpszFileName, nOpenFlag))
	{
		AfxMessageBox("Failed to open file to write!");
		return 0;
	}	

	if(this->m_hWnd) 
	{SendMessage(this->m_hWnd, UM_NETWORK_PROCESS, 0, 0);}

	tm.Start();
	pDataBuffer = new char[this->m_nDataBufferSize + 1];

	while(true)
	{
		//tm.Reset();
		memset(pDataBuffer, 0, this->m_nDataBufferSize + 1);

		bRet = this->ReceivedRawData(pSocket, pDataBuffer, this->m_nDataBufferSize, nReadBytes);
		if(!bRet || 0 == nReadBytes) 
		{break;}

		file.SeekToEnd();
		file.Write(pDataBuffer, nReadBytes);
		//Sleep(10);

		nTotalReceived += nReadBytes;
		fElapsedSeconds = tm.GetElapsedSeconds();

		if(this->m_hWnd)
		{SendMessage(this->m_hWnd, UM_NETWORK_PROCESS, UINT(100*nTotalReceived/nExpectLen), LONG(nTotalReceived/fElapsedSeconds));}

		if(nTotalReceived >= nExpectLen) 
		{break;}
	}

	delete[] pDataBuffer;
	file.Close();

	return nTotalReceived;
}

UINT CNetwork::GetSocketPort(void) const
{
	return this->GetSocketPort(this->m_psockClient);
}

UINT CNetwork::GetSocketPort(CSocket * pSocket) const
{
	VERIFY(NULL != pSocket);
	CString strTmp = "";
	UINT nPort = 0;
	if(0 == pSocket->GetSockName(strTmp, nPort))
	{
		AfxMessageBox("Failed to get socket port!");
	}
	return nPort;
}

bool CNetwork::SetSocketBufferSize(UINT nSendSize_Byte, UINT nReceiveSize_Byte)
{
	return this->SetSocketBufferSize(this->m_psockClient, nSendSize_Byte, nReceiveSize_Byte);
}

bool CNetwork::SetSocketBufferSize(CSocket * pSocket, UINT nSendSize_Byte, UINT nReceiveSize_Byte)
{
	VERIFY(NULL != pSocket);

	if(!pSocket->SetSockOpt(SO_SNDBUF, (const char*)&nSendSize_Byte, sizeof(UINT), SOL_SOCKET))
	{
		AfxMessageBox("Failed to set send buffer size for data!");
		return false;
	}
	if(!pSocket->SetSockOpt(SO_RCVBUF, (const char*)&nReceiveSize_Byte, sizeof(UINT), SOL_SOCKET))
	{
		AfxMessageBox("Failed to set receive buffer size for data!");
		return false;
	}

	return true;
}

bool CNetwork::GetSocketBufferSize(UINT & nSendSize_Byte, UINT & nReceiveSize_Byte)
{
	return this->GetSocketBufferSize(this->m_psockClient, nSendSize_Byte, nReceiveSize_Byte);
}

bool CNetwork::GetSocketBufferSize(CSocket * pSocket, UINT & nSendSize_Byte, UINT & nReceiveSize_Byte)
{
	VERIFY(NULL != pSocket);

	if(!pSocket->GetSockOpt(SO_SNDBUF, &nSendSize_Byte, NULL))
	{
		AfxMessageBox("Failed to get send buffer size for data!");
		return false;
	}

	if(!pSocket->GetSockOpt(SO_RCVBUF, &nReceiveSize_Byte, NULL))
	{
		AfxMessageBox("Failed to get receive buffer size for data!");
		return false;
	}

	return true;
}

bool CNetwork::SetSocketTimeout(int nSendTimeout, int nReceiveTimeout)
{
	return this->SetSocketTimeout(this->m_psockClient, nSendTimeout, nReceiveTimeout);
}

bool CNetwork::SetSocketTimeout(CSocket * pSocket, int nSendTimeout, int nReceiveTimeout)
{
	VERIFY(NULL != pSocket);
	
	if(!pSocket->SetSockOpt(SO_SNDTIMEO, (const char*)&nSendTimeout, sizeof(int), SOL_SOCKET))
	{
		AfxMessageBox("Failed to set sending timeout for socket!");
		return false;
	}
	if(!pSocket->SetSockOpt(SO_SNDTIMEO, (const char*)&nReceiveTimeout, sizeof(int), SOL_SOCKET))
	{
		AfxMessageBox("Failed to set receiving timeout for socket!");
		return false;
	}

	return true;
}

CString CNetwork::GetLocalIPAddress(void) const
{
	CString strIP = "";
	UINT nPort = 0;
	if(0 == this->m_psockClient->GetSockName(strIP, nPort))
	{
		AfxMessageBox("Failed to get local IP address!");
	}
	return strIP;
}

CString CNetwork::GetLocalHostName(void) const
{
	CString strName = "";
	UINT nPort = 0;
	gethostname(strName.GetBuffer(MAX_PATH), MAX_PATH);
	strName.ReleaseBuffer(MAX_PATH);
	return strName;
}

void CNetwork::SetDataBufferSize(int nBytes)
{
	this->m_nDataBufferSize = nBytes;
}

int CNetwork::GetDataBufferSize(void)
{
	return this->m_nDataBufferSize;
}