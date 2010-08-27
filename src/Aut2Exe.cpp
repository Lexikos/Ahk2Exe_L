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
// Aut2Exe.cpp
//
// WinMain entry point.
//
///////////////////////////////////////////////////////////////////////////////


/*
 * Aut2Exe.cpp
 *
 *
 * Dialog Based Main Window
 * - CreateDialog has no DlgProc as it is registered in the class
 * - Added "Class" parameter in DialogBox (Must turn off MFC controls for this
 *		entry to appear! (Must be same as szAppClass/APPCLASS in main.h).
 * - DLGWINDOWEXTRA in class registration
 * - IsDialogMessage needed in message loop for TABing between controls
 *
 * Common control usage (toolbar, status bar)
 * - Added link lib comctl32.lib
 * - Register controls using InitCommonControls()
 *
 * ToolBars
 * - Using icons from the system built in toolbar in commctrl
 * - Create using CreateToolBarEx and TBBUTTON array with IDs assigned
 *   from a previously created menubar
 * - WM_NOTIFY used to set tooltips
 * - Tooltip text, is same as command ID+1 - see resource.h
 *
 * Status bar
 * - Created using CreateWindow
 * - If using a non-dialog app must respond to sizing messages!!
 * - Each command (toolbar/command/menuitem) must correspond to a string
 *     resource for tips to work correctly.  Check resource.h for details.
 * - The status bar can be split into parts using SB_SETPARTS.
 * - The status bar can be written to using SB_SETTEXT.
 *
 *
 */


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
//	#include <stdlib.h>
//	#include <string.h>
	#include <windows.h>
#endif

#include "Aut2Exe.h"
#include "application.h"
#include "cmdline.h"
#include "util.h"


// Global Variables
HWND				g_hWnd;
HINSTANCE			g_hInstance;
char                g_ScriptDir[MAX_PATH];
bool                g_OmitFurtherIncludeComments = false;

App					g_oApp;
CmdLine				g_oCmdLine;

///////////////////////////////////////////////////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
	char		*szCommandLine;

	// Set hinst, and zero hWnd
	g_hInstance		= hInstance;
	g_hWnd			= NULL;

	// Init CmdLine (Get full command line -- same as in main())
	szCommandLine = GetCommandLine();
	g_oCmdLine.SetCmdLine(szCommandLine);

	// Run the app
	if (g_oApp.Create(nCmdShow) == FALSE)
		return 1;
	else
		return 0;

} // WinMain()

