// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#pragma once
#define _WIN32_WINNT 0x0502
#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS         // �Ƴ��Ի����е� MFC �ؼ�֧��

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ
#endif

#include <afx.h>
#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>

#include <shlwapi.h>
#include <atlbase.h>
#include "FTPClient.h"