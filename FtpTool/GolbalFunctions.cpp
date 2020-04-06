/********************************
 Scope:		Golbal function
 Author:	Walnut
 Updated:	2012-08-05
********************************/

#include "stdafx.h"
#include "globalfunctions.h"
#include "ftpclient.h"	
#include <math.h>

//void Base64Encode(const char *pszInString, unsigned long nInLen, char *pszOutString)
//{
//	VERIFY(NULL != pszInString && NULL != pszOutString);
//	const unsigned char * pszIn = (unsigned char *)pszInString;
//	unsigned char * pszOut = (unsigned char *)pszOutString;
//	const char base64digits[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//
//	for (; nInLen >= 3; nInLen -= 3)
//	{
//		*pszOut++ = base64digits[pszIn[0] >> 2];
//		*pszOut++ = base64digits[((pszIn[0] << 4) & 0x30) | (pszIn[1] >> 4)];
//		*pszOut++ = base64digits[((pszIn[1] << 2) & 0x3c) | (pszIn[2] >> 6)];
//		*pszOut++ = base64digits[pszIn[2] & 0x3f];
//		pszIn += 3;
//	}
//	if (nInLen > 0)
//	{
//		unsigned char fragment;
//		*pszOut++ = base64digits[pszIn[0] >> 2];
//		fragment = (pszIn[0] << 4) & 0x30;
//
//		if (nInLen > 1) fragment |= pszIn[1] >> 4;
//
//		*pszOut++ = base64digits[fragment];
//		*pszOut++ = (nInLen < 2) ? '=' : base64digits[(pszIn[1] << 2) & 0x3c];
//		*pszOut++ = '=';
//	}
//	*pszOut = '\0';
//}  
//
//void Base64Decode(const char * pszInString, unsigned long nInLen, char * pszOutString)
//{
//	VERIFY(NULL != pszInString && NULL != pszOutString);
//	const unsigned char * pszIn = (unsigned char *)pszInString;
//	unsigned char * pszOut = (unsigned char *)pszOutString;
//	const char base64digits[] =
//	{
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//		62, // '+'
//		0, 0, 0,
//		63, // '/'
//		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
//		0, 0, 0, 0, 0, 0, 0,
//		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
//		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
//		0, 0, 0, 0, 0, 0,
//		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
//		39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
//	};
//
//	int nValue;
//	unsigned long nIndex = 0;
//
//	while(nIndex < nInLen)
//	{
//		if ('\r' == *pszIn || '\n' == *pszIn)
//		{
//			pszIn++;
//			nIndex++;
//			continue;
//		}
//
//		nValue = base64digits[*pszIn++] << 18;
//		nValue += base64digits[*pszIn++] << 12;
//		*pszOut++ = (nValue & 0x00FF0000) >> 16;
//
//		if('=' != *pszIn)
//		{
//			nValue += base64digits[*pszIn++] << 6;
//			*pszOut++ = (nValue & 0x0000FF00) >> 8;
//
//			if('=' != *pszIn)
//			{
//				nValue += base64digits[*pszIn++];
//				* pszOut++ = nValue & 0x000000FF;
//			}
//		}
//		nIndex += 4;
//	}
//}

