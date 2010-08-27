#ifndef __GLOBALDATA_H
#define __GLOBALDATA_H

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
// globaldata.h
//
// Externs for the global data contained in Aut2Exe.cpp
//
///////////////////////////////////////////////////////////////////////////////


// Includes required for the globaldata
#include "application.h"
#include "cmdline.h"

// Global data external definitions
extern HWND					g_hWnd;
extern HINSTANCE			g_hInstance;
extern char                 g_ScriptDir[MAX_PATH];
extern bool                 g_OmitFurtherIncludeComments;

extern App					g_oApp;
extern CmdLine				g_oCmdLine;


#endif
