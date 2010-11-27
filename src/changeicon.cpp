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
// Helper lib for changing the icon of an exe.
//
///////////////////////////////////////////////////////////////////////////////

//
// source: http://msdn.microsoft.com/library/techart/msdn_icons.htm
// Icon IDs start at 1
//

// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
	#include <windows.h>
#endif

#include "Aut2Exe.h"
#include "changeicon.h"


// Icon editing structures
// Pragmas are used to ensure that the structures exist in memory in the same format that they exist
// in the EXE file, so that ops like IconGroupEntry[n] work correctly!
#pragma pack (push)
#pragma pack (2)
typedef struct {
  WORD wReserved;
  WORD wIsIcon;
  WORD wCount;
} IconGroupHeader;
#pragma pack (pop)

#pragma pack (push)
#pragma pack (2)
typedef struct {
  BYTE bWidth;
  BYTE bHeight;
  BYTE bPaletteEntries;
  BYTE bReserved;
  WORD wPlanes;
  WORD wBitsPerPixel;
  DWORD dwRawSize;
  WORD wRsrcId;
} RsrcIconGroupEntry;
#pragma pack (pop)

typedef struct {
  BYTE bWidth;
  BYTE bHeight;
  BYTE bPaletteEntries;
  BYTE bReserved;
  WORD wPlanes;
  WORD wBitsPerPixel;
  DWORD dwRawSize;
  DWORD dwImageOffset;
} FileIconGroupEntry;



#define SIZEOF_RSRC_ICON_GROUP_ENTRY 14
#define ICON_MAX_IMAGES	16						// Arbitary max images (RT_ICON) per ICON (RT_GROUP_ICON)
//#define ICON_DEBUG

int		g_Icon_IDs[ICON_MAX_IMAGES];			// List of the initial IDs for the requested icon
int		g_Icon_NumImages;						// Initial number of images (RT_ICON) associated with the requested icon group
int		g_Icon_nHighestID;						// Highest ID used for ALL icons in the exe



BOOL CALLBACK Icon_enum(
 HMODULE hExe,		// module handle
 LPCTSTR lpszType,  // resource type
 LPTSTR lpszName,   // resource name
 LPARAM lParam		// application-defined parameter
)
{
	HRSRC				hRsrc = 0;
	HGLOBAL				hMem;
	DWORD				nDataLen;
	IconGroupHeader		*pDirHeader;
	RsrcIconGroupEntry	*pResDir;
	unsigned int k;

	hRsrc = FindResource(hExe, lpszName, RT_GROUP_ICON);
	hMem = LoadResource(hExe, hRsrc);
	pDirHeader = (IconGroupHeader*)LockResource(hMem);
	pResDir = (RsrcIconGroupEntry*)(pDirHeader+1);

	for (k = 0; k < pDirHeader->wCount; k++)
	{
		hRsrc = FindResource(hExe, MAKEINTRESOURCE(pResDir[k].wRsrcId), RT_ICON);
		hMem = LoadResource(hExe, hRsrc );

		nDataLen = SizeofResource( hExe, hRsrc );

		// Is this the highest ID so far?
		if ((int)pResDir[k].wRsrcId > g_Icon_nHighestID)
			g_Icon_nHighestID = (int)pResDir[k].wRsrcId;

#ifdef ICON_DEBUG
		char szBuffer[256];
		sprintf(szBuffer, "Icon found: %d.%d[%d bytes] %dx%dx%d (%d bytes loaded).\n",
			lpszName,
			pResDir[k].wRsrcId, pResDir[k].dwRawSize,
			pResDir[k].bWidth, pResDir[k].bHeight, pResDir[k].bPaletteEntries, nDataLen);
		MessageBox(NULL, szBuffer, "", MB_OK);
#endif

	}

    return TRUE; // return false stop enumeration after first icon group
}


