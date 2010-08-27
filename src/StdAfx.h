#ifndef __STDAFX_H
#define __STDAFX_H

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
// StdAfx.h
//
// The pre-compiled header file.  This should be included in any source file
// that uses the includes defined here (it is a lot faster!)
// If you don't want to include this header then change the properties of the
// source file to not use .pch
//
///////////////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_WARNINGS


// Only use the includes here if running Visual C - it is really slow under
// MingW to use this method so we will comment out the headers unless the
// compiler is VC (and manually include the required headers in the source files)

#define	WINVER	0x0400							// Make "windows.h" support 95 & NT4.0
#define  _WIN32_IE 0x0400						// Set commctrl.h Compatability to IE4+

#ifdef _MSC_VER
	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
	#include <sys/timeb.h>
	#include <string.h>
	#include <ctype.h>							// tolower()
	#include <limits.h>							// INT limits macros
	#include <windows.h>
	#include <mmsystem.h>
	#include <process.h>						// Multithreaded commands (beginthreadex, etc)
	#include <commctrl.h>
	#include <sys/stat.h>
#endif

///////////////////////////////////////////////////////////////////////////////

#endif
