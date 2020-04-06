/********************************
 Scope:		CFTPClient
 Author:	Walnut
 Updated:	2012-08-08
 Version 1.1.1
********************************/
#include "stdafx.h"
#include <Winuser.h>
#include <Windows.h>
#include <Wininet.h>
#include <shlwapi.h>
#include <afxdlgs.h>
#include <atlenc.h>
#include "ftpclient.h"
#include "globalfunctions.h"

#pragma comment(lib, "WinInet.Lib")

CFTPClient::CFTPClient(LPCTSTR lpszFTPServer, LPCTSTR lpszFTPUser, LPCTSTR lpszFTPPassword, UINT nFTPPort, HWND hWnd) : CNetwork(hWnd),
				m_strFTPServer(lpszFTPServer),
				m_strFTPUser(lpszFTPUser),
				m_strFTPPassword(lpszFTPPassword),
				m_nFTPPort(nFTPPort),

				m_strProxyAuthorization(""),
				m_strProxyServer(""),
				m_nProxyPort(0),

				m_strServerReply(""),

				m_bPassiveMode(false),
				m_nProxyType(PROXY_NONE),
				m_strWorkingPath("/"),
				m_pProfile(NULL),

				m_hWnd(hWnd),
				m_bRecordLog(false)
{
	this->m_pProfile = new FTP_PROFILE;
}

CFTPClient::~CFTPClient(void)
{
	if(this->m_pProfile)
	{
		delete m_pProfile;
		m_pProfile = NULL;
	}
}

bool CFTPClient::Login(void)
{
	CString strCmd = "";

	if(PROXY_NONE == this->m_nProxyType)
	{
		if(!this->Connect(this->m_strFTPServer, this->m_nFTPPort))
		{return false;}
		if(!this->ReceiveRawData((this->m_strServerReply)))
		{return false;}
		if("220" != this->m_strServerReply.Left(3))
		{
			//should should reply 220 if server is ready
			SaveLog(this->m_strServerReply);
			return false;
		}
	}
	
	if(PROXY_HTTP11_GET != this->m_nProxyType)
	{
		strCmd.Format("USER %s\r\n", this->m_strFTPUser);
		if(!this->SendSimpleData(strCmd))
		{return false;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return false;}

		if("331" == this->m_strServerReply.Left(3))//need password
		{
			strCmd.Format("PASS %s\r\n", this->m_strFTPPassword);
			if(!this->SendSimpleData(strCmd))
			{return false;}
			if(!this->ReceiveRawData(this->m_strServerReply))
			{return false;}

			if("530" == this->m_strServerReply.Left(3))//password error
			{
				SaveLog(this->m_strServerReply + "\nPassword error!");
				return false;
			}
			else if("230" != this->m_strServerReply.Left(3)) 
			{
				SaveLog(this->m_strServerReply + "\nFailed to login!");
				return false;
			}
		}
		else if("230" != this->m_strServerReply.Left(3)) 
		{
			SaveLog(this->m_strServerReply + "\nFailed to login!");
			return false;
		}
		
		this->CheckFTPProfile();
	}
	
	this->m_strWorkingPath = this->GetWorkingDirectory();
	if(this->m_bRecordLog) 
	{
		strCmd.Format("Login okay (server:%s, user:%s, port:%d, client:%s)", this->m_strFTPServer, this->m_strFTPUser, this->m_nFTPPort, this->GetLocalIPAddress());
		this->SaveLog(strCmd);
	}

	return true;
}

void CFTPClient::Logout(void)
{
	if(PROXY_HTTP11_GET == this->m_nProxyType) 
	{return;}

	this->SendSimpleData("QUIT \r\n");
	this->ReceiveSimpleData(this->m_strServerReply);
	if("221" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply);
	}

	if(this->m_bRecordLog) 
	{this->SaveLog("Logout okay\r\n");}
	//Sleep(1000);
}

