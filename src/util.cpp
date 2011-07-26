
///////////////////////////////////////////////////////////////////////////////
//
// AutoIt
//
// Copyright (C)1999-2003:
//		- Jonathan Bennett <jon@hiddensoft.com>
//		- See "AUTHORS.txt" for contributors.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
///////////////////////////////////////////////////////////////////////////////
//
// util.cpp
//
// Util functions.
//
///////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
//	#include <stdlib.h>
//	#include <string.h>
	#include <windows.h>
#endif

#include "Aut2Exe.h"
#include "globaldata.h"
#include "util.h"


///////////////////////////////////////////////////////////////////////////////
// Util_ShowError()
///////////////////////////////////////////////////////////////////////////////

void Util_ShowError(char *szText)
{
	// AutoHotkey:
	MessageBox(g_hWnd, szText, "Ahk2Exe Error", MB_ICONSTOP | MB_OK | MB_SETFOREGROUND);

} // Util_ShowError()


///////////////////////////////////////////////////////////////////////////////
// Util_ShowErrorIDS()
///////////////////////////////////////////////////////////////////////////////

void Util_ShowErrorIDS(unsigned int iErr)
{
	char szText[AUT_MAX_LINESIZE+1];

	if ( LoadString(g_hInstance, iErr, szText, AUT_MAX_LINESIZE) == 0)
		szText[0] = '\0';

	// AutoHotkey:
	MessageBox(g_hWnd, szText, "Ahk2Exe Error", MB_ICONSTOP | MB_OK | MB_SETFOREGROUND);

} // Util_ShowError()


///////////////////////////////////////////////////////////////////////////////
// Util_ShowInfoIDS()
///////////////////////////////////////////////////////////////////////////////

void Util_ShowInfoIDS(unsigned int iErr)
{
	char szText[AUT_MAX_LINESIZE+1];

	if ( LoadString(g_hInstance, iErr, szText, AUT_MAX_LINESIZE) == 0)
		szText[0] = '\0';

	// AutoHotkey:
	MessageBox(g_hWnd, szText, "Ahk2Exe", MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);

} // Util_ShowError()


///////////////////////////////////////////////////////////////////////////////
// Util_GetFullPath()
///////////////////////////////////////////////////////////////////////////////

void	Util_GetFullPath(char *szInput)
{
	char	szNewPath[_MAX_PATH+1];
	char	*szFilePart;

	if (GetFullPathName(szInput, _MAX_PATH, szNewPath, &szFilePart) == 0)
		szInput[0] = '\0';
	else
		strcpy(szInput, szNewPath);

} // Util_ShowError()



///////////////////////////////////////////////////////////////////////////////
// Util_Run()
///////////////////////////////////////////////////////////////////////////////

int	Util_Run(const char *szCmd, const char *szDir, int nShow, bool bWait)
{
	char	szRun[_MAX_PATH+1];

	STARTUPINFO				si;

	PROCESS_INFORMATION		pi;
	BOOL					bRes;

	strcpy(szRun, szCmd);

	// init structure for running programs
	si.cb			= sizeof(STARTUPINFO);
	si.lpReserved	= NULL;
	si.lpDesktop	= NULL;
	si.lpTitle		= NULL;
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.cbReserved2	= 0;
	si.lpReserved2	= NULL;
	si.wShowWindow	= nShow;

	// Directory specified?
	if (szDir[0] == '\0')
		bRes = CreateProcess(NULL, szRun, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	else
		bRes = CreateProcess(NULL, szRun, NULL, NULL, FALSE, 0, NULL, szDir, &si, &pi);


	// Check for success
	if (bRes == FALSE)
		return -1;								// Oops

	CloseHandle(pi.hThread);


	while (bWait == true)
	{
		DWORD dwexitcode;
		GetExitCodeProcess(pi.hProcess, &dwexitcode);

		if (dwexitcode != STILL_ACTIVE)
		{
			CloseHandle(pi.hProcess);		// Close handle
			return (int)dwexitcode;
		}
		else
            Sleep(50);						// Quick sleep to reduce CPU load
	}

	// Not waiting if here, just return 0
	CloseHandle(pi.hProcess);
	return 0;

} // Util_Run()


///////////////////////////////////////////////////////////////////////////////
// Util_GetFileVersion()
//
// Gets the FileVersion info for a given filename, returns false if unsuccessful.
// szVersion must be 43+1 chars long for maxium version number size
//
///////////////////////////////////////////////////////////////////////////////

bool Util_GetFileVersion(char *szFile, char *szVersion)
{
	DWORD				dwUnused;
	DWORD				dwSize;
	BYTE				*pInfo;
	VS_FIXEDFILEINFO	*pFFI;
	UINT				uSize;
	unsigned int		iFileMS, iFileLS;
	
	// Get size of the info block
	dwSize = GetFileVersionInfoSize(szFile, &dwUnused);

	if (dwSize)
	{
		pInfo = (BYTE*)malloc(dwSize);

		// Read the version resource
		GetFileVersionInfo((LPSTR)szFile, 0, dwSize, (LPVOID)pInfo);

		// Locate the fixed information
		if (VerQueryValue(pInfo, "\\", (LPVOID *)&pFFI, &uSize)!=0)
		{
			//extract the fields you want from pFFI
			iFileMS = (unsigned int)pFFI->dwFileVersionMS;
			iFileLS = (unsigned int)pFFI->dwFileVersionLS;
			sprintf(szVersion, "%u.%u.%u.%u", (iFileMS >> 16), (iFileMS & 0xffff), (iFileLS >> 16), (iFileLS & 0xffff) );

			free(pInfo);
			return true;
		}
		else
		{
			free(pInfo);
			return false;
		}
	}
	else
		return false;

} // Util_GetFileVersion

/*
///////////////////////////////////////////////////////////////////////////////
// Util_GetFileMachine()
///////////////////////////////////////////////////////////////////////////////
WORD Util_GetFileMachine(const char *szFile)
{
	FILE *fp;
	if (fp = fopen(szFile, "rb"))
	{
		IMAGE_DOS_HEADER dosHeader;
		IMAGE_NT_HEADERS ntHeaders;
		if (   fread(&dosHeader, sizeof(dosHeader), 1, fp)	== 1   )
		if (   fseek(fp, dosHeader.e_lfanew, SEEK_SET)		== 0   )
		if (   fread(&ntHeaders, sizeof(ntHeaders), 1, fp)	== 1   )
		{
			fclose(fp);
			return ntHeaders.FileHeader.Machine;
		}
		fclose(fp);
	}
	return 0; // FAIL
}
*/