// Enums all icons to find the highest ID, then finds the IDs of the requested icon
bool Icon_list(const char *exename, WORD wIconId)
{
	HINSTANCE	hExe;
	int			cnt = 0;

    hExe = LoadLibraryEx(exename, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hExe == 0)
		return false;

	// Get the highest icon ID in the file (g_Icon_nHighestID) 
	g_Icon_nHighestID = 0;
    if (EnumResourceNames(hExe, RT_GROUP_ICON, Icon_enum, (LPARAM)&cnt) == 0 && cnt == 0)
	{
		FreeLibrary(hExe);
		return false;							// No icons at all found
    }

#ifdef ICON_DEBUG
	char szBuffer[256];
	sprintf(szBuffer, "Highest icon ID was: %d", g_Icon_nHighestID);
	MessageBox(NULL, szBuffer, "", MB_OK);
#endif


	// Find the IDs for our _requested_ icon
	HRSRC				hRsrc = 0;
	HGLOBAL				hMem;
	IconGroupHeader		*pDirHeader;
	RsrcIconGroupEntry	*pResDir;
	int k;
	unsigned int j;

	hRsrc		= FindResource(hExe, MAKEINTRESOURCE(wIconId), RT_GROUP_ICON);
	hMem		= LoadResource(hExe, hRsrc);
	pDirHeader	= (IconGroupHeader*)LockResource(hMem);
	pResDir		= (RsrcIconGroupEntry*)(pDirHeader+1);

	// Note all the ids in our ID array
	g_Icon_NumImages = pDirHeader->wCount;
	for (k = 0; k < g_Icon_NumImages; k++)
		g_Icon_IDs[k] = pResDir[k].wRsrcId;

#ifdef ICON_DEBUG
	char szBuffer2[256];
	sprintf(szBuffer2, "Number images in requested icon: %d",g_Icon_NumImages);
	MessageBox(NULL, szBuffer2, "", MB_OK);
#endif

	// Now, we will be deleting the images for this icon, change our array so that the
	// remaining entries start at highest ID (the array will then contain the image IDs
	// of the request icon followed by "free" entries - which will help when we are writing
	// the replacement icon
	j = g_Icon_nHighestID + 1;					// Next available icon ID
	for (k=g_Icon_NumImages; k < ICON_MAX_IMAGES; k++)
		g_Icon_IDs[k] = j++;
	
#ifdef ICON_DEBUG
	char szBuffer3[256];
	for (k=0; k < ICON_MAX_IMAGES; k++)
	{
		sprintf(szBuffer3, "Icon IDs: %d",g_Icon_IDs[k]);
		MessageBox(NULL, szBuffer3, "", MB_OK);
	}
#endif

	FreeLibrary(hExe);
	return true;
}


bool Icon_DoReplace(HANDLE re, WORD wIconId, const char* szIcon, const char *szExe)
{
	// Populates our array with the number of images in our requested icon and sets up the 
	// array of icon ids that we should use when replacing
	if (Icon_list(szExe, wIconId) == false)
		return false;							// Error processing icons in the exe


	FILE* f = fopen(szIcon, "rb");
	if (!f) 
		return false;

	IconGroupHeader igh;
	fread(&igh, sizeof(IconGroupHeader), 1, f);

	if (igh.wIsIcon != 1 && igh.wReserved != 0) 
		return false;

	BYTE* rsrcIconGroup = (BYTE*)malloc(sizeof(IconGroupHeader) + igh.wCount*SIZEOF_RSRC_ICON_GROUP_ENTRY);
	if (!rsrcIconGroup) 
		return false;

	CopyMemory(rsrcIconGroup, &igh, sizeof(IconGroupHeader));

	RsrcIconGroupEntry* ige = (RsrcIconGroupEntry*)(rsrcIconGroup + sizeof(IconGroupHeader));

	// Delete all the images in our requested icon
	int i;
	for (i=0; i<g_Icon_NumImages; i++)
		// AutoHotkey: Replaced SUBLANG_ENGLISH_UK with SUBLANG_ENGLISH_US so that icon replacement works
		// with the target EXE's language:
		UpdateResource(re, RT_ICON, MAKEINTRESOURCE(g_Icon_IDs[i]), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NULL, 0);


	// Now replace the images
	int iNewIconSize = 0;

	for (i = 0; i < igh.wCount; i++) 
	{
		fread(ige, sizeof(FileIconGroupEntry)-sizeof(DWORD), 1, f);
		ige->wRsrcId = g_Icon_IDs[i];

		DWORD dwOffset;
		fread(&dwOffset, sizeof(DWORD), 1, f);

		fpos_t pos;
		fgetpos(f, &pos);

		if (fseek(f, dwOffset, SEEK_SET)) 
			return false;

		BYTE* iconData = (BYTE*)malloc(ige->dwRawSize);
		if (!iconData) 
		{
			free(rsrcIconGroup);
			return false;
		}
		fread(iconData, sizeof(BYTE), ige->dwRawSize, f);
		// AutoHotkey: Replaced SUBLANG_ENGLISH_UK with SUBLANG_ENGLISH_US so that icon replacement works
		// with the target EXE's language:
		UpdateResource(re, RT_ICON, MAKEINTRESOURCE(g_Icon_IDs[i]), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), iconData, ige->dwRawSize);
		free(iconData);

		fsetpos(f, &pos);

		// Every icon entry should be 8 aligned
		iNewIconSize += ((ige->dwRawSize%8 == 0) ? ige->dwRawSize : ige->dwRawSize - (ige->dwRawSize%8) + 8);

		// Seems like the compiler refuses to increase the pointer by just 14.
		// If you'll replace this line by ige++ you will get unwanted results.
		// Note: actually this would work with the correct packing pragmas on the structures
		ige = (RsrcIconGroupEntry*)((BYTE*)ige + SIZEOF_RSRC_ICON_GROUP_ENTRY);
	}

	fclose(f);

	// Write the icon group
	// AutoHotkey: Replaced SUBLANG_ENGLISH_UK with SUBLANG_ENGLISH_US so that icon replacement works
	// with the target EXE's language:
	UpdateResource(re, RT_GROUP_ICON, MAKEINTRESOURCE(wIconId), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), rsrcIconGroup, sizeof(IconGroupHeader) + igh.wCount*SIZEOF_RSRC_ICON_GROUP_ENTRY);

	free(rsrcIconGroup);

	return true;
}