bool CFTPClient::SetProxy(PROXY_TYPE nProxyType, LPCTSTR lpszProxyServer, UINT nProxyPort, LPCTSTR lpszProxyUser, LPCTSTR lpszProxyPassword)
{
	if(NULL == lpszProxyServer || PROXY_NONE == nProxyType) 
	{return false;}
	
	CString strCmd = "";
	unsigned char szAuth[64] = "";
	char szBase64[128] = "";
	int nBase64Len = 128;
	CSocket sockProxy;

	memset(szAuth, 0, 64);
	memset(szBase64, 0, 128);

	switch(nProxyType)
	{
	case PROXY_HTTP11_GET:
		if(!sockProxy.Create())
		{
			SaveLog("Failed to create proxy socket!");
			return false;
		}
		if(!sockProxy.Connect(lpszProxyServer, nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return false;
		}
		if(NULL != lpszProxyUser && "" != CString(lpszProxyUser))
		{
			sprintf_s((char *)szAuth, sizeof(szAuth), "%s:%s", lpszProxyUser, lpszProxyPassword);
			nBase64Len = sizeof(szBase64);
			Base64Encode(szAuth,  (int)strlen((char *)szAuth), szBase64, &nBase64Len);

			strCmd.Format("HEAD ftp://%s:%s@%s:%d/ HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n"
								//"Proxy-Connection: keep-alive\r\n"
								"User-Agent: Mozilla/4.0 (compatible; FTP; Windows XP)\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strFTPServer, this->m_nFTPPort, 
								szBase64);
		}
		else
		{
			strCmd.Format("HEAD ftp://%s:%s@%s:%d/ HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								//"Proxy-Connection: keep-alive\r\n"
								"User-Agent: Mozilla/4.0 (compatible; FTP; Windows XP)\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strFTPServer, this->m_nFTPPort);
		}

		if(!this->SendRawData(&sockProxy, strCmd))
		{
			sockProxy.Close();
			return false;
		}
		if(!this->ReceiveRawData(&sockProxy, this->m_strServerReply))
		{
			sockProxy.Close();
			return false;
		}
		sockProxy.Close();
		break;
	case PROXY_HTTP11_CONNECT:
		if(!this->Connect(lpszProxyServer, nProxyPort))
		{return false;}

		if(NULL != lpszProxyUser && "" != CString(lpszProxyUser))
		{
			sprintf_s((char *)szAuth, sizeof(szAuth), "%s:%s", lpszProxyUser, lpszProxyPassword);
			nBase64Len = sizeof(szBase64);
			Base64Encode(szAuth,  (int)strlen((char *)szAuth), szBase64, &nBase64Len);

			strCmd.Format("CONNECT %s:%d HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n"
								"Proxy-Connection: keep-alive\r\n\r\n",
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strFTPServer, this->m_nFTPPort, 
								szBase64);
		}
		else
		{
			strCmd.Format("CONNECT %s:%d HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Connection: keep-alive\r\n\r\n",
								this->m_strFTPServer, this->m_nFTPPort, this->m_strFTPServer, this->m_nFTPPort);
		}
		if(!this->SendRawData(strCmd)) 
		{return false;}
		if(!this->ReceiveRawData(this->m_strServerReply))
		{return false;}
		break;
	case PROXY_SOCKS5:
		char szBuffer[128];
		unsigned short uPort;
		memset(szBuffer, 0, sizeof(szBuffer));
		uPort = 0;

		if(!this->Connect(lpszProxyServer, nProxyPort))
		{return false;}
		if(NULL != lpszProxyUser && "" != CString(lpszProxyUser))
		{
			szBuffer[0] = 5;
			szBuffer[1] = 2;
			szBuffer[2] = 0;
			szBuffer[3] = 2;
			if(!this->SendRawData(szBuffer, 4))
			{return false;}

			if(!this->ReceiveRawData(this->m_strServerReply))
			{return false;}
			strcpy_s(szBuffer, sizeof(szBuffer), this->m_strServerReply);
			if(5 != szBuffer[0])
			{return false;}

			if(2 == szBuffer[1]) //2 indicate need authentication
			{
				memset(szBuffer, 0, sizeof(szBuffer));
				szBuffer[0] = 1;
				szBuffer[1] = (int)strlen(lpszProxyUser);
				memcpy(szBuffer + 2, lpszProxyUser, strlen(lpszProxyUser));
				szBuffer[2 + szBuffer[1]] = (int)strlen(lpszProxyPassword);
				memcpy(szBuffer + 3 + szBuffer[1], lpszProxyPassword, strlen(lpszProxyPassword));
				
				if(!this->SendRawData(szBuffer, 3 + szBuffer[1] + szBuffer[2 + szBuffer[1]]))
				{return false;}
				if(!this->ReceiveRawData(this->m_strServerReply))
				{return false;}

				strcpy_s(szBuffer, sizeof(szBuffer), this->m_strServerReply);
				if(0 != szBuffer[1]) //0 indicate user and password is okay
				{return false;}
			}
			else if(0 != szBuffer[1]) //0 indicate donot need authentication
			{
				return false;
			}
		}
		else
		{
			memset(szBuffer, 0, sizeof(szBuffer));
			szBuffer[0] = 5;
			szBuffer[1] = 1;
			szBuffer[2] = 0;

			if(!this->SendRawData(szBuffer, 3))
			{return false;}
			if(!this->ReceiveRawData(this->m_strServerReply))
			{return false;}

			strcpy_s(szBuffer, sizeof(szBuffer), this->m_strServerReply);
			if(szBuffer[0] != 5 || szBuffer[1] != 0)
			{return false;}
		}

		//translate target address
		memset(szBuffer, 0, sizeof(szBuffer));
		szBuffer[0] = 5;
		szBuffer[1] = 1;//connect
		szBuffer[2] = 0;
		szBuffer[3] = 3;//domain
		szBuffer[4] = (int)strlen(this->m_strFTPServer);//domain len
		memcpy(szBuffer + 5, this->m_strFTPServer, strlen(this->m_strFTPServer));

		uPort = htons(this->m_nFTPPort);
		memcpy(szBuffer + 5 + szBuffer[4], &uPort, 2);

		if(!this->SendRawData(szBuffer, 7 + szBuffer[4]))
		{return false;}
		if(!this->ReceiveRawData(this->m_strServerReply))
		{return false;}

		strcpy_s(szBuffer, sizeof(szBuffer), this->m_strServerReply);
		if(szBuffer[0] != 5 || szBuffer[1] != 0)
		{return false;}
		break;
	default:
		return false;
	}

	switch(nProxyType)
	{
	case PROXY_SOCKS5:
		if(this->m_bRecordLog) 
		{
			strCmd.Format("Set socks5 proxy okay (server:%s, user:%s, port:%d)", lpszProxyServer, lpszProxyUser, nProxyPort);
			this->SaveLog(strCmd);
		}
		break;
	case PROXY_HTTP11_GET:
	case PROXY_HTTP11_CONNECT:
		if("HTTP/1.0 200" != this->m_strServerReply.Left(12) && "HTTP/1.1 200" != this->m_strServerReply.Left(12)) 
		{return false;}
		if(this->m_bRecordLog) 
		{
			strCmd.Format("Set http proxy okay (server:%s, user:%s, port:%d)", lpszProxyServer, lpszProxyUser, nProxyPort);
			this->SaveLog(strCmd);
		}
		break;
	default:
		break;
	}

	this->m_nProxyType = nProxyType;
	this->m_strProxyServer = lpszProxyServer;
	this->m_strProxyAuthorization = szBase64;
	this->m_nProxyPort = nProxyPort;

	return true;
}

void CFTPClient::SetPassiveMode(bool bPassive)
{
	this->m_bPassiveMode = bPassive;
}

void CFTPClient::SetBufferSize(int nBytes)
{
	this->SetDataBufferSize(nBytes);
}

bool CFTPClient::DirectoryList(LPCTSTR lpszFTPDirectory, CString & strOut)
{
	if(NULL == lpszFTPDirectory) 
	{return "";}

	CSocket sockListen;
	CSocket sockData;
	UINT nDataPort;
	CString strCmd;

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CString strFTPDir  = "";
		URLEncode(lpszFTPDirectory, strFTPDir);

		if(!sockData.Create())
		{
			SaveLog("Failed to create data proxy!");
			return false;
		}
		if(!sockData.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return false;
		}

		if(!this->SetSocketBufferSize(&sockData, this->GetDataBufferSize(), 8688))
		{
			sockData.Close();
			return false;
		}

		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("GET ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n"
								//"Cache-Control:  max-age=0\r\n"
								"User-Agent: Mozilla/4.0 (compatible; FTP; Windows XP)\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPDir, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization);
		}
		else
		{
			strCmd.Format("GET ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"User-Agent: Mozilla/4.0 (compatible; FTP; Windows XP)\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPDir, 
								this->m_strFTPServer, this->m_nFTPPort);
		}

		if(!this->SendRawData(&sockData, strCmd))
		{
			sockData.Close();
			return false;
		}
		Sleep(100);
		if(!this->ReceiveRawData(&sockData, this->m_strServerReply))
		{
			sockData.Close();
			return false;
		}
		sockData.Close();

		strOut= this->m_strServerReply;
		if("HTTP/1.0 200" != strOut.Left(12) && "HTTP/1.1 200" != strOut.Left(12))
		{return false;}

		//end of directory list through http get proxy server
	}
	else
	{
		if(!this->SendSimpleData("TYPE I\r\n"))
		{return false;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return false;}
		if("200" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to set transfer type!");
			return false;
		}
		
		if(this->m_pProfile->bSupportCompress)
		{
		}

		if(this->m_pProfile->bSupportMLSD) 
		{strCmd.Format("MLSD %s\r\n", lpszFTPDirectory);}
		else 
		{strCmd.Format("LIST %s\r\n", lpszFTPDirectory);}

		if(this->m_bPassiveMode)
		{	
			//Passive mode
			if(!this->GetDataPort(nDataPort))
			{
				SaveLog("Failed to get data port from server!");
				return false;
			}
			if(!sockData.Create())
			{
				SaveLog("Failed to create data socket under passive mode!");
				return false;
			}
			if(!sockData.Connect(this->m_strFTPServer, nDataPort))
			{
				SaveLog("Failed to connect to server under passive mode!");
				return false;
			}

			//Send directory list command
			if(!this->SendSimpleData(strCmd))
			{return false;}
			if(!this->ReceiveSimpleData(this->m_strServerReply))
			{return false;}

			if("150" != this->m_strServerReply.Left(3))
			{
				if("125" != this->m_strServerReply.Left(3))
				{
					sockData.Close();
					SaveLog(this->m_strServerReply + "\nFailed to send directory list command!");
					return false;
				}
			}
		}
		else
		{
			//Non passive mode
			if(!sockListen.Create(0, SOCK_STREAM, this->GetLocalIPAddress()))
			{
				SaveLog("Failed to create listen socket!");
				return false;
			}
			if(!sockListen.Listen())
			{
				sockListen.Close();
				SaveLog("Failed to listen data socket!");
				return false;
			}

			nDataPort = this->GetSocketPort(&sockListen);
			if(!this->SendDataPort(nDataPort))
			{
				sockListen.Close();
				SaveLog("Failed to send data port to server!");
				return false;
			}

			//Send directory list command
			if(!this->SendSimpleData(strCmd))
			{return false;}
			if(!this->ReceiveSimpleData(this->m_strServerReply))
			{return false;}
			if("150" != this->m_strServerReply.Left(3) && "125" != this->m_strServerReply.Left(3))
			{
				sockListen.Close();
				SaveLog(this->m_strServerReply + "\nFailed to send directory list command!");
				return false;
			}

			if(!sockListen.Accept(sockData))
			{
				sockListen.Close();
				SaveLog("Failed to accept data socket!");
				return false;
			}
		}

		if(!this->SetSocketBufferSize(&sockData, this->GetDataBufferSize(), 8688))
		{
			sockData.Close();
			sockListen.Close();
			return false;
		}
		Sleep(100);

		//recieve data
		if(!this->ReceiveRawData(&sockData, strOut))
		{
			sockData.Close();
			sockListen.Close();
			return false;
		}

		sockData.Close();
		sockListen.Close();

		if(!this->ReceiveRawData(this->m_strServerReply))
		{return false;}

		if("425" == this->m_strServerReply.Left(3))
		{
			SaveLog("Failed to set up connection!");
			return false;
		}
		else if("226" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply);
			return false;
		}
	}

	return true;
}

