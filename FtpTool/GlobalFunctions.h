#pragma once
#include <afx.h>
#include <afxwin.h>		// MFC core and standard components
#include <afxext.h>		// MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <afxcmn.h>		// MFC support for Windows Common Controls
#include <iostream>		// io
#include <afxsock.h>		// socket
#include <winspool.h>	// printer
#include "FTPClient.h"	// ftp

//void Base64Encode(const char *pszInString, unsigned long nInLen, char *pszOutString);
//void Base64Decode(const char * pszInString, unsigned long nInLen, char * pszOutString);

void UTF8GB2312Convert(CString & strString, int nSourceCodePage, int nTargetCodePage);
void UnicodeToUTF8(const wchar_t * pUnicode, unsigned long * pUTF8);
void UTF8ToUnicode(const unsigned long * pUTF8, wchar_t * pUnicode);

void URLEncode(const CString & strString, CString & strURLString);
void URLDecode(const CString & strURLString, CString & strOutString);

void GetHtmlLinkString(const CString & strHtml, CStringArray & strOutArray);
void GetHtmlLinkTarget(const CString & strHtml, CStringArray & strOutArray);

bool GetAppPath(CString & strAppPath);
bool SaveLog(LPCTSTR lpszLogFileName, LPCTSTR lpszLog);

void TarverseLocalDir(CString strDir);
void TarverseFTPDir(CFTPClient & ftp, CString strDir);
CString GetLocalPath(void);

bool AvailablePrinter(CStringArray & strArray);
bool PrinterFile(LPCTSTR lpszPrinterName, CString strPrintData, LPCTSTR lpszOutPutFile = NULL);

void EncryptString(CString strInput, WORD nKey, CString * pstrOutput, const WORD nC1 = 58329, const WORD nC2 = 45218); 
void DecryptString(CString strInput, WORD nKey, CString * pstrOutput, const WORD nC1 = 58329, const WORD nC2 = 45218);

//us delay
bool Delay(double fMicrosencod);

//mathematical simulation
bool Linest(const double * pXValues, const double * pYValues, int nSampleNum, double & fSlope, double & fOffset, double & fMeanError);//y=slope*x+offset
bool Constantest(const double * pXValues, const double * pYValues, int nSampleNum, double & fCoefficient, double & fBase, double & fMeanError);//y=coff*(base^x)
bool Expest(const double * pXValues, const double * pYValues, int nSampleNum, double & fCoefficient, double & fSlope, double & fMeanError);//y=coff*e^(slope*x)
bool Logest(const double * pXValues, const double * pYValues, int nSampleNum, double & fSlope, double & fOffset, double & fMeanError);//y=slope*ln(x) + offset
bool Multiest(const double * pXValues, const double * pYValues, int nSampleNum, double * pCoefficient, double & fMeanError);//y=a1*x^n+a2*x^(n-1)+...an
bool CalculateDeterminant(const double *pDeterminant, int nPhase, double & fResult);//calculate the value of determinant