/*
CP_ACP					ANSI code page 
CP_MACCP				Macintosh code page 
CP_OEMCP				OEM code page 
CP_SYMBOL			Windows 2000/XP: Symbol code page (42) 
CP_THREAD_ACP	Windows 2000/XP: Current thread's ANSI code page 
CP_UTF7				Windows 98/Me, Windows NT 4.0 and later: Translate using UTF-7. When this is set, lpDefaultChar and lpUsedDefaultChar must be NULL 
CP_UTF8				Windows 98/Me, Windows NT 4.0 and later: Translate using UTF-8. When this is set, dwFlags must be zero and both lpDefaultChar and lpUsedDefaultChar must be NULL. 
*/
void UTF8GB2312Convert(CString & strString, int nSourceCodePage, int nTargetCodePage)
{	
	wchar_t * pUnicode = NULL;
	BYTE * pTargetData = NULL;
	
	int nStrLen = strString.GetLength();
	int nUnicodeLen = MultiByteToWideChar(nSourceCodePage, 0, strString, -1, NULL, 0);
	int nTargetLen = 0;

	pUnicode = new wchar_t[nUnicodeLen];
	memset(pUnicode, 0, (nUnicodeLen)*sizeof(wchar_t));

	MultiByteToWideChar(nSourceCodePage, 0, strString, -1, (LPWSTR)pUnicode, nUnicodeLen);

	nTargetLen = WideCharToMultiByte(nTargetCodePage, 0, (LPWSTR)pUnicode, -1, NULL, 0, NULL, NULL);
	pTargetData = new BYTE[nTargetLen];
	memset(pTargetData,0,nTargetLen);

	WideCharToMultiByte(nTargetCodePage, 0, (LPWSTR)pUnicode, -1, (char *)pTargetData, nTargetLen, NULL, NULL);

	strString.Format("%s", pTargetData);

	delete[] pUnicode;
	delete[] pTargetData;
}

void UnicodeToUTF8(const wchar_t * pUnicode, unsigned long * pUTF8)
{
	//Chinese character unicode range: 0x4E00--0x9FA5
	//So the Chinese character UTF8 range is: 0xE4B880--0xE9BEA5
	unsigned char utf8[3];
	unsigned long nTmp = 0;

	memset(utf8, 0, 3);
	utf8[0] = 0xE0 | (*pUnicode >> 12);
	utf8[1] = 0x80 | ((*pUnicode >> 6) & 0x3F);
	utf8[2] = 0x80 | (*pUnicode & 0x3F);
	
	*pUTF8 = (utf8[0] << 16) + (utf8[1] << 8) + utf8[2];
}

void UTF8ToUnicode(const unsigned long * pUTF8, wchar_t * pUnicode)
{
	unsigned char utf8[3];
	memset(utf8, 0, 3);
	utf8[0] = unsigned char((*pUTF8 >> 16) & 0xFF);
	utf8[1] = unsigned char((*pUTF8 >> 8) & 0xFF);// / 256 % 256);
	utf8[2] = unsigned char(*pUTF8 & 0xFF);

    *pUnicode = (utf8[0] & 0x1F) << 12;
    *pUnicode |= (utf8[1] & 0x3F) << 6;
    *pUnicode |= (utf8[2] & 0x3F);
}

void URLEncode(const CString & strInString, CString & strURLString)
{
	wchar_t nTmp = 0;
	unsigned long nUTF8;
	CString strTmp = "";
	CStringW strwInStr = CA2W(strInString); //for chinese char

	strURLString = "";
	for(int nIndex = 0; nIndex < strwInStr.GetLength(); nIndex ++)
	{
		nTmp = strwInStr.GetAt(nIndex);
		if(nTmp <= 47 || (nTmp >= 58 && nTmp <= 64) || (nTmp >= 91 && nTmp <= 96) || nTmp >= 123)
		{
			if(nTmp > 255) 
			{
				UnicodeToUTF8(&nTmp, &nUTF8);
				strTmp.Format("%%%2x%%%2x%%%2x", unsigned char((nUTF8 >> 16) & 0xFF), unsigned char((nUTF8 >> 8) & 0xFF), unsigned char(nUTF8 & 0xFF));
			}
			else if('/' == nTmp || '!' == nTmp || '_' == nTmp || '+' == nTmp || '-' == nTmp || '.' == nTmp || ',' == nTmp || '$' == nTmp || '(' == nTmp || ')' == nTmp)
			{
				strTmp = nTmp;
			}
			else
			{
				strTmp.Format("%%%2x", nTmp);
			}

			strTmp.MakeUpper();
		}
		else
		{
			strTmp = nTmp;
		}
		
		strURLString.Append(strTmp);
	}
}