bool CFTPClient::DirectoryList(LPCTSTR lpszFTPDirectory, CStringArray & strOutNameArray, CStringArray & strOutSizeArray, CStringArray & strOutMdTimeArray)
{
	if(NULL == lpszFTPDirectory) return false;

	CString strOut = "";
	CString strLine = "";
	CString strName = "";
	CString strSize = "";
	CString strMdTime = "";
	CString strTmp = "";
	
	COleVariant vTime;
	COleDateTime oleTime;
	
	if(!this->DirectoryList(lpszFTPDirectory, strOut))
	{return false;	}

	int nLocation = strOut.Find('\n');

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		int nStartFlag = -1;
		int nEndFlag = -1;
		while(nLocation > 0)
		{
			strLine = strOut.Left(nLocation);
			strOut = strOut.Mid(nLocation + 1);
			if("<A HREF=\"" != strLine.Left((int)strlen("<A HREF=\"")) && "<a href=\"" != strLine.Left((int)strlen("<A HREF=\""))) 
			{
				nLocation = strOut.Find('\n');
				continue;
			}
			
			while(true)
			{
				nStartFlag = strLine.Find("<A HREF=\"");
				if(-1 == nStartFlag) nStartFlag = strLine.Find("<a href=\"");
				if(-1 == nStartFlag) break;		
				strLine = strLine.Mid(nStartFlag);

				nEndFlag = strLine.Find("\">");
				if(-1 == nEndFlag) break;
				if('<' == strLine.Mid(nEndFlag + (int)strlen("\">"), 1)) 
				{
					strLine = strLine.Mid(nEndFlag + (int)strlen("\">"));
					continue;//ensure is text link
				}

				nStartFlag = strLine.Find("<A HREF=\"");
				if(-1 == nStartFlag) nStartFlag = strLine.Find("<a href=\"");
				strName = strLine.Mid(nStartFlag + (int)strlen("<A HREF=\""), nEndFlag - (nStartFlag + (int)strlen("<A HREF=\"")));
				if(strName.Right(1) == '/') strName = strName.Left(strName.GetLength() - 1);
				//strName = strName.Mid((int)strName.ReverseFind('/') + 1);
				URLDecode(strName.Mid((int)strName.ReverseFind('/') + 1), strName);

				strLine = strLine.Mid(nEndFlag + (int)strlen("\">"));
				nLocation = strLine.Find("</A>");
				if(-1 == nLocation) nLocation = strLine.Find("</a>");
				if(-1 == nLocation) break;

				strLine = strLine.Mid(nLocation + (int)strlen("</A>"));
				if(-1 != strLine.Find("<")) strTmp = strLine.Left(strLine.Find("<"));
				else strTmp = strLine;

				if(12 > strTmp.GetLength()) continue;
				strTmp.Replace(".", "");
				strTmp.Trim();

				int nTmpLen = strTmp.GetLength();
				int nStrIndex = strTmp.Find("AM");
				if(-1 == nStrIndex) nStrIndex = strTmp.Find("am");
				if(-1 == nStrIndex) nStrIndex = strTmp.Find("PM");
				if(-1 == nStrIndex) nStrIndex = strTmp.Find("pm");
				if(-1 == nStrIndex)
				{
					char nFirstChar =  strTmp.GetAt(0);
					//memcpy_s(&nFirstChar, 1, strTmp.Left(1), 1);
					if(nFirstChar >= 48 && nFirstChar <= 57)
					{
						//2011-05-01 12:00 or 11-05-01 12:00
						if(16 >= nTmpLen) 
						{
							vTime = strTmp;
							strSize = "DIR";
						}
						else
						{
							vTime = strTmp.Left(12);

							strSize = strTmp.Mid(12);
							strSize.Trim();
						}
					}
					else if((nFirstChar >= 65 && nFirstChar <= 90) || (nFirstChar >= 97 && nFirstChar <= 122))
					{
						//May 24 10:13   1270K / Aug 13  2010
						if(12 >= nTmpLen) 
						{
							vTime = strTmp;
							strSize = "DIR";
						}
						else
						{
							vTime = strTmp.Left(12);

							strSize = strTmp.Mid(12);
							strSize.Trim();
						}
					}
				}
				else
				{
					//01-25-11 12:36AM   1270K
					if(nTmpLen == (nStrIndex + 2))
					{
						vTime = strTmp;
						
						strSize = "DIR";
					}
					else
					{
						vTime = strTmp.Left(nStrIndex + 2);
	
						strSize = strTmp.Mid(nStrIndex + 2);
						strSize.Trim();
					}
				}

				try
				{
					vTime.ChangeType(VT_DATE);
					oleTime = vTime;
					strMdTime = oleTime.Format("%Y-%m-%d %H:%M:%S");
				}
				catch(CException * e)
				{
					e->ReportError();
					e->Delete();
					return false;
				}

				strOutNameArray.Add(strName);
				strOutSizeArray.Add(strSize);
				strOutMdTimeArray.Add(strMdTime);
			}

			nLocation = strOut.Find('\n');
		}
	}
	else
	{
		int nIndex = -1;
		bool bDir = false;
		while(nLocation > 0)
		{	
			strLine = strOut.Left(nLocation);

			if(this->m_pProfile->bSupportMLSD) //use MLSD command
			{
				strName = strLine.Mid(strLine.ReverseFind(';') + 1);
				strName.Trim();

				nIndex = strLine.Find(';');
				if("DIR" == strLine.Left(nIndex).Right(3).MakeUpper()) 
				{strSize = "DIR";}
				else
				{
					strLine = strLine.Mid(nIndex + 1);
					nIndex = strLine.Find(';');
					strSize = strLine.Left(nIndex);
					strSize = strSize.Mid(strSize.Find('=') + 1);
					strSize.Trim();
				}

				strLine = strLine.Mid(nIndex + 1);
				nIndex = strLine.Find(';');
				strMdTime = strLine.Left(nIndex);
				strMdTime = strMdTime.Mid(strMdTime.Find('=') + 1);
				strMdTime.Trim();
				strMdTime.Format("%04d-%02d-%02d %02d:%02d:%02d", atoi(strMdTime.Left(4)), atoi(strMdTime.Mid(4, 2)), atoi(strMdTime.Mid(6, 2)), atoi(strMdTime.Mid(8, 2)) + 8, atoi(strMdTime.Mid(10, 2)), atoi(strMdTime.Mid(12, 2)));
			}
			else
			{
				if(this->m_pProfile->bUnix)
				{
					bDir = ('d' == strLine.Left(1));
					
					//drwxrwxrwx
					strLine = strLine.Mid(15);
					strLine.Trim();
					
					//user and group
					for(int i = 0; i < 2; i++)
					{
						nIndex = strLine.Find(" ");
						strLine = strLine.Mid(nIndex);
						strLine.Trim();
					}

					//size
					if(!bDir) 
					{
						nIndex = strLine.Find(" ");
						strSize = strLine.Left(nIndex);
						strSize.Trim();
					}
					else 
					{
						strSize = "DIR";
					}
					
					nIndex = strLine.Find(" ");
					strLine = strLine.Mid(nIndex);
					strLine.Trim();

					vTime = strLine.Left(12);

					strLine.Trim();
					strName = strLine.Mid(12);;
					strName.Trim();
					
					try
					{
						vTime.ChangeType(VT_DATE);
						oleTime = vTime;
						strMdTime = oleTime.Format("%Y-%m-%d %H:%M:%S");	
					}
					catch(CException * e)
					{
						e->ReportError();
						e->Delete();
						return false;
					}
				}
				else if(this->m_pProfile->bWin)
				{
					if(":" != strLine.Mid(11, 1) && ":" != strLine.Mid(13, 1) && ":" != strLine.Mid(12, 1) && ":" != strLine.Mid(14, 1))
					{
						return false;
					}

					vTime = strLine.Mid(0, 20).Trim(); //file name can't contain :
					try
					{
						vTime.ChangeType(VT_DATE);
						oleTime = vTime;
						strMdTime = oleTime.Format("%Y-%m-%d %H:%M:%S");	
					}
					catch(CException * e)
					{
						e->ReportError();
						e->Delete();
						return false;
					}

					strLine = strLine.Mid(20);
					strLine.Trim();

					bDir = (-1 != strLine.Find("DIR") || -1 != strLine.Find("dir"));
					if(!bDir)
					{
						nIndex = strLine.Find(" ");
						strSize = strLine.Left(nIndex);
						strSize.Trim();
					}
					else
					{
						strSize = "DIR";
					}

					nIndex = strLine.Find(" ");
					strName = strLine.Mid(nIndex);
					strName.Trim();
				}
			}

			strOutNameArray.Add(strName);
			strOutSizeArray.Add(strSize);
			strOutMdTimeArray.Add(strMdTime);

			strOut.Delete(0, nLocation + 1);
			nLocation = strOut.Find("\n");
		}
	}

	return true;
}

