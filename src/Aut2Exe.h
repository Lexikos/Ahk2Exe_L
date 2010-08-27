#ifndef __AUT2EXE_H
#define __AUT2EXE_H

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
// Aut2Exe.h
//
// Includes and defines used in all modules.
//
///////////////////////////////////////////////////////////////////////////////

// Basic types
// 	Assumptions:
//		char 	= 	8bit
//		short	=	16bit
//		int		=	32bit
//		long	=	32bit
typedef unsigned char 	uchar;
typedef unsigned short 	ushort;
typedef unsigned int 	uint;
typedef unsigned long 	ulong;


// Match these with those in the main AutoIt source code
#define AUT_MAX_LINESIZE	4095				// Max size for a line of script - match with AutoIt.h
#define MAX_PASSLEN	64

// AutoHotkey: I changed the main Icon ID to match this value (could have been done here more
// easily, but I was having trouble getting a custom icon to be injected into a compiled script
// and thought some magic number might make it work):
#define AUT_ICONID	159							// MUST MATCH the icon ID in AutoItSC.bin

// General includes useful to all modules
#include "resources\resource.h"


// Windows related Defines
#define APPCLASS		"Aut2Exe"				// Must be same as .rc file!
#define APPTITLE		"Aut2Exe"

#define WM_NOTIFYICON	WM_USER+1

#define NOTIFY_ICON_ID	9001
#define STATUSBAR_ID	9002
#define REBAR_ID		9003
#define TOOLBAR_ID		9004
#define DIALOGBAR_ID	9005


// Safely delete an object
#define SAFE_DELETE(x) { if (x) delete(x); x = NULL; }

// Return 'x' and print error 'y' if 'x'
#define TRY_FALSE(x, y, z) { if (x == FALSE) {Util_ShowErrorIDS(y); return z;} }
#define TRY_NULL(x, y, z) { if (x == NULL) {Util_ShowErrorIDS(y); return z;} }


// Error codes and macros
#define AUTOIT_RESULT	bool
#define AUTOIT_OK		true
#define AUTOIT_ERR		false

#define AUTOIT_FAILED(Status)		((AUTOIT_RESULT)(Status)==AUTOIT_ERR)
#define AUTOIT_SUCCEEDED(Status)	((AUTOIT_RESULT)(Status)==AUTOIT_OK)


// Debug build macros for assertion checking and debug general output
#ifdef _DEBUG
	#include <assert.h>
	#define AUT_COMPILERDEBUGMSG(szText) OutputDebugString(szText);
	#define AUT_ASSERT(x) assert(x);
	#define AUT_DEBUGMESSAGEBOX(szText) MessageBox(NULL, szText, "", MB_OK);
#else
	#define AUT_COMPILERDEBUGMSG(szText)
	#define AUT_ASSERT(x)
	#define AUT_DEBUGMESSAGEBOX(szText)
#endif

///////////////////////////////////////////////////////////////////////////////

#endif