void URLDecode(const CString & strURLString, CString & strOutString)
{
	unsigned char nTmp[3];
	unsigned long nValue = 0;
	CStringW strwOutStr = L"";
	int nStrLen = strURLString.GetLength();

	strOutString = L"";
	for(int nIndex = 0; nIndex < nStrLen; nIndex++)
	{
		nTmp[0] = strURLString.GetAt(nIndex);
		if(nStrLen - nIndex >= 1) nTmp[1] = strURLString.GetAt(nIndex + 1);
		if(nStrLen - nIndex >= 2) nTmp[2] = strURLString.GetAt(nIndex + 2);
		
		if('%' == nTmp[0] && ((nTmp[1] >= '0' && nTmp[1] <= '9')  || (nTmp[1] >= 'a' && nTmp[1] <= 'f') || (nTmp[1] >= 'A' || nTmp[1] <='F'))
			&& ((nTmp[2] >= '0' && nTmp[2] <= '9')  || (nTmp[2] >= 'a' && nTmp[2] <= 'f') || (nTmp[2] >= 'A' || nTmp[2] <='F')))
		{
			nValue = strtoul(strURLString.Mid(nIndex + 1, 2), NULL, 16);
			
			if(nValue >= 0xE4 && nValue <= 0xE9) //chinese char, need read 2 bytes
			{
				unsigned char nChar[2][3];
				bool bOk[2] = {false, false};

				for(int i = 0; i < 2; i++)
				{
					nChar[i][0] = strURLString.GetAt(nIndex + i*3 + 3);
					nChar[i][1] = strURLString.GetAt(nIndex + i*3 + 4);
					nChar[i][2] = strURLString.GetAt(nIndex + i*3 + 5);

					if('%' == nChar[i][0] && ((nChar[i][1] >= '0' && nChar[i][1] <= '9')  || (nChar[i][1] >= 'a' && nChar[i][1] <= 'f') || (nChar[i][1] >= 'A' || nChar[i][1] <='F'))
						&& ((nChar[i][2] >= '0' && nChar[i][2] <= '9')  || (nChar[i][2] >= 'a' && nChar[i][2] <= 'f') || (nChar[i][2] >= 'A' || nChar[i][2] <='F')))
					{
						bOk[i] = true;
					}
					else 
					{
						bOk[i] = false;
						break;
					}
				}

				if(bOk[0] && bOk[1])
				{
					unsigned long nUTF8 = nValue;
					wchar_t nUnicode[2] = {0, 0};

					nUTF8 <<= 8;
					nUTF8 += strtoul(strURLString.Mid(nIndex + 4, 2), NULL, 16);
					nUTF8 <<= 8;
					nUTF8 += strtoul(strURLString.Mid(nIndex + 7, 2), NULL, 16);
					
					if(nUTF8 >= 0xE4B880 && nUTF8 <= 0xE9BEA5) //Chinese character UTF8 range is: 0xE4B880--0xE9BEA5
					{
						UTF8ToUnicode(&nUTF8, nUnicode);
						strwOutStr.Append(nUnicode);

						nIndex += 8;
						continue; //next char
					}
				}
			}

			nIndex += 2;
		}
		else 
		{
			nValue = nTmp[0];	
		}
		
		strwOutStr.Append((wchar_t*)&nValue);
	}

	strOutString = CW2A(strwOutStr);
}