bool CFTPClient::DirectoryCreate(LPCTSTR lpszFTPDirectory)
{
	if(PROXY_HTTP11_GET == this->m_nProxyType) 
	{return false;}
	if(NULL == lpszFTPDirectory) 
	{return false;}
	
	CString strCmd = "";
	strCmd.Format("MKD %s\r\n", lpszFTPDirectory);

	if(!this->SendSimpleData(strCmd))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("257" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply + "\nFailed to create directory!");
		return false;
	}

	if(this->m_bRecordLog) 
	{this->SaveLog("Create folder: " + CString(lpszFTPDirectory));}

	return true;
}

bool CFTPClient::DirectoryDelete(LPCTSTR lpszFTPDirectory)
{
	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{return false;}
	if(NULL == lpszFTPDirectory) 
	{return false;}

	CString strCmd = "";

	if(IDYES != AfxMessageBox("Are you sure you want to delete this folder?\n" + CString(lpszFTPDirectory), MB_YESNO))
	{
		return false;
	}

	strCmd.Format("RMD %s\r\n", lpszFTPDirectory);
	if(!this->SendSimpleData(strCmd))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("250" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply + "\nFailed to delete directory!");
		return false;
	}

	if(this->m_bRecordLog)
	{this->SaveLog("Delete folder: " + CString(lpszFTPDirectory));}

	return true;
}

CString CFTPClient::GetWorkingDirectory(void)
{
	if(PROXY_HTTP11_GET != this->m_nProxyType) 
	{
		if(!this->SendSimpleData("PWD \r\n"))
		{return "";}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return "";}
		if("257" != this->m_strServerReply.Left(3))
		{
			if(!this->ReceiveSimpleData(this->m_strServerReply))
			{return "";}
			if("257" != this->m_strServerReply.Left(3))
			{
				SaveLog(this->m_strServerReply + "\nFailed to get current directory!");
				return "";
			}
		}

		int nIndex = this->m_strServerReply.Find("\"", 5);
		if(nIndex >= 5)
		{this->m_strWorkingPath = this->m_strServerReply.Mid(5, nIndex - 5);}
		else
		{this->m_strWorkingPath = this->m_strServerReply.Mid(5);}

		this->m_strWorkingPath.Replace("\"", "");
	}
	return this->m_strWorkingPath;
}

bool CFTPClient::DirectoryOpen(LPCTSTR lpszFTPDirectory)
{
	if(NULL == lpszFTPDirectory)
	{return false;}

	CString strCmd = "";

	if(PROXY_HTTP11_GET != this->m_nProxyType)
	{
		strCmd.Format("CWD %s\r\n", lpszFTPDirectory);
		if(!this->SendSimpleData(strCmd))
		{return false;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return false;}
		if("250" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to change directory!");
			return false;
		}
		this->m_strWorkingPath = this->GetWorkingDirectory();//lpszFTPDirectory;
	}
	else
	{
		this->m_strWorkingPath = lpszFTPDirectory;
	}
	
	if(this->m_bRecordLog) 
	{this->SaveLog("Enter directory: " + m_strWorkingPath);}

	return true;
}

bool CFTPClient::FolderList(LPCTSTR lpszFolderName, CString & strOut)
{
	char szDir[MAX_PATH];
	memset(szDir, 0, MAX_PATH);
	
	if("/" == this->m_strWorkingPath.Right(1))	
	{sprintf_s(szDir, sizeof(szDir), "%s%s", this->m_strWorkingPath, lpszFolderName);}
	else
	{sprintf_s(szDir, sizeof(szDir), "%s/%s", this->m_strWorkingPath, lpszFolderName);}
	
	return this->DirectoryList(szDir, strOut);
}

bool CFTPClient::FolderList(LPCTSTR lpszFolderName, CStringArray & strOutNameArray, CStringArray & strOutSizeArray, CStringArray & strOutMdTimeArray)
{
	char szDir[MAX_PATH];
	memset(szDir, 0, MAX_PATH);
	
	if("/" == this->m_strWorkingPath.Right(1))	
	{sprintf_s(szDir, sizeof(szDir), "%s%s", this->m_strWorkingPath, lpszFolderName);}
	else 
	{sprintf_s(szDir, sizeof(szDir), "%s/%s", this->m_strWorkingPath, lpszFolderName);}
	
	return this->DirectoryList(szDir, strOutNameArray, strOutSizeArray, strOutMdTimeArray);
}

bool CFTPClient::FolderOpen(LPCTSTR lpszFolderName)
{
	if(NULL == lpszFolderName) 
	{return false;}
	
	char szDir[MAX_PATH];
	memset(szDir, 0, MAX_PATH);

	if("." == lpszFolderName)
	{lpszFolderName = "";}
	else if(".." == lpszFolderName) 
	{
		this->m_strWorkingPath = this->m_strWorkingPath.Left(this->m_strWorkingPath.ReverseFind('/')+1);
		lpszFolderName = "";
	}

	if("" == lpszFolderName)
	{
		if("/" == this->m_strWorkingPath.Right(1) && "/" != this->m_strWorkingPath) 
		{sprintf_s(szDir, sizeof(szDir), "%s", this->m_strWorkingPath.Left(this->m_strWorkingPath.GetLength()-1));}
		else 
		{sprintf_s(szDir, sizeof(szDir), "%s", this->m_strWorkingPath);}
	}
	else
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
		{sprintf_s(szDir, sizeof(szDir), "%s%s", this->m_strWorkingPath, lpszFolderName);}
		else 
		{sprintf_s(szDir, sizeof(szDir), "%s/%s", this->m_strWorkingPath, lpszFolderName);}
	}
	
	return this->DirectoryOpen(szDir);
}

