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
// changeicon.cpp
//
// Same helper lib for changing the icon of an exe.
//
///////////////////////////////////////////////////////////////////////////////

//
// Icon information was from: http://msdn.microsoft.com/library/tet/msdn_icons.htm
//

// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
	#include <windows.h>
	#include <sys/stat.h>

#endif

#include "changeicon.h"


// Defines and strcutures
//#define ICON_DEBUG

#ifdef ICON_DEBUG
	char g_Icon_szDebug[2048];
#endif

// Icon file and Icon in memory/dll structures
// Pragmas used to enure that packing in memory matches packing of the EXE/DLL
#ifdef _MSC_VER
	#pragma pack(push)
	#pragma pack( 2 )
#endif
typedef struct
{
	BYTE		Width;
	BYTE		Height;
	BYTE		ColorCount;
	BYTE		reserved;
} ICONRESDIR;

typedef struct
{
	ICONRESDIR	Icon;
	WORD		Planes;
	WORD		BitCount;
	DWORD		BytesInRes;
	WORD		IconCursorId;
} RESDIR;

typedef struct
{
	ICONRESDIR	Icon;
	WORD		Planes;
	WORD		BitCount;
	DWORD		BytesInRes;
	DWORD		ImageOffset;
} RESDIRONDISK;

typedef struct
{
	WORD		Reserved;
	WORD		ResType;
	WORD		ResCount;
} NEWHEADER;
#ifdef _MSC_VER
	#pragma pack (pop)
#endif

// Global variables
HMODULE		g_Icon_hExe;
BYTE		*g_Icon_buf		= NULL;
int			g_Icon_buflen	= 0;
BYTE		*g_Icon_ico		= NULL;
int			g_Icon_icolen	= 0;


// Functions

int Icon_PerformReplace(BYTE *pIconBuf, DWORD BytesInRes, BYTE Width, BYTE Height, BYTE ColorCount)
{
    NEWHEADER		*pDirHeader;
    RESDIRONDISK	*pResDir;
    int				k;

	pDirHeader	= (NEWHEADER*)g_Icon_ico;
	pResDir		= (RESDIRONDISK*)(pDirHeader+1);

#ifdef ICON_DEBUG
    sprintf(g_Icon_szDebug, "Ico file contains %d icons\n", pDirHeader->ResCount);
	MessageBox(NULL, g_Icon_szDebug, "Debug", MB_OK);
#endif 
	for (k = 0; k < pDirHeader->ResCount; k++)
	{
		if (pResDir[k].BytesInRes == BytesInRes && pResDir[k].Icon.Width == Width &&
			pResDir[k].Icon.Height == Height && pResDir[k].Icon.ColorCount == ColorCount)
		{
#ifdef ICON_DEBUG
			sprintf(g_Icon_szDebug, "match! : %d[%d bytes] %dx%dx%d.\n", k, pResDir[k].BytesInRes,
					pResDir[k].Icon.Width, pResDir[k].Icon.Height, pResDir[k].Icon.ColorCount);
			MessageBox(NULL, g_Icon_szDebug, "Debug", MB_OK);
#endif 

			memcpy(pIconBuf, g_Icon_ico + pResDir[k].ImageOffset, BytesInRes);
			return TRUE;
		}

#ifdef ICON_DEBUG
		else
		{
			sprintf(g_Icon_szDebug, "no match! : %d[%d bytes] %dx%dx%d.\n", k, pResDir[k].BytesInRes,
					pResDir[k].Icon.Width, pResDir[k].Icon.Height, pResDir[k].Icon.ColorCount);
			MessageBox(NULL, g_Icon_szDebug, "Debug", MB_OK);

		}
#endif 
	}
	return FALSE; // match not found
}


