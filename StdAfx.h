// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
#pragma once

#define WINVER 0x0600

#include <afxwin.h>         // MFC core and standard components

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning( disable : 4786 )
#pragma warning( disable : 4996 )

#define D2GDKEXPORT __declspec(dllimport)
#define D2GDKCLIENTEXPORT __declspec(dllexport)


extern DWORD GetWoWTickCount();