bool CFTPClient::FolderCreate(LPCTSTR lpszFolderName)
{
	if(PROXY_HTTP11_GET == this->m_nProxyType) 
	{return false;}
	if(NULL == lpszFolderName) 
	{return false;}

	char szDir[MAX_PATH];
	memset(szDir, 0, MAX_PATH);
	if("/" == this->m_strWorkingPath.Right(1))
	{printf_s(szDir, sizeof(szDir), "%s%s", this->m_strWorkingPath, lpszFolderName);}
	else 
	{sprintf_s(szDir, sizeof(szDir), "%s/%s", this->m_strWorkingPath, lpszFolderName);}
	
	return this->DirectoryCreate(szDir);
}

bool CFTPClient::FolderDelete(LPCTSTR lpszFolderName)
{
	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{return false;}
	if(NULL == lpszFolderName) 
	{return false;}

	char szDir[MAX_PATH];
	memset(szDir, 0, MAX_PATH);
	
	if("/" == this->m_strWorkingPath.Right(1)) 
	{sprintf_s(szDir, sizeof(szDir), "%s%s", this->m_strWorkingPath, lpszFolderName);}
	else 
	{sprintf_s(szDir, sizeof(szDir), "%s/%s", this->m_strWorkingPath, lpszFolderName);}
	
	return this->DirectoryDelete(szDir);
}

unsigned long long CFTPClient::FileDownload(LPCTSTR lpszFTPFile, LPCTSTR lpszLocalStorePath, TRANSFER_TYPE nTranserType)
{
	if(NULL == lpszFTPFile)
	{return 0;}

	CSocket sockListen;
	CSocket sockData;
	UINT nDataPort;
	
	CString strCmd = "";
	CString strPath = lpszLocalStorePath;
	CString strFullFileName = "";
	CString strFileName = "";
	CString strFTPFileName = lpszFTPFile;

	unsigned long long nTotalDownloadByte = 0;
	unsigned long long nFileSize = 0;

	if("/" != strFTPFileName.Left(1)) //relative path
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
		{strFTPFileName.Format("%s%s", this->m_strWorkingPath, lpszFTPFile);}
		else
		{strFTPFileName.Format("%s/%s", this->m_strWorkingPath, lpszFTPFile);}
	}
	else
	{strFTPFileName = lpszFTPFile;}

	if(NULL == lpszLocalStorePath || "" == lpszLocalStorePath)
	{
		strPath = GetLocalPath();	
		lpszLocalStorePath = strPath;
		if("" == strPath) 
		{return 0;	}
	}

	strFileName = strFTPFileName.Mid(strFTPFileName.ReverseFind('/') + 1);
	if("\\" == strPath.Right(1) || "/" == strPath.Right(1))
	{strFullFileName.Format("%s%s", lpszLocalStorePath, strFileName);}
	else if(-1 != strPath.Find('\\'))
	{strFullFileName.Format("%s\\%s", lpszLocalStorePath, strFileName);}
	else if(-1 != strPath.Find('/'))
	{strFullFileName.Format("%s/%s", lpszLocalStorePath, strFileName);}
	else
	{strFullFileName.Format("%s\\%s", lpszLocalStorePath, strFileName);}

	nFileSize = this->FileSize(strFTPFileName);
	if(0 == nFileSize)
	{
		SaveLog("FTP file size is 0!");
		return 0;
	}
	
	if(this->m_bRecordLog)
	{this->SaveLog("Download " + strFTPFileName);}

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CFile fileDownload;
		char * pDataBuffer = NULL;
		CString strTmp;
		int nIndex = -1;
		int nReadBytes = 0;

		URLEncode(strFTPFileName, strTmp);
		strFTPFileName = strTmp;

		if(!sockData.Create())
		{
			SaveLog("Failed to create data proxy!");
			return 0;
		}
		if(!sockData.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return 0;
		}

		if(!this->SetSocketBufferSize(&sockData, 8688, this->GetDataBufferSize()))
		{
			sockData.Close();
			return 0;
		}

		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("GET ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n\r\n", 
								//"Cache-Control:  max-age=0\r\n\r\n",
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFileName, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization);
		}
		else
		{
			strCmd.Format("GET ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n\r\n", 
								//"Cache-Control:  max-age=0\r\n\r\n",
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFileName, 
								this->m_strFTPServer, this->m_nFTPPort);
		}

		//Send download command
		if(!this->SendRawData(&sockData, strCmd)) 
		{
			sockData.Close();
			return 0;
		}
		Sleep(100);

		//Receive data
		pDataBuffer = new char[this->GetDataBufferSize() + 1];

		while(true)
		{
			memset(pDataBuffer, 0, this->GetDataBufferSize() + 1);
			if(!this->ReceivedRawData(&sockData, pDataBuffer, this->GetDataBufferSize(), nReadBytes))
			{
				sockData.Close();
				delete[] pDataBuffer;
				return 0;
			}

			this->m_strServerReply = pDataBuffer;
			nIndex = m_strServerReply.Find("\r\n\r\n");
			if(nIndex >= 0)
			{break;}

			//Can't find http header flag?
			nTotalDownloadByte += nReadBytes;
			if(nTotalDownloadByte >= nFileSize)
			{
				sockData.Close();
				delete[] pDataBuffer;
				return 0;
			}
		}		

		if(!fileDownload.Open(strFullFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary))
		{
			SaveLog("Failed to open file to write!");
			sockData.Close();
			return 0;
		}
		fileDownload.Write(pDataBuffer + nIndex + 4, nReadBytes - nIndex - 4);
		fileDownload.Close();
		delete[] pDataBuffer;

		nTotalDownloadByte = (nReadBytes - nIndex - 4);
		if(nTotalDownloadByte < nFileSize)
		{nTotalDownloadByte += this->ReceiveFileData(&sockData, strFullFileName.GetString(), nFileSize - nTotalDownloadByte, false);}
		
		sockData.Close();
		//end of download file through proxy server
	}
	else
	{
		//Set transfer mode
		if(TT_ASCII == nTranserType)
		{strCmd = "TYPE A\r\n";}
		else if(TT_BINARY == nTranserType)
		{strCmd = "TYPE I\r\n";	}
		else 
		{
			SaveLog("Unknown transfer mode!");
			return 0;
		}
		if(!this->SendSimpleData(strCmd))
		{return 0;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return 0;}

		if("200" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to set transfer type!");
			return 0;
		}

		strCmd.Format("RETR %s\r\n", strFTPFileName);
		if(this->m_bPassiveMode)
		{
			//Passive mode
			if(!this->GetDataPort(nDataPort)) 
			{
				SaveLog("Failed to get data port from server!");
				return 0;
			}
			if(!sockData.Create())
			{
				SaveLog("Failed to create data socket under passive mode!");
				return 0;
			}
			if(!sockData.Connect(this->m_strFTPServer, nDataPort))
			{
				SaveLog("Failed to connect to server under passive mode!");
				return 0;
			}

			if(!this->SendSimpleData(strCmd))
			{return 0;}
			if(!this->ReceiveRawData(this->m_strServerReply))
			{return 0;}
			if("150" != this->m_strServerReply.Left(3) && "125" != this->m_strServerReply.Left(3))
			{
				sockListen.Close();
				SaveLog(this->m_strServerReply + "\nFailed to send RETR command!");
				return 0;
			}
		}
		else
		{
			//Non passive mode
			if(!sockListen.Create(0, SOCK_STREAM))
			{
				SaveLog("Failed to create listen socket!");
				return 0;
			}
			if(!sockListen.Listen())
			{
				sockListen.Close();
				SaveLog("Failed to listen data socket!");
				return 0;
			}

			nDataPort = this->GetSocketPort(&sockListen);
			if(!this->SendDataPort(nDataPort))
			{
				sockListen.Close();
				SaveLog("Failed to send port to server!");
				return 0;
			}

			//Send download command
			if(!this->SendSimpleData(strCmd))
			{return 0;}
			if(!this->ReceiveRawData(this->m_strServerReply))
			{return 0;}
			if("150" != this->m_strServerReply.Left(3))
			{
				sockListen.Close();
				SaveLog(this->m_strServerReply + "\nFailed to send RETR command!");
				return 0;
			}

			if(!sockListen.Accept(sockData))
			{
				sockListen.Close();
				SaveLog("Failed to accept data socket!");
				return 0;
			}
		}

		if(!this->SetSocketBufferSize(&sockData, 8688, this->GetDataBufferSize()))
		{
			sockListen.Close();
			sockData.Close();
			return 0;
		}
		Sleep(100);
		
		//recieve data
		nTotalDownloadByte = this->ReceiveFileData(&sockData, strFullFileName.GetString(), nFileSize);
		sockData.Close();
		sockListen.Close();
		
		//this->ReceiveRawData(this->m_strServerReply);
		//if("425" == this->m_strServerReply.Left(3))
		//{SaveLog("Failed to set up connection!");}
		//else if("226" != this->m_strServerReply.Left(3))
		//{SaveLog(this->m_strServerReply);}
		//else
		//{/*SaveLog("Download file successfully!");*/}
	}

	return nTotalDownloadByte;
}