BOOL CALLBACK Icon_EnumCallback(
	HMODULE		hExe,		// module handle
	LPCTSTR		lpszType,	// resource type
	LPTSTR		lpszName,	// resource name
	LPARAM		lParam		// application-defined parameter
)
{
	HRSRC		hRsrc = 0;
	HGLOBAL		hMem;
	DWORD		nDataLen;
	NEWHEADER	*pDirHeader;
	RESDIR		*pResDir;
	BYTE		*pData;
	DWORD		 i, k;

	hRsrc		= FindResource(hExe, lpszName, RT_GROUP_ICON);
	hMem		= LoadResource(hExe, hRsrc);
	pDirHeader	= (NEWHEADER*)LockResource(hMem);
	pResDir		= (RESDIR*)(pDirHeader+1);

    for (k = 0; k < pDirHeader->ResCount; k++)
    {
         hRsrc = FindResource(hExe, MAKEINTRESOURCE(pResDir[k].IconCursorId), RT_ICON);
         hMem = LoadResource(hExe, hRsrc );

         nDataLen = SizeofResource( hExe, hRsrc );
         pData = (BYTE *)LockResource(hMem);

#ifdef ICON_DEBUG
         sprintf(g_Icon_szDebug, "Icon found: %d.%d[%d bytes] %dx%dx%d (%d bytes loaded).\n",
              lpszName, pResDir[k].IconCursorId, pResDir[k].BytesInRes,
              pResDir[k].Icon.Width, pResDir[k].Icon.Height, pResDir[k].Icon.ColorCount, nDataLen);
		 MessageBox(NULL, g_Icon_szDebug, "Debug", MB_OK);
#endif 

		// Do a manual search of the exe data to match with the resource we are currently looking
		// at.  Brute force is the only way in this case :(
		for (i = 0; i <= g_Icon_buflen - nDataLen; i++)
		{
			DWORD j;
			for (j = 0; j < nDataLen; j++)
			{
				if (g_Icon_buf[i+j] != pData[j])
					break;
			}

			if (j == nDataLen)
			{
				if (Icon_PerformReplace(g_Icon_buf+i, pResDir[k].BytesInRes, pResDir[k].Icon.Width, pResDir[k].Icon.Height, pResDir[k].Icon.ColorCount))
					*(int*)lParam += 1;
				break;
			}
		}

	}
	return FALSE;								// stop enumeration after first icon group
}


bool Icon_List(const char *exename)				// Returns true if icon data was replaced
{
    int cnt = 0;

    g_Icon_hExe = LoadLibraryEx(exename, NULL, LOAD_LIBRARY_AS_DATAFILE);

    if (g_Icon_hExe == 0)
		return false;

    if (EnumResourceNames(g_Icon_hExe, RT_GROUP_ICON, Icon_EnumCallback, (LPARAM)&cnt) == 0 && cnt == 0)
    {
	    FreeLibrary(g_Icon_hExe);
		return false;							// Didn't find any icons to replace
    }

    FreeLibrary(g_Icon_hExe);

	return true;
}


long Icon_ReadFile(const char* filename, BYTE **pbuf)
{
	FILE	*in;
	long	bytesin;
	struct stat ST;

	in = fopen(filename, "rb");
	if (in == NULL)
		return 0;

    fstat(fileno(in), &ST);
    if (ST.st_size == 0)
		return 0;

	// Allocate enough memory to hold this file
	*pbuf = (BYTE *)malloc(ST.st_size);
    
	if (*pbuf == NULL)
    {
		fclose(in);
		return 0;
	}

    if (ST.st_size != (bytesin = fread(*pbuf, 1, ST.st_size, in)))
    {
		free(*pbuf);
		fclose(in);
		return 0;
    }

	fclose(in);

	return ST.st_size;
}


int Icon_Replace(const char *szExe, const char *szIcon)
{
    FILE	*out;

	// Read in the exe file to change
    g_Icon_buflen = Icon_ReadFile(szExe, &g_Icon_buf);
	if (g_Icon_buflen == 0)
		return ICON_E_READEXE;

	// Read in the icon file
    g_Icon_icolen = Icon_ReadFile(szIcon, &g_Icon_ico);
	if (g_Icon_icolen == 0)
	{
		free(g_Icon_buf);
		return ICON_E_READICON;
	}

	// List and replace icons
    if (Icon_List(szExe) == false)			// Any icons replaced?
	{
	    free(g_Icon_buf);
		free(g_Icon_ico);
		return ICON_E_NOMATCHINGICON;
	}

	// No longer need the icon file data in memory
    free(g_Icon_ico);

    out = fopen(szExe, "wb");
    if (out == NULL)
    {
		free(g_Icon_buf);
		return ICON_E_WRITEEXE;
    }

	// Write out our modified exe
    fwrite(g_Icon_buf, 1, g_Icon_buflen, out);

    // Clean up
	fclose(out);
    free(g_Icon_buf);

	return ICON_E_OK;
}