void GetHtmlLinkString(const CString & strHtml, CStringArray & strOutArray)
{
	const char * pszStartFlag = "<A HREF=\"";
	const char * pszStartFlag_L = "<a href=\"";
	const char * pszEndFlag = "\">";
	const char * pszEndLink = "</A>";
	const char * pszEndLink_L = "</a>";
	
	int nStrLen = 0;
	int nStartFlag = -1;
	int nEndFlag = -1;
	int nEndLink = -1;
	
	CString strTmp = strHtml;

	while(true)
	{
		nStartFlag = strTmp.Find(pszStartFlag); //find string: <A HREF="
		if(-1 == nStartFlag)
		{
			nStartFlag = strTmp.Find(pszStartFlag_L);
			if(-1 == nStartFlag) break;	
		}

		nStrLen = strTmp.GetLength();
		strTmp = strTmp.Right(nStrLen - (nStartFlag + (int)strlen(pszStartFlag))); //move the all string until end of string <A HREF="

		nEndFlag = strTmp.Find(pszEndFlag); //find string: ">
		if(-1 == nEndFlag) break;

		nStrLen = strTmp.GetLength();
		//strTmp = strTmp.Right(nStrLen - nEndFlag - 1); //move all string until end of ">
		strTmp = strTmp.Right(nStrLen - (nEndFlag + (int)strlen(pszEndFlag)));

		if('<' != strTmp.Left(1)) //ensure it's string link, not picture of something else
		{
			nEndLink = strTmp.Find(pszEndLink); //find string: </A>
			if(-1 == nEndLink) nEndLink = strTmp.Find(pszEndLink_L);

			if(nEndLink >= 0) 
			{
				strOutArray.Add(strTmp.Left(nEndLink));
				nStrLen = strTmp.GetLength();
				strTmp = strTmp.Right(nStrLen - nEndLink); //remove all string before </A>
			}
		}
	}
}

void GetHtmlLinkTarget(const CString & strHtml, CStringArray & strOutArray)
{
	const char * pszStartFlag = "<A HREF=\"";
	const char * pszStartFlag_L = "<a href=\"";
	const char * pszEndFlag = "\">";
	
	int nStrLen = 0;
	int nStartFlag = -1;
	int nEndFlag = -1;

	CString strTmp = strHtml;

	while(true)
	{
		nStartFlag = strTmp.Find(pszStartFlag); //find string: <A HREF="
		if(-1 == nStartFlag) 
		{
			nStartFlag = strTmp.Find(pszStartFlag_L);
			if(-1 == nStartFlag) break;	
		}

		nStrLen = strTmp.GetLength();
		strTmp = strTmp.Right(nStrLen - (nStartFlag + (int)strlen(pszStartFlag))); //move the all string until end of string <A HREF="

		nEndFlag = strTmp.Find(pszEndFlag); //find string: ">
		if(-1 == nEndFlag) break;

		if('<' != strTmp.Mid(nEndFlag + (int)strlen(pszEndFlag), 1)) //ensure it is string link, not picture or something else
		{
			strOutArray.Add(strTmp.Left(nEndFlag));
			CString strdbg = strTmp.Left(nEndFlag);
			nStrLen = strTmp.GetLength();		
			strTmp = strTmp.Right(nStrLen - nEndFlag); //remove all string before ">
		}
	}
}

bool GetAppPath(CString & strAppPath)
{
	if(0 == GetModuleFileName(NULL, strAppPath.GetBuffer(MAX_PATH), MAX_PATH))
	{
		AfxMessageBox("Failed to get application path");
		return false;
	}
	strAppPath.ReleaseBuffer(MAX_PATH);
	strAppPath = strAppPath.Left(strAppPath.ReverseFind('\\'));
	return true;
}

bool SaveLog(LPCTSTR lpszLogFileName, LPCTSTR lpszLog)
{
	ASSERT(NULL != lpszLog);

	CFile fileLog;
	CTime tmNow;
	CString strMsg;

	tmNow = CTime::GetCurrentTime();
	strMsg.Format("%s\t%s\r\n", tmNow.Format("%Y-%m-%d %H:%M:%S"), lpszLog);

	if(!fileLog.Open(lpszLogFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone))
	{
		AfxMessageBox("Failed to open log file to write!");
		return false;
	}

	fileLog.SeekToEnd();
	fileLog.Write(strMsg, strMsg.GetLength());
	fileLog.Close();

	return true;
}