unsigned long long CFTPClient::FileUpload(LPCTSTR lpszFTPPath, LPCTSTR lpszLocalFile, TRANSFER_TYPE nTranserType)
{
	if(NULL == lpszFTPPath) 
	{return 0;}

	CSocket sockListen;
	CSocket sockData;
	UINT nDataPort;
	unsigned long long nTotalUploadByte = 0;
	CString strCmd = "";
	CString strLocalFile = lpszLocalFile;
	CString strFTPFile = "";
	int nLocation  = -1;
	
	if(NULL == lpszLocalFile || "" == lpszLocalFile)
	{
		CFileDialog filedlg(true);
		filedlg.m_ofn.lpstrTitle = "Please choose a file to upload";
		filedlg.m_ofn.lpstrDefExt = "*.*";
		filedlg.m_ofn.lpstrFilter = "All Files\0*.*\0\0";
		if(IDOK == filedlg.DoModal())
		{
			strLocalFile = filedlg.GetPathName();
			if("" == strLocalFile)
			{return 0;}
		}
		else
		{return 0;}
	}

	nLocation = strLocalFile.ReverseFind('\\');
	if(-1 == nLocation) 
	{nLocation = strLocalFile.ReverseFind('/');}
	if("/" != CString(lpszFTPPath).Left(1)) //relative path
	{
		if("/" == CString(lpszFTPPath).Right(1))
		{
			if("/" == this->m_strWorkingPath.Right(1))
				strFTPFile.Format("%s%s%s", this->m_strWorkingPath, lpszFTPPath, strLocalFile.Mid(nLocation + 1));
			else
				strFTPFile.Format("%s/%s%s", this->m_strWorkingPath, lpszFTPPath, strLocalFile.Mid(nLocation + 1));
		}
		else
		{
			if("/" == this->m_strWorkingPath.Right(1))
				strFTPFile.Format("%s%s/%s", this->m_strWorkingPath, lpszFTPPath, strLocalFile.Mid(nLocation + 1));
			else
				strFTPFile.Format("%s/%s/%s", this->m_strWorkingPath, lpszFTPPath, strLocalFile.Mid(nLocation + 1));
		}
	}
	else
	{
		if("/" == CString(lpszFTPPath).Right(1))
			strFTPFile.Format("%s%s", lpszFTPPath, strLocalFile.Mid(nLocation + 1));
		else
			strFTPFile.Format("%s/%s", lpszFTPPath, strLocalFile.Mid(nLocation + 1));
	}

	if(this->m_bRecordLog) 
	{this->SaveLog("Upload " + strLocalFile);}

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CString strTmp;
		CFile fileUpload(strLocalFile, CFile::modeRead | CFile::shareDenyNone);
		unsigned long long nFileSize = fileUpload.GetLength();
		fileUpload.Close();

		URLEncode(strFTPFile, strTmp);
		strFTPFile = strTmp;
		
		if(!sockData.Create())
		{
			SaveLog("Failed to create data proxy!");
			return 0;
		}
		if(!sockData.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return 0;
		}
		
		if(!this->SetSocketBufferSize(&sockData, this->GetDataBufferSize(), 8688))
		{
			sockData.Close();
			return 0;
		}
		
		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("PUT ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n"
								"Content-Length: %d\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization,
								nFileSize);
		}
		else
		{
			strCmd.Format("PUT ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Content-Length: %d\r\n"
								"Connection: close\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort, 
								nFileSize);
		}

		//Send upload command
		if(!this->SendRawData(&sockData, strCmd))
		{
			sockData.Close();
			fileUpload.Close();
			return 0;
		}

		//Send file data
		nTotalUploadByte = this->SendFileData(&sockData, strLocalFile.GetString());

		this->ReceiveRawData(&sockData, this->m_strServerReply);
		sockData.Close();

		if(("HTTP/1.0 202" != this->m_strServerReply.Left(12) && "HTTP/1.1 202" != this->m_strServerReply.Left(12)) //file already exist on ftp server, acceped
			&& ("HTTP/1.0 201" != this->m_strServerReply.Left(12) && "HTTP/1.1 201" != this->m_strServerReply.Left(12))) //file not exist on ftp server, created
		{
			SaveLog(this->m_strServerReply + "\nFailed to upload file!");
		}
		//end of upload file through http11 get proxy server
	}
	else
	{
		//Set transfer mode
		if(TT_ASCII == nTranserType)
		{strCmd = "TYPE A\r\n";}
		else if(TT_BINARY == nTranserType)
		{strCmd = "TYPE I\r\n";}
		else 
		{
			SaveLog("Unknown transfer mode!");
			return 0;
		}
		if(!this->SendSimpleData(strCmd))
		{return 0;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return 0;}
		if("200" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to set transfer type!");
			return 0;
		}

		strCmd.Format("STOR %s\r\n", strFTPFile); 
		if(this->m_bPassiveMode)
		{
			//Passive mode
			if(!this->GetDataPort(nDataPort)) 
			{
				SaveLog("Failed to get data port from server!");
				return 0;
			}
			if(!sockData.Create())
			{
				SaveLog("Failed to create data socket under passive mode!");
				return 0;
			}
			if(!sockData.Connect(this->m_strFTPServer, nDataPort))
			{
				SaveLog("Failed to connect to server under passive mode!");
				return 0;
			}

			//Send upload command
			if(!this->SendSimpleData(strCmd))
			{return 0;}
			if(!this->ReceiveSimpleData(this->m_strServerReply))
			{return 0;}
			if("150" != this->m_strServerReply.Left(3))
			{
				sockListen.Close();
				SaveLog(this->m_strServerReply + "\nFailed to send RETR command!");
				return 0;
			}
		}
		else
		{
			//Non passive mode
			if(!sockListen.Create(0, SOCK_STREAM))
			{
				SaveLog("Failed to create listen socket!");
				return 0;
			}
			if(!sockListen.Listen())
			{
				sockListen.Close();
				SaveLog("Failed to listen data socket!");
				return 0;
			}

			nDataPort = this->GetSocketPort(&sockListen);
			if(!this->SendDataPort(nDataPort))
			{
				sockListen.Close();
				SaveLog("Failed to send port to server!");
				return 0;
			}

			//Send upload command
			if(!this->SendSimpleData(strCmd))
			{return 0;}
			if(!this->ReceiveSimpleData(this->m_strServerReply))
			{return 0;}
			if("150" != this->m_strServerReply.Left(3))
			{
				sockListen.Close();
				SaveLog(this->m_strServerReply + "\nFailed to send RETR command!");
				return 0;
			}

			if(!sockListen.Accept(sockData))
			{
				sockListen.Close();
				SaveLog("Failed to accept data socket!");
				return 0;
			}
		}

		if(!this->SetSocketBufferSize(&sockData, this->GetDataBufferSize(), 8688))
		{
			sockListen.Close();
			sockData.Close();
			return 0;
		}
		Sleep(300);

		//send data
		nTotalUploadByte = this->SendFileData(&sockData, strLocalFile.GetString());
		sockData.Close(); //close data socket to tell the server transition is finished
		sockListen.Close();

		this->ReceiveRawData(this->m_strServerReply);
		if("425" == this->m_strServerReply.Left(3))
		{SaveLog("Failed to set up connection!");}
		else if("226" != this->m_strServerReply.Left(3))
		{SaveLog(this->m_strServerReply);}
		else
		{/*SaveLog("Upload file successfully!");*/}
	}

	return nTotalUploadByte;
}

