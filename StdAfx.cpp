// stdafx.cpp : source file that includes just the standard includes
//	D2GDK_Client.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"




DWORD GetWoWTickCount()
{
	LARGE_INTEGER now = {0};
	QueryPerformanceCounter(&now);
	LARGE_INTEGER freq={0};
	QueryPerformanceFrequency(&freq); 

	return (DWORD)((now.QuadPart * 1000)/freq.QuadPart);
};