void TarverseLocalDir(CString strDir)
{
	CFileFind fileFind;	
	CString strName;
	BOOL bFind;
	
	if(-1 != strDir.ReverseFind('\\')) 
	{
		if('\\' == strDir.Right(1)) strName.Format("%s*.*", strDir);
		else strName.Format("%s\\*.*", strDir);
	}
	else if(-1 != strDir.ReverseFind('/')) 
	{
		if('/' == strDir.Right(1)) strName.Format("%s*.*", strDir);
		else strName.Format("%s/*.*", strDir);
	}
	else 
	{
		strName.Format("%s\\*.*", strDir);
	}
	
	bFind = fileFind.FindFile(strName);

	while(bFind)
	{
		bFind = fileFind.FindNextFile();
		if(fileFind.IsDots() || fileFind.IsHidden() || fileFind.IsSystem()) continue;
		if(fileFind.IsDirectory())
		{
			strName = fileFind.GetFilePath();
			TRACE(strName);
			TRACE("\n");
			TarverseLocalDir(strName);
		}
		else
		{
			TRACE(fileFind.GetFilePath());
			TRACE("\n");
		}
	}

	if(bFind) fileFind.Close();
}

CString GetLocalPath(void)
{
	CString strLocalPath = "";
	BROWSEINFO bi;
	char szTitle[] = "Please choose the local path";
	char szPath[MAX_PATH];

	memset(szPath, 0, MAX_PATH);
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = szTitle;
	bi.ulFlags = /*BIF_BROWSEINCLUDEFILES|*/BIF_RETURNONLYFSDIRS | BIF_BROWSEFORCOMPUTER | BIF_NEWDIALOGSTYLE;
	ITEMIDLIST * pidl = SHBrowseForFolder(&bi); 
	if(NULL == pidl)
	{
		return "";
	}
	SHGetPathFromIDList(pidl, szPath);
	strLocalPath.Format("%s", szPath);

	return strLocalPath;
}

void TarverseFTPDir(CFTPClient & ftp, CString strDir)
{
	CStringArray strName;
	CStringArray strSize;
	CStringArray strTime;
	CString strFolderName;

	if(!ftp.DirectoryOpen(strDir)) return;
	TRACE(ftp.GetWorkingDirectory());
	TRACE("\n");
	
	if(!ftp.DirectoryList(strDir, strName, strSize, strTime)) return;
	for(int n=0; n < strName.GetCount(); n++)
	{
		if("DIR" == strSize.GetAt(n))
		{
			strFolderName = strName.GetAt(n);
			if(".." == strFolderName || "." == strFolderName) continue;
			//TRACE(ftp.GetWorkingDirectory());
			//TRACE("\n");
			
			if(!ftp.FolderOpen(strFolderName)) return;
			TarverseFTPDir(ftp, ftp.GetWorkingDirectory());
			if(!ftp.FolderOpen("..")) return;
		}
		else
		{
			TRACE(ftp.GetWorkingDirectory() + "/" + strName.GetAt(n));
			TRACE("\n");
		}
	}
}

bool AvailablePrinter(CStringArray & strArray)
{
	DWORD dwBytesNeeded=0, dwReturned=0;
	PRINTER_INFO_1 * pinfo1 = NULL;

	if(!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 1, (LPBYTE)pinfo1, dwBytesNeeded, &dwBytesNeeded, &dwReturned))
	{
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
            if(pinfo1) LocalFree(pinfo1);
            pinfo1 = (PRINTER_INFO_1 *)LocalAlloc(LPTR, dwBytesNeeded);
        }
        else
        {
			AfxMessageBox("Failed enum printers!");
			return false;
		}
	}

	if(!EnumPrinters(PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS, NULL, 1, (LPBYTE)pinfo1, dwBytesNeeded, &dwBytesNeeded, &dwReturned))
	{
		LocalFree(pinfo1);
		return false;
	}

	for(int i = 0; i < (int)dwReturned; i++)
	{
		strArray.Add(pinfo1[i].pName);
		TRACE(pinfo1[i].pName);
		TRACE("\n");
	}

	LocalFree(pinfo1);
	return true;
}