unsigned long long CFTPClient::FileSize(LPCTSTR lpszFTPFile)
{
	if(NULL == lpszFTPFile) 
	{return false;}

	CString strCmd = "";
	CString strFTPFile = lpszFTPFile;
	unsigned long long nSize = 0;

	if("/" != strFTPFile.Left(1)) //relative path
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
		{strFTPFile.Format("%s%s", this->m_strWorkingPath, lpszFTPFile);}
		else 
		{strFTPFile.Format("%s/%s", this->m_strWorkingPath, lpszFTPFile);}
	}
	else
	{strFTPFile = lpszFTPFile;}

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CSocket sockProxy;
		CString strTmp;
		URLEncode(strFTPFile, strTmp);
		strFTPFile = strTmp;

		if(!sockProxy.Create())
		{
			SaveLog("Failed to create proxy socket!");
			return 0;
		}
		if(!sockProxy.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return 0;
		}

		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("HEAD ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization);
		}
		else
		{
			strCmd.Format("HEAD ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort);
		}

		if(!this->SendRawData(&sockProxy, strCmd))
		{
			sockProxy.Close();
			return 0;
		}
		if(!this->ReceiveRawData(&sockProxy, this->m_strServerReply))
		{
			sockProxy.Close();
			return 0;
		}
		sockProxy.Close();

		if((-1 != this->m_strServerReply.Find("HTTP/1.0 200")) || (-1 != this->m_strServerReply.Find("HTTP/1.1 200")))
		{
			CString strTmp = "";
			const char * pszFlag = "Content-Length:";
			int nLocation = this->m_strServerReply.Find(pszFlag);
			if(-1 != nLocation)
			{
				strTmp = this->m_strServerReply.Mid(nLocation + (int)strlen(pszFlag), this->m_strServerReply.Find("\r\n", nLocation) - nLocation - (int)strlen(pszFlag));
				strTmp.Trim();
				nSize = atoi(strTmp);
			}
		}
		else SaveLog(this->m_strServerReply);
	}
	else
	{
		strCmd.Format("SIZE %s\r\n", lpszFTPFile);
		if(!this->SendSimpleData(strCmd))
		{return 0;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return 0;}
		nSize = atoi(this->m_strServerReply.Mid(4));
	}

	return nSize;
}

bool CFTPClient::FileDelete(LPCTSTR lpszFTPFile)
{
	if(NULL == lpszFTPFile)
	{return false;}

	CString strCmd = "";
	CString strFTPFile = lpszFTPFile;
	CSocket sockProxy;

	if("/" != strFTPFile.Left(1)) //relative path
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
			strFTPFile.Format("%s%s", this->m_strWorkingPath, lpszFTPFile);
		else
			strFTPFile.Format("%s/%s", this->m_strWorkingPath, lpszFTPFile);
	}
	else strFTPFile = lpszFTPFile;


	if(IDYES != AfxMessageBox("Are you sure you want to delete this file?\n" + strFTPFile, MB_YESNO))
	{
		return false;
	}

	if(this->m_bRecordLog) 
	{this->SaveLog("Delete " + strFTPFile);}
	
	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CString strTmp;
		URLEncode(strFTPFile, strTmp);
		strFTPFile = strTmp;

		if(!sockProxy.Create())
		{
			SaveLog("Failed to create data proxy!");
			return false;
		}
		if(!sockProxy.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return false;
		}

		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("DELETE ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization);
		}
		else
		{
			strCmd.Format("DELETE ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPFile, 
								this->m_strFTPServer, this->m_nFTPPort);
		}
		
		if(!this->SendRawData(&sockProxy, strCmd))
		{
			sockProxy.Close();
			return false;
		}
		if(!this->ReceiveRawData(&sockProxy, this->m_strServerReply))
		{
			sockProxy.Close();
			return false;
		}
		sockProxy.Close();

		if("HTTP/1.0 200" != this->m_strServerReply.Left(12) && "HTTP/1.1 200" != this->m_strServerReply.Left(12))
		{
			SaveLog(this->m_strServerReply + "\nFailed to delete file!");
			return false;
		}
	}
	else
	{
		strCmd.Format("DELE %s\r\n", strFTPFile);
		if(!this->SendSimpleData(strCmd))
		{return false;}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return false;}
		if("250" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to delete file!");
			return false;
		}
	}
	return true;
}

bool CFTPClient::FileRename(LPCTSTR lpszFTPFile, LPCTSTR lpszNewName)
{
	if(NULL == lpszFTPFile || NULL == lpszNewName || PROXY_HTTP11_GET == this->m_nProxyType) 
	{return false;}

	CString strCmd = "";
	CString strFTPFile = lpszFTPFile;
	CString strNewName = lpszNewName;

	if("/" != strFTPFile.Left(1))
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
			strFTPFile.Format("%s%s", this->m_strWorkingPath, lpszFTPFile);
		else
			strFTPFile.Format("%s/%s", this->m_strWorkingPath, lpszFTPFile);
	}
	else strFTPFile = lpszFTPFile;

	if("/" != strNewName.Left(1))
	{
		if("/" == this->m_strWorkingPath.Right(1)) 
			strNewName.Format("%s%s", this->m_strWorkingPath, lpszNewName);
		else
			strNewName.Format("%s/%s", this->m_strWorkingPath, lpszNewName);
	}
	else strNewName = lpszNewName;

	strCmd.Format("RNFR %s\r\n", strFTPFile);
	if(!this->SendSimpleData(strCmd))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("350" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply + "\nFailed to rename file!");
		return false;
	}

	strCmd.Format("RNTO %s\r\n", strNewName);
	if(!this->SendSimpleData(strCmd))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("250" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply + "\nFailed to rename file!");
		return false;
	}

	this->SaveLog("Rename " + strFTPFile + " to " + strNewName);

	return true;
}

CString CFTPClient::GetModifiedTime(LPCTSTR lpszFTPObject)
{
	if(NULL == lpszFTPObject) 
	{return false;}

	CString strTime = "";
	CString strCmd = "";

	if(PROXY_HTTP11_GET == this->m_nProxyType)
	{
		CSocket sockProxy;
		CString strFTPObj = "";
		URLEncode(lpszFTPObject, strFTPObj);

		if(!sockProxy.Create())
		{
			SaveLog("Failed to create proxy socket!");
			return "";
		}
		if(!sockProxy.Connect(this->m_strProxyServer, this->m_nProxyPort))
		{
			SaveLog("Failed to connect to proxy server!");
			return "";
		}

		if("" != this->m_strProxyAuthorization)
		{
			strCmd.Format("HEAD ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n"
								"Proxy-Authorization: Basic %s\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPObj, 
								this->m_strFTPServer, this->m_nFTPPort, 
								this->m_strProxyAuthorization);
		}
		else
		{
			strCmd.Format("HEAD ftp://%s:%s@%s:%d%s HTTP/1.1\r\n"
								"Host: %s:%d\r\n\r\n", 
								this->m_strFTPUser, this->m_strFTPPassword, this->m_strFTPServer, this->m_nFTPPort, strFTPObj, 
								this->m_strFTPServer, this->m_nFTPPort);
		}
	
		if(!this->SendRawData(&sockProxy, strCmd))
		{
			sockProxy.Close();
			return "";
		}
		if(!this->ReceiveRawData(&sockProxy, this->m_strServerReply))
		{
			sockProxy.Close();
			return "";
		}
		sockProxy.Close();

		if((-1 != this->m_strServerReply.Find("HTTP/1.0 200")) || (-1 != this->m_strServerReply.Find("HTTP/1.1 200")))
		{
			const char * pszFlag = "Last-Modified:";
			int nLocation = this->m_strServerReply.Find(pszFlag);
			if(-1 != nLocation)
			{
				strTime = this->m_strServerReply.Mid(nLocation + (int)strlen(pszFlag), this->m_strServerReply.Find("\r\n", nLocation) - nLocation - (int)strlen(pszFlag));
				strTime.Trim();
			}

			SYSTEMTIME tmSys;
			if(InternetTimeToSystemTime(strTime, &tmSys, 0))
			{
				CTime tmTmp(tmSys);
				CTimeSpan tmSpan(0, 16, 0, 0); //16hours
				tmTmp += tmSpan;
				strTime = tmTmp.Format("%Y-%m-%d %H:%M:%S");
			}
			//strTime.Format("%04d-%02d-%02d %02d:%02d:%02d", tmSys.wYear, tmSys.wMonth, tmSys.wDay, tmSys.wHour, tmSys.wMinute, tmSys.wSecond);
		}
		else 
		{
			SaveLog(this->m_strServerReply);
			strTime = "";
		}
	}
	else
	{
		strCmd.Format("MDTM %s\r\n", lpszFTPObject);
		if(!this->SendSimpleData(strCmd))
		{return "";}
		if(!this->ReceiveSimpleData(this->m_strServerReply))
		{return "";}
		if("213" != this->m_strServerReply.Left(3))
		{
			SaveLog(this->m_strServerReply + "\nFailed to get modified time!");
			return "";
		}
		strTime.Format("%04d-%02d-%02d %02d:%02d:%02d", atoi(this->m_strServerReply.Mid(4, 4)), atoi(this->m_strServerReply.Mid(8, 2)), atoi(this->m_strServerReply.Mid(10, 2)), atoi(this->m_strServerReply.Mid(12, 2)) + 8, atoi(this->m_strServerReply.Mid(14, 2)), atoi(this->m_strServerReply.Mid(16, 2)));
	}
	
	return strTime;
}

bool CFTPClient::SendDataPort(UINT nPort)
{
	CString strTmp = this->GetLocalIPAddress();
	CString strCmd = "";
	CString strSub[4];
	int nLocation = 0;
	
	if(this->m_bPassiveMode) 
	{return false;}

	for(int i = 0; i < 3; i++)
	{
		nLocation = strTmp.Find(".");
		strSub[i] = strTmp.Left(nLocation);
		strTmp.Delete(0, nLocation + 1);
	}
	strSub[3] = strTmp;

	strCmd.Format("PORT %s,%s,%s,%s,%d,%d\r\n", strSub[0], strSub[1], strSub[2], strSub[3], nPort/256, nPort%256);
	if(!this->SendSimpleData(strCmd))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("200" != this->m_strServerReply.Left(3))
	{
		SaveLog(this->m_strServerReply);
		return false;
	}
	
	return true;
}

bool CFTPClient::GetDataPort(UINT & nPort)
{
	CString strTmp = "";
	BYTE nHight = 0;
	BYTE nLow = 0;
	
	if(!this->m_bPassiveMode) 
	{return false;}

	if(!this->SendSimpleData("PASV \r\n"))
	{return false;}
	if(!this->ReceiveSimpleData(this->m_strServerReply))
	{return false;}
	if("227" != this->m_strServerReply.Left(3)) 
	{
		SaveLog(this->m_strServerReply + "\nFailed to enter passive mode!");
		nPort = 0;
		return false;
	}
	
	strTmp = this->m_strServerReply;
	for(int i = 0; i < 4; i++)
	{
		strTmp.Delete(0, strTmp.Find(',') + 1);
	}

	nHight = atoi(strTmp.Left(strTmp.Find(',')));
	strTmp.Delete(0, strTmp.Find(',') + 1);
	nLow = atoi(strTmp.Left(strTmp.GetLength() -1));
	nPort = nHight * 256 + nLow;
	return true;
}

void CFTPClient::CheckFTPProfile(void)
{
	if(!this->SendSimpleData("HELP \r\n"))
	{return;}
	Sleep(300);
	if(!this->ReceiveRawData(this->m_strServerReply))
	{return;}

	if(-1 != this->m_strServerReply.Find("MLSD"))
	{this->m_pProfile->bSupportMLSD = true;}
	else 
	{this->m_pProfile->bSupportMLSD = false;}

	if(-1 != this->m_strServerReply.Find("MODE Z")) 
	{m_pProfile->bSupportCompress = true;}
	else 
	{m_pProfile->bSupportCompress = false;}

	if(!this->SendSimpleData("SYST \r\n"))
	{return;}
	if(!this->ReceiveRawData(this->m_strServerReply))
	{return;}

	this->m_strServerReply.MakeLower();
	if(-1 != this->m_strServerReply.Find("unix"))
	{m_pProfile->bUnix = true;}
	if(-1 != this->m_strServerReply.Find("windows")) 
	{m_pProfile->bWin = true;}
}

bool CFTPClient::SaveLog(LPCTSTR lpszLog)
{
	ASSERT(NULL != lpszLog);

	WIN32_FIND_DATA   wfd;
	CFile fileLog;
	CTime tmNow;
	CString strName = "";
	CString strMsg = "";
	CString strAppPath = "";
	HANDLE hFind = NULL;

	::GetModuleFileName(NULL, strAppPath.GetBuffer(MAX_PATH), MAX_PATH);
	strAppPath.ReleaseBuffer(MAX_PATH);
	strAppPath = strAppPath.Left(strAppPath.ReverseFind('\\'));

	hFind = FindFirstFile(strAppPath + "\\Log", &wfd);

	if((hFind == INVALID_HANDLE_VALUE) || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//Log directory is not exist
		SECURITY_ATTRIBUTES attrib;
		attrib.bInheritHandle = FALSE;
		attrib.lpSecurityDescriptor = NULL;
		attrib.nLength =sizeof(SECURITY_ATTRIBUTES);

		if(!CreateDirectory(strAppPath + "\\Log", &attrib))
		{
			SaveLog("Failed to create log directory!");
			return false;
		}
	}
	else
	{
		if(!FindClose(hFind))
		{
			return false;
		}
	}

	tmNow = CTime::GetCurrentTime();
	strName = tmNow.Format("%Y-%m-%d");
	strName = strAppPath + "\\Log\\FTPClient" + strName + ".log";

	return ::SaveLog(strName, lpszLog);
}

void CFTPClient::About(void)
{
	CString strMsg;
	strMsg.Format("FTP Client Version %s\nCopyright (C) 2010-2012, Foxconn, All rights reserved\nATP SW Group", Version());
	SaveLog(strMsg);
}

CString CFTPClient::Version(void)
{
	return "1.1.1";
}