bool PrinterFile(LPCTSTR lpszPrinterName, CString strPrintData, LPCTSTR lpszOutPutFile)
{
	HANDLE hPrinter = NULL;
	DWORD dwJob;
	DWORD dwWritten = 0;
	DOC_INFO_1  doc;

	memset(&doc, 0, sizeof(doc));
	doc.pDocName = "EDFA Print Job"; //document name (which is shown in the PrintManager
	doc.pDatatype = "RAW"; //RAW, EMF, TEXT
	if(lpszOutPutFile) doc.pOutputFile = (LPSTR)lpszOutPutFile;
	
	if(!OpenPrinter((LPSTR)lpszPrinterName, &hPrinter, NULL))
	{
		AfxMessageBox("Failed to open printer!");
		return false;
	}

	dwJob = StartDocPrinter(hPrinter, 1, (LPBYTE)&doc);
	if(0 == dwJob)
	{
		AfxMessageBox("Failed to start a doc printer job!");
		ClosePrinter(hPrinter);
		return false;
	}

	if(!StartPagePrinter(hPrinter)) //start a new page
	{
		AfxMessageBox("Failed to start a page printer!");
		EndDocPrinter(hPrinter);
		ClosePrinter(hPrinter);
		return false;
	}
	if(!WritePrinter(hPrinter, (LPVOID)strPrintData.GetString(), strPrintData.GetLength(), &dwWritten)) //send data
	{
		AfxMessageBox("Failed to write data to printer!");
		EndPagePrinter(hPrinter);
        EndDocPrinter(hPrinter);
		ClosePrinter(hPrinter);
		return false;
	}
	if(!EndPagePrinter(hPrinter))//end of the page print
	{
		AfxMessageBox("Failed to end page printer!");
		EndDocPrinter(hPrinter);
		ClosePrinter(hPrinter);
		return false;
	}
	if(!EndDocPrinter(hPrinter))//doc end
	{
		AfxMessageBox("Failed to end doc printer job!");
		ClosePrinter(hPrinter);
		return false;
	}
	if(!ClosePrinter(hPrinter))
	{
		AfxMessageBox("Failed to close printer!");
		return false;
	}

	return true;
}

void EncryptString(CString strInput, WORD nKey, CString * pstrOutput, const WORD nC1, const WORD nC2)
{
	CString strOutput = "";
	VERIFY(NULL != pstrOutput);
	
	for(int n = 0; n < strInput.GetLength(); n++)
	{
		strOutput.AppendChar(strInput.GetAt(n) ^ (nKey>>8)); //XOR
		nKey = (strOutput.GetAt(n) + nKey) * nC1 + nC2; //Next key
	}

	strInput = strOutput;
	strOutput.Empty();
	for(int n = 0; n < strInput.GetLength(); n++)
	{
		unsigned char nTmp = strInput.GetAt(n); //Keep as 2 ASCII char
		strOutput.AppendChar(33 + nTmp/93); //ACSII char 33 to 126 (! to ~)
		strOutput.AppendChar(33 + nTmp%93); //ACSII char 33 to 126 (! to ~)
	}

	*pstrOutput = strOutput;
}

void DecryptString(CString strInput, WORD nKey, CString * pstrOutput, const WORD nC1, const WORD nC2)
{
	CString strOutput = "";
	VERIFY(NULL != pstrOutput);

	for(int n = 0; n < strInput.GetLength() / 2; n++)
	{
		unsigned char nTmp = ((strInput.GetAt(n*2) - 33) * 93) + (strInput.GetAt(n*2 + 1) - 33);
		strOutput.AppendChar(nTmp);
	}

	strInput = strOutput;
	strOutput.Empty();
	for(int n = 0; n < strInput.GetLength(); n++)
	{
		strOutput.AppendChar(strInput.GetAt(n) ^ (nKey>>8)); //XOR again will revert
		nKey = (strInput.GetAt(n) + nKey) * nC1 + nC2; //Next key
	}

	*pstrOutput = strOutput;
}

bool Delay(double fMicrosencod) //us
{
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nStartTime;
	LARGE_INTEGER nStopTime;
	double fDelay;

	if(FALSE == QueryPerformanceFrequency(&nFreq))//get clock frequency
	{return false;}
	if(FALSE == QueryPerformanceCounter(&nStartTime))//local count
	{return false;}

	while(true)
	{
		if(FALSE == QueryPerformanceCounter(&nStopTime))//local count
		{return false;}

		if(nStopTime.LowPart < nStartTime.LowPart)
		{nStopTime.LowPart += 0xFFFFFFFF;}

		fDelay = (double)(nStopTime.LowPart - nStartTime.LowPart)/(double)nFreq.QuadPart;
		if(fDelay*1E6 >= fMicrosencod)
		{break;}
	}

	return true;
}

bool Linest(const double * pXValues, const double * pYValues, int nSampleNum, double & fSlope, double & fOffset, double & fMeanError)
{
	if(NULL == pYValues || NULL == pXValues || 0 >= nSampleNum) return false;

	double fSumXMultipleY = 0;
	double fSumX = 0, fSumY = 0;
	double fSumXMultipleX = 0;
	for(int i = 0; i < nSampleNum; i ++)
	{
		fSumX += pXValues[i];
		fSumY += pYValues[i];
		fSumXMultipleY += pXValues[i]*pYValues[i];
		fSumXMultipleX += pXValues[i]*pXValues[i];
	}
	
	fSlope = (fSumXMultipleY*nSampleNum - fSumY*fSumX)/(fSumXMultipleX*nSampleNum - fSumX*fSumX);
	fOffset = (fSumXMultipleY*fSumX - fSumY*fSumXMultipleX)/(fSumX*fSumX - fSumXMultipleX*nSampleNum);
	
	fMeanError = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		fMeanError += (pow(pYValues[n]-(fSlope*pXValues[n]+fOffset), 2));
	}
	fMeanError = sqrt(fMeanError/(nSampleNum-1));
	return true;
}

bool Constantest(const double * pXValues, const double * pYValues, int nSampleNum, double & fCoefficient, double & fBase, double & fMeanError)
{
	if(NULL == pYValues || NULL == pXValues || 0 >= nSampleNum) return false;
	
	double * pTmpYValues = new double[nSampleNum];
	double fLna = 0, fLnb = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		pTmpYValues[n] = log(pYValues[n]);
	}
	if(!Linest(pXValues, pTmpYValues, nSampleNum, fLna, fLnb, fMeanError))
	{
		delete[] pTmpYValues;
		return false;
	}

	fBase = exp(fLna);
	fCoefficient = exp(fLnb);

	fMeanError = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		fMeanError += (pow(pYValues[n]-(fCoefficient*pow(fBase, pXValues[n])), 2));
	}
	fMeanError = sqrt(fMeanError/(nSampleNum-1));

	delete[] pTmpYValues;
	return true;
}

bool Expest(const double * pXValues, const double * pYValues, int nSampleNum, double & fCoefficient, double & fSlope, double & fMeanError)
{
	if(NULL == pYValues || NULL == pXValues || 0 >= nSampleNum) return false;
	
	double * pTmpYValues = new double[nSampleNum];
	double fLnb = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		pTmpYValues[n] = log(pYValues[n]);
	}
	if(!Linest(pXValues, pTmpYValues, nSampleNum, fSlope, fLnb, fMeanError))
	{
		delete[] pTmpYValues;
		return false;
	}

	fCoefficient = exp(fLnb);

	fMeanError = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		fMeanError += pow(pYValues[n]-(fCoefficient*exp(fSlope*pXValues[n])), 2);
	}
	fMeanError = sqrt(fMeanError/(nSampleNum-1));

	delete[] pTmpYValues;
	return true;
}

bool Logest(const double * pXValues, const double * pYValues, int nSampleNum, double & fSlope, double & fOffset, double & fMeanError)
{
	if(NULL == pYValues || NULL == pXValues || 0 >= nSampleNum) return false;

	double * pTmpXValues = new double[nSampleNum];
	for(int i = 0; i < nSampleNum; i ++)
	{
		pTmpXValues[i] = log(pXValues[i]);
	}
	if(!Linest(pTmpXValues, pYValues, nSampleNum, fSlope, fOffset, fMeanError))
	{
		delete[] pTmpXValues;
		return false;
	}

	fMeanError = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		fMeanError += pow(pYValues[n]-(fSlope*pTmpXValues[n] + fOffset), 2);
	}
	fMeanError = sqrt(fMeanError/(nSampleNum-1));

	delete[] pTmpXValues;
	return true;
}

bool Multiest(const double * pXValues, const double * pYValues, int nSampleNum, double * pCoefficient, double & fMeanError)
{
	if(NULL == pYValues || NULL == pXValues || 0 >= nSampleNum || NULL == pCoefficient) return false;
	
	double * pDeterminant = new double[nSampleNum*nSampleNum];
	double * pDetResult = new double[nSampleNum+1];
	
	for(int nIndex = 0; nIndex < nSampleNum*nSampleNum; nIndex++)
	{
		pDeterminant[nIndex] = pow(pXValues[nIndex/nSampleNum], nSampleNum-nIndex%nSampleNum-1);
	}
	
	if(!CalculateDeterminant(pDeterminant, nSampleNum, pDetResult[0]))
	{
		delete[] pDeterminant;
		delete[] pDetResult;
		return false;
	}
	for(int nDetIndex = 1; nDetIndex < nSampleNum+1; nDetIndex++)
	{
		for(int nIndex = 0; nIndex < nSampleNum*nSampleNum; nIndex++)
		{
			if((nIndex%nSampleNum)+1 == nDetIndex)
			{
				pDeterminant[nIndex] = pYValues[nIndex/nSampleNum];
			}
			else
			{
				pDeterminant[nIndex] = pow(pXValues[nIndex/nSampleNum], nSampleNum-nIndex%nSampleNum-1);
			}
		}
		if(!CalculateDeterminant(pDeterminant, nSampleNum, pDetResult[nDetIndex]))
		{
			delete[] pDeterminant;
			delete[] pDetResult;
			return false;
		}
		pCoefficient[nDetIndex-1] = pDetResult[nDetIndex]/pDetResult[0];//calculate the value of the cofficient
	}

	fMeanError = 0;
	for(int n = 0; n < nSampleNum; n ++)
	{
		double fCalValue = 0;
		for(int m = 0; m < nSampleNum; m ++)
		{
			fCalValue += (pCoefficient[m]*pow(pXValues[n], nSampleNum-m-1));
		}
		fMeanError += (pow(pYValues[n]-fCalValue, 2));
	}
	fMeanError = sqrt(fMeanError/(nSampleNum-1));

	delete[] pDeterminant;
	delete[] pDetResult;
	return true;
}

bool CalculateDeterminant(const double *pDeterminant, int nPhase, double & fResult)
{
	if(nPhase < 2) 
	{
		return false;
	}
	else if(2 == nPhase)
	{
		fResult = pDeterminant[0]*pDeterminant[3] - pDeterminant[1]*pDeterminant[2];
		return true;
	}
	
	double * pRemainder = new double[(nPhase-1)*(nPhase-1)];
	int nRemainderIndex = 0;
	double fRemainderResult = 0;
	int nExpansionRow = 0;//the expansion row index, the value can be 0 to nPhase-1

	fResult = 0;
	for(int nXIndex = 0; nXIndex < nPhase; nXIndex++)
	{
		nRemainderIndex = 0;
		for(int nIndex = 0; nIndex < nPhase*nPhase; nIndex++)
		{
			if((nExpansionRow != nIndex/nPhase) && (nXIndex != nIndex%nPhase))
			{
				pRemainder[nRemainderIndex++] = pDeterminant[nIndex];//remdainder
			}
		}
	
		if(!CalculateDeterminant(pRemainder, nPhase-1, fRemainderResult))//calculate the value of the remadinder
		{
			delete[] pRemainder;
			return false;
		}
		fRemainderResult *= pow(-1.0f, (nXIndex+1)+(nExpansionRow+1));
		fResult += (pDeterminant[nExpansionRow*nPhase+nXIndex]*fRemainderResult);
	}
	
	delete[] pRemainder;
	return true;
}