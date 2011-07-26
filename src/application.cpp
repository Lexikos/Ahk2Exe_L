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
// application.cpp
//
// The main application object.
//
///////////////////////////////////////////////////////////////////////////////

// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
//	#include <stdlib.h>
//	#include <string.h>
	#include <windows.h>
	#include <commctrl.h>
	#include <sys/stat.h>
#endif


#include "Aut2Exe.h"
#include "globaldata.h"
#include "application.h"
#include "util.h"
// #include "lib\exearc_write.h" // AutoHotkey: This needs to be included in the application.h instead.
#include "astring_datatype.h"
#include "token_datatype.h"
#include "vector_token_datatype.h"
#include "changeicon.h"
#include "scriptfile.h"
#include <Shlobj.h> // for SHGetFolderPath


///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////

App::App()
{
	int		n;

	// Zero everything 
	m_hWndStatusbar		= NULL;
	m_nCmdShow			= 0;

	m_szLastScriptDir[0]	= '\0';
	m_szLastExeDir[0]		= '\0';
	m_szLastIconDir[0]		= '\0';
	m_szLastIcon[0]			= '\0';
	m_dwLastCompression		= 2;


	// Setup our directories
	GetModuleFileName(NULL, m_szAut2ExeDir, _MAX_PATH);

	n = strlen(m_szAut2ExeDir)-1;
	while (n >= 0)
	{
		if (m_szAut2ExeDir[n] == '\\')
		{	
			m_szAut2ExeDir[n+1] = '\0';
			break;
		}
		n--;
	}

	// m_szAut2ExeDir will always end in \ 

	// Get the full path of the AutoItSC.bin file
	strcpy(m_szAutoItSC, m_szAut2ExeDir);
	// AutoHotkey:
	//strcat(m_szAutoItSC, "AutoItSC.bin");
	strcat(m_szAutoItSC, "AutoHotkeySC.bin");


} // App()


///////////////////////////////////////////////////////////////////////////////
// Create
///////////////////////////////////////////////////////////////////////////////

BOOL App::Create(int nCmdShow)
{
	MSG		msg;	
	HACCEL	hAccel;

	// Save the app details in our class
	m_nCmdShow		= nCmdShow;

	// Load settings from registry
	LoadSettings();

	// Check if this was called with correct cmdline options, if so do the convert
	// directly and don't register classes/open windows etc.
	if ( CmdLineMode() == true )
	{
		SaveSettings();
		return TRUE;
	}

	// Register our class
	TRY_FALSE( RegisterClass(), IDS_E_REGISTERCLASS, FALSE )

	// Register common controls
	RegisterCommonControls();

	// Load accelerators
	hAccel = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL1));


	// Create main window
	g_hWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL,	NULL);
	TRY_NULL( g_hWnd, IDS_E_CREATEMAINWINDOW, FALSE )
	

	// Create status bar
	TRY_FALSE( StatusbarCreate(), IDS_E_CREATETOOLS, FALSE )


	// Show our window
	ShowWindow(g_hWnd, m_nCmdShow);
	UpdateWindow(g_hWnd);
	SetFocus(GetDlgItem(g_hWnd, ID_CONVERT));		// set focus on the CONVERT button

	// Set any defaults for the dialog
	SetDlgItemText(g_hWnd, IDC_ICONEDIT, m_szLastIcon);
	SendDlgItemMessage(g_hWnd, IDC_PASSEDIT, EM_LIMITTEXT, MAX_PASSLEN, 0);
#ifdef SHOW_COMPRESSION_MENU
	CheckMenuItem(GetMenu(g_hWnd), ID_COMPRESSION_LOWEST + m_dwLastCompression, MF_CHECKED);
#endif


	// Process the message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(g_hWnd, hAccel, &msg))
		{
			if(!IsDialogMessage(g_hWnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	// Save our settings back to the registry
	SaveSettings();

	return TRUE;

} // Create()


///////////////////////////////////////////////////////////////////////////////
// WndProc
//
// C++ Gotcha
// This member function has been declared as STATIC so that it can be used as
// a callback function.  This means that there is only ONE instance of this 
// function irrespective of how many parent class objects there are.
// Very dodgy, but IMHO better that having a global callback function.  Also
// This function will not know which object instance called it - so we will have
// to go back to the gApp object to get our bearings.
// In our case there is only ever ONE object, so not much of a problem
///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK App::WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	return g_oApp.WndProcHandler(hWnd, iMsg, wParam, lParam);

} // WndProc()


///////////////////////////////////////////////////////////////////////////////
// WndProcHandler
// Handle messages that are sent to our app window
///////////////////////////////////////////////////////////////////////////////

LRESULT App::WndProcHandler (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
		// Window initial creation
		case WM_CREATE:
			return 0;

		// Window close clicked
		case WM_CLOSE:
			break;	// DefWindowProc, returning 0 here would prevent closing

		// Window is being destroyed
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		// User has moved over a menu item
		case WM_MENUSELECT:
			MenuSelect(hWnd, iMsg, wParam, lParam);
			return 0;


		// Execute menu/button command
		case WM_COMMAND:
			Command(hWnd, iMsg, wParam, lParam);
			return 0;

	} // end main switch

	return DefWindowProc(hWnd, iMsg, wParam, lParam);

} // WndProcHandler()


///////////////////////////////////////////////////////////////////////////////
// MenuSelect
///////////////////////////////////////////////////////////////////////////////

void App::MenuSelect(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu;
	int iMenu;

	hMenu = (HMENU)lParam;						// get the menu handle
	iMenu = LOWORD(wParam);						// get the index

	// if hMenu is out of Main menu (File, Edit, etc), then just use
	// IDS_MENU_MAIN1 as reference and add on iMenu, otherwise, load a string
	// resource with the same ID as iMenu!

	if (hMenu == GetMenu(hWnd))
		StatusbarWrite(IDS_MENU_MAIN1 + iMenu);
	else
		StatusbarWrite(iMenu);

} // mainMenuSelect()


///////////////////////////////////////////////////////////////////////////////
// Command
// Execute a button/menu selection
///////////////////////////////////////////////////////////////////////////////

void App::Command(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	HWND			hCtrl;
	OPENFILENAME	ofn;
	char			szSource[_MAX_PATH+1];
	char			szDest[_MAX_PATH+1];
	char			szIcon[_MAX_PATH+1];
	char			szPass[MAX_PASSLEN+1];
	char			szPassVerify[MAX_PASSLEN+1];

	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];

	// Save the focus in case a "MessageBox" or similar changes it
	hCtrl = GetDlgItem(hWnd, LOWORD(wParam));

	// Setup the common members of the OFN struct
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = _MAX_PATH;
	ofn.nFilterIndex = 1;

	switch (LOWORD(wParam))
	{
		case IDC_SOURCEFILE:
			// AutoHotkey:
			//ofn.lpstrFilter = "AutoIt files (*.au3)\0*.au3\0All files (*.*)\0*.*\0";
			//ofn.lpstrDefExt = "aut";
			ofn.lpstrFilter = "AutoHotkey files (*.ahk)\0*.ahk\0All files (*.*)\0*.*\0";
			ofn.lpstrDefExt = "ahk";
			ofn.lpstrInitialDir = m_szLastScriptDir;
			szSource[0] = '\0';
			ofn.lpstrFile = szSource;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if ( GetOpenFileName(&ofn) == TRUE ) 
			{
				SetDlgItemText(hWnd, IDC_SOURCEEDIT, szSource);
				_splitpath( szSource, szDrive, szDir, szFname, szExt );
				strcpy(m_szLastScriptDir, szDrive);
				strcat(m_szLastScriptDir, szDir);
			}
			SetFocus(hCtrl);
			break;

		case IDC_DESTFILE:
			ofn.lpstrFilter = "Executable files (*.exe)\0*.exe\0";
			ofn.lpstrDefExt = "exe";
			ofn.lpstrInitialDir = m_szLastExeDir;
			szDest[0] = '\0';
			ofn.lpstrFile = szDest;
			ofn.Flags = OFN_HIDEREADONLY;
			if ( GetSaveFileName(&ofn) == TRUE ) 
			{
				SetDlgItemText(hWnd, IDC_DESTEDIT, szDest);
				_splitpath( szDest, szDrive, szDir, szFname, szExt );
				strcpy(m_szLastExeDir, szDrive);
				strcat(m_szLastExeDir, szDir);
			}
			SetFocus(hCtrl);
			break;

		case IDC_ICONFILE:
			ofn.lpstrFilter = "Icon files (*.ico)\0*.ico\0";
			ofn.lpstrDefExt = "ico";
			ofn.lpstrInitialDir = m_szLastIconDir;
			szIcon[0] = '\0';
			ofn.lpstrFile = szIcon;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

			if ( GetOpenFileName(&ofn) == TRUE ) 
			{
				SetDlgItemText(hWnd, IDC_ICONEDIT, szIcon);
				_splitpath( szIcon, szDrive, szDir, szFname, szExt );
				strcpy(m_szLastIconDir, szDrive);
				strcat(m_szLastIconDir, szDir);
				strcpy(m_szLastIcon, szIcon);
			}
			SetFocus(hCtrl);
			break;

		case IDC_ICONDEFAULT:
			SetDlgItemText(hWnd, IDC_ICONEDIT, "");
			m_szLastIconDir[0] = '\0';
			m_szLastIcon[0] = '\0';
			SetFocus(hCtrl);
			break;

		case IDC_PASSDEFAULT:
			SetDlgItemText(hWnd, IDC_PASSEDIT, "");
			SetDlgItemText(hWnd, IDC_PASSVERIFY, "");

			SetFocus(hCtrl);
			break;

#ifdef SHOW_COMPRESSION_MENU
		case ID_COMPRESSION_LOWEST:
		case ID_COMPRESSION_LOW:
		case ID_COMPRESSION_NORMAL:
		case ID_COMPRESSION_HIGH:
		case ID_COMPRESSION_HIGHEST:
			CheckMenuItem(GetMenu(hWnd), ID_COMPRESSION_LOWEST, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_COMPRESSION_LOW, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_COMPRESSION_NORMAL, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_COMPRESSION_HIGH, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_COMPRESSION_HIGHEST, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), LOWORD(wParam), MF_CHECKED);
			m_dwLastCompression = LOWORD(wParam) - ID_COMPRESSION_LOWEST;
			break;
#endif


		case ID_CONVERT:
			// Get the text values from the edit controls
			GetDlgItemText(hWnd, IDC_SOURCEEDIT, szSource, _MAX_PATH);
			GetDlgItemText(hWnd, IDC_DESTEDIT, szDest, _MAX_PATH);
			GetDlgItemText(hWnd, IDC_ICONEDIT, szIcon, _MAX_PATH);
			GetDlgItemText(hWnd, IDC_PASSEDIT, szPass, _MAX_PATH);
			GetDlgItemText(hWnd, IDC_PASSVERIFY, szPassVerify, _MAX_PATH);

			// Check that passphrases match, if so compile
			if ( !strcmp(szPass, szPassVerify) )
			{
				// Run conversion
				if ( Convert(szSource, szDest, szIcon, szPass) == true )
					Util_ShowInfoIDS(IDS_CONVERTCOMPLETE);
			}
			else
				Util_ShowErrorIDS(IDS_E_PASSVERIFY);

			SetFocus(hCtrl);
			break;

		case ID_ABOUT:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
			SetFocus(GetDlgItem(hWnd, ID_CONVERT));
			break;
	
		case ID_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;

	} // end switch

} // Command()


///////////////////////////////////////////////////////////////////////////////
// AboutProc
///////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK App::AboutProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
		case WM_INITDIALOG:
			return TRUE;

		case WM_CLOSE:
			EndDialog(hDlg, 0);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD (wParam))
			{
				case ID_OK:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			break;

	} // end switch

	return FALSE;

} // mainAboutProc()


///////////////////////////////////////////////////////////////////////////////
// StatusbarCreate()
///////////////////////////////////////////////////////////////////////////////

BOOL App::StatusbarCreate(void)
{
	m_hWndStatusbar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
					CCS_BOTTOM, "Ready", g_hWnd, STATUSBAR_ID);
	
	if (m_hWndStatusbar == NULL)
		return FALSE;
	else
		return TRUE;

} // StatusBarCreate()


///////////////////////////////////////////////////////////////////////////////
// StatusBarWrite()
// Overloaded function
///////////////////////////////////////////////////////////////////////////////

void App::StatusbarWrite(UINT nID)
{
	/* matches up an ID (command, menu, etc) and loads a corresponding
	   string resource, or "Ready" if there is no match */

	char szText[127+1];

	if ( LoadString(g_hInstance, nID, szText, 127) == 0)
		LoadString(g_hInstance, IDS_READY, szText, 127);

	SendMessage(m_hWndStatusbar, SB_SETTEXT, 0, (LPARAM)szText);

} // StatusBarWrite()


void App::StatusbarWrite(char *szText)
{
	SendMessage(m_hWndStatusbar, SB_SETTEXT, 0, (LPARAM)szText);

} // StatusBarWrite()


///////////////////////////////////////////////////////////////////////////////
// RegisterClass()
///////////////////////////////////////////////////////////////////////////////

BOOL App::RegisterClass(void)
{
	char		szAppClassName[] = APPCLASS;
	WNDCLASSEX	wndClass;

	// Init the class structure
	wndClass.cbSize			= sizeof(wndClass);
	wndClass.style			= CS_HREDRAW | CS_VREDRAW;
	wndClass.cbClsExtra		= 0;
	wndClass.cbWndExtra		= DLGWINDOWEXTRA;
	wndClass.hInstance		= g_hInstance;
	wndClass.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	//wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName	= MAKEINTRESOURCE(IDR_MENU1);
	wndClass.lpszClassName	= szAppClassName;
	wndClass.hIconSm		= LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wndClass.hIcon			= LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MAIN));
	wndClass.lpfnWndProc	= WndProc;

	if (RegisterClassEx(&wndClass) == 0)
		return FALSE;
	else
		return TRUE;

} // RegisterClass()


///////////////////////////////////////////////////////////////////////////////
// RegisterCommonControls()
///////////////////////////////////////////////////////////////////////////////

void App::RegisterCommonControls(void)
{
	INITCOMMONCONTROLSEX	iccex; 

	iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;	// Rebar support
	InitCommonControlsEx(&iccex);

} // RegisterCommonControls()


///////////////////////////////////////////////////////////////////////////////
// LoadSettings()
///////////////////////////////////////////////////////////////////////////////

void App::LoadSettings(void)
{
	DWORD	dwRes;
	HKEY	hRegKey;

	// Get the registry key values - create if neccessary 
	// AutoHotkey:
	//RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\HiddenSoft\\AutoIt3\\Aut2Exe", 0,
	//	"", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hRegKey, &dwRes);
	RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\AutoHotkey\\Ahk2Exe", 0,
		"", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hRegKey, &dwRes);

	dwRes = _MAX_PATH;	
	RegQueryValueEx(hRegKey, "LastScriptDir", NULL, NULL, (LPBYTE)m_szLastScriptDir, &dwRes);
	
	dwRes = _MAX_PATH;	
	RegQueryValueEx(hRegKey, "LastExeDir", NULL, NULL, (LPBYTE)m_szLastExeDir, &dwRes);
	
	dwRes = _MAX_PATH;	
	RegQueryValueEx(hRegKey, "LastIconDir", NULL, NULL, (LPBYTE)m_szLastIconDir, &dwRes);
	
	dwRes = _MAX_PATH;	
	RegQueryValueEx(hRegKey, "LastIcon", NULL, NULL, (LPBYTE)m_szLastIcon, &dwRes);

	dwRes = sizeof(m_dwLastCompression);	
	RegQueryValueEx(hRegKey, "LastCompression", NULL, NULL, (LPBYTE)&m_dwLastCompression, &dwRes);
	if (m_dwLastCompression > 4)
		m_dwLastCompression = 2;

	RegCloseKey(hRegKey);

} // GetSettings()


///////////////////////////////////////////////////////////////////////////////
// SaveSettings()
///////////////////////////////////////////////////////////////////////////////

void App::SaveSettings(void)
{
	HKEY	hRegKey;
	DWORD	dwRes;

	// Get the registry key values - create if neccessary 
	// AutoHotkey:
	//RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\HiddenSoft\\AutoIt3\\Aut2Exe", 0,
	//	"", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hRegKey, &dwRes);
	RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\AutoHotkey\\Ahk2Exe", 0,
		"", REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hRegKey, &dwRes);

	RegSetValueEx(hRegKey, "LastScriptDir", 0, REG_SZ, (LPBYTE)m_szLastScriptDir, strlen(m_szLastScriptDir)+1);
	RegSetValueEx(hRegKey, "LastExeDir", 0, REG_SZ, (LPBYTE)m_szLastExeDir, strlen(m_szLastExeDir)+1);
	RegSetValueEx(hRegKey, "LastIconDir", 0, REG_SZ, (LPBYTE)m_szLastIconDir, strlen(m_szLastIconDir)+1);
	RegSetValueEx(hRegKey, "LastIcon", 0, REG_SZ, (LPBYTE)m_szLastIcon, strlen(m_szLastIcon)+1);
	RegSetValueEx(hRegKey, "LastCompression", 0, REG_DWORD, (LPBYTE)&m_dwLastCompression, sizeof(m_dwLastCompression));

	RegCloseKey(hRegKey);

} // GetSettings()


///////////////////////////////////////////////////////////////////////////////
// CmdLineMode()
///////////////////////////////////////////////////////////////////////////////

bool App::CmdLineMode(void)
{
	char	szIn[CMDLINE_MAXLEN+1];
	char	szOut[CMDLINE_MAXLEN+1];
	char	szIcon[CMDLINE_MAXLEN+1];
	char	szTemp[CMDLINE_MAXLEN+1];
	char	szPass[MAX_PASSLEN+1];
	int		i;

	// aut2exe.exe /in nnnn [/out nnnn] [/icon nnnn] [/pass nnnn]
	// 0            1   2     

	// Zero (some \0 checks are made for optional parameters)
	szIn[0]		= '\0';
	szOut[0]	= '\0';
	szIcon[0]	= '\0';
	szPass[0]	= '\0';

	// Any params? (should always be 1 -- 1st param is aut2exe.exe )
	if ( g_oCmdLine.GetNumParams() == 1 )
		return false;

	// Reset the parsing
	g_oCmdLine.GetNextParamReset();

	// Skip the program name in the command line
	g_oCmdLine.GetNextParam(szTemp);

	// Check in
	if ( g_oCmdLine.GetNextParam(szTemp) == false || g_oCmdLine.GetNextParam(szIn) == false )
	{
		Util_ShowInfoIDS(IDS_CMDLINEPARAMS);
		return false;
	}
	else
	{
		if ( _stricmp(szTemp, "/in") )
		{
			Util_ShowInfoIDS(IDS_CMDLINEPARAMS);
			return false;
		}
	}

	for (i=0; i<4; i++)		// Allow up to 4 distinct command-line switches.
	{
		// Check next param (/out or /icon or /pass)
		g_oCmdLine.GetNextParam(szTemp);
		if ( !_stricmp(szTemp, "/out") )
			g_oCmdLine.GetNextParam(szOut);
		else if ( !_stricmp(szTemp, "/icon") )
			g_oCmdLine.GetNextParam(szIcon);
		else if ( !_stricmp(szTemp, "/pass") )
			g_oCmdLine.GetNextParam(szPass);
		else if ( !_stricmp(szTemp, "/bin") )
			g_oCmdLine.GetNextParam(m_szAutoItSC);
		else if ( !_stricmp(szTemp, "/NoDecompile") )
		{
			Util_ShowInfoIDS(IDS_E_NODECOMPILE);
			return false;
		}
		else if ( szTemp[0] != '\0' )	// not /out /icon /pass or blank - error
		{
			Util_ShowInfoIDS(IDS_CMDLINEPARAMS);
			return false;
		}
	}


	// if icon not specified, use default
	if ( szIcon[0] == '\0' )
		strcpy(szIcon, m_szLastIcon);

	// Ensure that both source and dest are LFN! - They sometimes aren't when you do drag and
	// Drop type operations - 95 Option required!
	// Now using %l instead of %1 to force ShellCommands to pass a LFN
	//GetLongPathName(szIn, szIn, _MAX_PATH);
	//GetLongPathName(szOut, szOut, _MAX_PATH);

	// OK, run the conversion
	if (Convert(szIn, szOut, szIcon, szPass)) // v1.0.42.08: To support compiling from inside editors such as PSPad (by Toralf).
		printf("Successfully compiled: %s\n", szOut);
	else
		printf("Failed to compile: %s\n", szOut);
	return true;

} // CmdLineMode()


///////////////////////////////////////////////////////////////////////////////
// ConvertCheckFilenames()
///////////////////////////////////////////////////////////////////////////////

bool App::ConvertCheckFilenames(const char *szSource, char *szDest)
{
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];

	// Source is required, Dest is optional 

	// Filenames specified?
	if ( szSource[0] == '\0' )
	{
		Util_ShowErrorIDS(IDS_E_NOFILES);
		return false;
	}
	
	if ( szDest[0] != '\0' )
		return true;

	// No destination was specified, strip the extension from the source and
	// change to .exe
	_splitpath( szSource, szDrive, szDir, szFname, szExt );
	sprintf(szDest, "%s%s%s.exe", szDrive, szDir, szFname);
	return true;

} // ConvertCheckFilenames()


///////////////////////////////////////////////////////////////////////////////
// Convert()
// True = success
///////////////////////////////////////////////////////////////////////////////

bool App::Convert(char *szSource, char *szDest, char *szIcon, char *szPass)
{
	char	szScriptTemp[_MAX_PATH+1];
	char	szTempPath[_MAX_PATH+1];
	FILE	*fScript;

	char	szOldWorkingDir[_MAX_PATH+1];
	char	*szFilePart;

	char	szCmdLine[_MAX_PATH*3]; // v1.0.47: Increased from _MAX_PATH+1 to allow room for two full-paths with extra switches/options.

	// Make all the filenames are fullpaths (not relative)
	Util_GetFullPath(szDest);
	Util_GetFullPath(szSource);
	Util_GetFullPath(szIcon);

//	MessageBox(NULL, szSource, "", MB_OK);
//	MessageBox(NULL, szDest, "", MB_OK);
//	MessageBox(NULL, szIcon, "", MB_OK);

	// Check if both a source and dest was specified
	if ( ConvertCheckFilenames(szSource, szDest) == false )
		return false;

	// First we put up an hourglass - this may take a while
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	// Copy our sc.bin file to the destination
	if (!CopyFile(m_szAutoItSC, szDest, FALSE))
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));	// Reset cursor
		Util_ShowErrorIDS(IDS_E_COPYSCBIN);
		return false;
	}

	
	// Now, change the resources as required (icons, versioninfo)
	if (*szIcon) // AutoHotkey: version info isn't changed, so skip this step if no icon was specified.
	{
		HANDLE re;

		// Open exe file for updating resources
		re = BeginUpdateResource(szDest, FALSE);
		if ( re == NULL )
		{
			Util_ShowErrorIDS(IDS_E_WRITEDEST);
			return false;
		}

		// Change the icon if a user one is specified
		if ( Icon_DoReplace(re, AUT_ICONID, szIcon, szDest) != true )
		{
			EndUpdateResource(re, TRUE);
			Util_ShowErrorIDS(IDS_E_ICONFORMAT);
			return false;
		}

		// Write out our modified exe
		if ( EndUpdateResource(re, FALSE) == FALSE )
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));	// Reset cursor
			DeleteFile(szDest);						// The output is trashed -- delete it.
			Util_ShowErrorIDS(IDS_E_WRITEDEST);
			return false;
		}
	}


	StatusbarWrite(IDS_READY);		// Reset statusbar


	/////////////////////////////////////////////////////////////////////////////
	// AutoHotkey:
	// Handle #include directives by merging them into one large combined script.
	// This will result the line numbers being unmeaningful as reported by any
	// runtime error dialogs, but those are pretty rare, so its just documented
	// here as a limitation.  This is mostly the AutoIt3 code with a few changes
	// to handle the slightly different #include syntax.
	/////////////////////////////////////////////////////////////////////////////

	// Get a temporary filename for the script
	GetTempPath(_MAX_PATH, szTempPath);
	// AutoHotkey:
	//GetTempFileName(szTempPath, "aut", 0, szScriptTemp);
	GetTempFileName(szTempPath, "ahk", 0, szScriptTemp);

	// Create the script file for text writing
	if ( (fScript = fopen(szScriptTemp, "w+")) == NULL )
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));	// Reset cursor
		DeleteFile(szDest);						// The output is trashed -- delete it.
		Util_ShowErrorIDS(IDS_E_TEMPFILE);
		return false;
	}

	// Get the current working directory
	GetCurrentDirectory(_MAX_PATH, szOldWorkingDir);

	// Set the working directory based on the script dir
	// this dir will be used in nested includes and Fileinstalls
	GetFullPathName(szSource, _MAX_PATH, g_ScriptDir, &szFilePart);
	szFilePart[0] = '\0';
	SetCurrentDirectory(g_ScriptDir);
		
	// Open our exe file for archive operations (max compression - range is 0-4)
	EXEArc_Write	oWrite;
	if ( oWrite.Open(szDest, szPass, m_dwLastCompression) != HS_EXEARC_E_OK )
	{
		Util_ShowErrorIDS(IDS_E_CREATEARCHIVE);
		return false;
	}

	// Read the script (and any includes) and write to fScript
	if ( ScriptRead(szSource, fScript, oWrite, szScriptTemp) == false ) // It's also responsible for closing fScript.
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));	// Reset cursor
		DeleteFile(szDest);						// The output is trashed -- delete it.
		SetCurrentDirectory(szOldWorkingDir);	// Reset the working directory
		DeleteFile(szScriptTemp);				// And delete it (we won't use it now)
		oWrite.Close(); // AutoHotkey: added.
		return false;
	}
	// If we get to here then the temporary scripts have been populated and any include files included.

	// Reset the working directory to be g_ScriptDir in case ScriptRead's call of
	// LoadIncludedFile's call of IsPreprocessorDirective() changed it via "#Include NewDir".
	SetCurrentDirectory(g_ScriptDir);

	// Compile!!!!!
	// Use a different filename if the script has a custom icon so that the self-contained EXE
	// can detect whether it's safe to use the special green icons for the tray in Win95/98/NT/2000:
	if ( CompileScript(szScriptTemp, szDest, szPass, oWrite, *szIcon ? ">AHK WITH ICON<" : ">AUTOHOTKEY SCRIPT<") == false)
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));	// Reset cursor
		SetCurrentDirectory(szOldWorkingDir);	// Reset working directory
		DeleteFile(szScriptTemp);				// And delete temp script (we won't use it now)
		DeleteFile(szDest);						// The output is trashed -- delete it.
		oWrite.Close(); // AutoHotkey: added.
		return false;
	}
	oWrite.Close(); // AutoHotkey: added.

	// Compress the final executable
	StatusbarWrite("Compressing final executable...");
	//sprintf(szCmdLine, "\"%supx.exe\" --best --filter=73 --lzma --compress-icons=0 \"%s\""
	sprintf(szCmdLine, "\"%smpress.exe\" -q -x \"%s\""
			, m_szAut2ExeDir // Contains a trailing backslash.
			, szDest);
	Util_Run(szCmdLine, m_szAut2ExeDir, SW_HIDE, true);

	// Change cursor back to an arrow
	SetCursor(LoadCursor(NULL, IDC_ARROW));

	// Reset the working directory
	SetCurrentDirectory(szOldWorkingDir);

	// Delete our temporary script file
	DeleteFile(szScriptTemp);

	StatusbarWrite("");
	return true;

} // Convert()



/////////////////////////////////////////////////////////////////////////////////////////////////////
// AutoHotkey: These items have been added to support doing the #Include's and FileInstall commands.
/////////////////////////////////////////////////////////////////////////////////////////////////////

// These are global rather than function-scope static so that they can be reinitialized whenever
// the user leaves the app. running and presses the "compile" button a second time:
#define LINE_SIZE (16384 + 1)  // Matches the size in the AutoHotkey source code.
#define MAX_SCRIPT_FILES 10000 // Supports less than AutoHotkey.exe, but doesn't seem likely to ever be a problem.
static int sSourceFileCount = 0;
static char *sSourceFile[MAX_SCRIPT_FILES];

char g_delimiter = ',';
char g_DerefChar = '%';
char g_EscapeChar = '`';
#define MAX_COMMENT_FLAG_LENGTH 15
static char g_CommentFlag[MAX_COMMENT_FLAG_LENGTH + 1] = ";";
static size_t g_CommentFlagLength = strlen(g_CommentFlag); // pre-calculated for performance

enum ResultType {FAIL = 0, OK, WARN = OK, CRITICAL_ERROR, CONDITION_TRUE, CONDITION_FALSE}; // FAIL must be 0.
typedef UINT LineNumberType;

#define IS_SPACE_OR_TAB(c) (c == ' ' || c == '\t')
#define IS_SPACE_OR_TAB_OR_NBSP(c) (c == ' ' || c == '\t' || c == -96) // Use a negative to support signed chars.

inline char *StrChrAny(char *aStr, char *aCharList)
// Returns the position of the first char in aStr that is of any one of
// the characters listed in aCharList.  Returns NULL if not found.
{
	if (aStr == NULL || aCharList == NULL) return NULL;
	if (!*aStr || !*aCharList) return NULL;
	// Don't use strchr() because that would just find the first occurrence
	// of the first search-char, which is not necessarily the first occurrence
	// of *any* search-char:
	char *look_for_this_char;
	for (; *aStr; ++aStr) // It's safe to use the value-parameter itself.
		// If *aStr is any of the search char's, we're done:
		for (look_for_this_char = aCharList; *look_for_this_char; ++look_for_this_char)
			if (*aStr == *look_for_this_char)
				return aStr;  // Match found.
	return NULL; // No match.
}



inline char *omit_leading_whitespace(char *aBuf)
// While aBuf points to a whitespace, moves to the right and returns the first non-whitespace
// encountered.
{
	for (; IS_SPACE_OR_TAB(*aBuf); ++aBuf);
	return aBuf;
}



char *ltrim(char *aStr)
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids
// trimming newlines because some callers want to retain those.
{
	if (!aStr || !*aStr) return aStr;
	char *ptr;
	// Find the first non-whitespace char (which might be the terminator):
	for (ptr = aStr; IS_SPACE_OR_TAB(*ptr); ++ptr);
	memmove(aStr, ptr, strlen(ptr) + 1); // +1 to include the '\0'.  memmove() permits source & dest to overlap.
	return aStr;
}

char *rtrim(char *aStr)
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids
// trimming newlines because some callers want to retain those.
{
	if (!aStr || !*aStr) return aStr; // Must do this prior to below.
	// It's done this way in case aStr just happens to be address 0x00 (probably not possible
	// on Intel & Intel-clone hardware) because otherwise --cp would decrement, causing an
	// underflow since pointers are probably considered unsigned values, which would
	// probably cause an infinite loop.  Extremely unlikely, but might as well try
	// to be thorough:
	for (char *cp = aStr + strlen(aStr) - 1; ; --cp)
	{
		if (!IS_SPACE_OR_TAB(*cp))
		{
			*(cp + 1) = '\0';
			return aStr;
		}
		if (cp == aStr)
		{
			if (IS_SPACE_OR_TAB(*cp))
				*cp = '\0';
			return aStr;
		}
	}
}

size_t rtrim_with_nbsp(char *aStr, size_t aLength = -1)
// Returns the new length of the string.
// Caller must ensure that aStr is not NULL.
// Same as rtrim but also gets rid of those annoying nbsp (non breaking space) chars that sometimes
// wind up on the clipboard when copied from an HTML document, and thus get pasted into the text
// editor as part of the code (such as the sample code in some of the examples).
{
	if (!*aStr) return 0; // The below relies upon this check having been done.
	if (aLength == -1)
		aLength = strlen(aStr); // Set aLength for use below and also as the return value.
	for (char *cp = aStr + aLength - 1; ; --cp, --aLength)
	{
		if (!IS_SPACE_OR_TAB_OR_NBSP(*cp))
		{
			cp[1] = '\0';
			return aLength;
		}
		if (cp == aStr)
		{
			if (IS_SPACE_OR_TAB_OR_NBSP(*cp))
				*cp = '\0';
			return aLength;
		}
	}
}

inline char *trim (char *aStr)
// NOTE: THIS VERSION trims only tabs and spaces.  It specifically avoids
// trimming newlines because some callers want to retain those.
{
	return ltrim(rtrim(aStr));
}



inline size_t strlcpy (char *aDst, const char *aSrc, size_t aDstSize)
// Same as strncpy() but guarantees null-termination of aDst upon return.
// No more than aDstSize - 1 characters will be copied from aSrc into aDst
// (leaving room for the zero terminator).
// This function is defined in some Unices but is not standard.  But unlike
// other versions, this one returns the number of unused chars remaining
// in aDst after the copy.
{
	if (!aDst || !aSrc || !aDstSize) return aDstSize;  // aDstSize must not be zero due to the below method.
	strncpy(aDst, aSrc, aDstSize - 1);
	aDst[aDstSize - 1] = '\0';
	return aDstSize - strlen(aDst) - 1; // -1 because the zero terminator is defined as taking up 1 space.
}



int snprintf(char *aBuf, int aBufSize, const char *aFormat, ...)
// aBufSize is an int so that any negative values passed in from caller are not lost.
// aBuf will always be terminated here except when aBufSize is <= zero (in which case the caller should
// already have terminated it).  If aBufSize is greater than zero but not large enough to hold the
// entire result, as much of the result as possible is copied and the return value is aBufSize - 1.
// Returns the exact number of characters written, not including the zero terminator.  A negative
// number is never returned, even if aBufSize is <= zero (which means there isn't even enough space left
// to write a zero terminator), under the assumption that the caller has already terminated the string
// and thus prefers to have 0 rather than -1 returned in such cases.
// MSDN says (about _snprintf(), and testing shows that it applies to _vsnprintf() too): "This function
// does not guarantee NULL termination, so ensure it is followed by sz[size - 1] = 0".
{
	// The following should probably never be changed without a full suite of tests to ensure the
	// change doesn't cause the finicky _vsnprintf() to break something.
	if (aBufSize < 1 || !aBuf || !aFormat) return 0; // It's called from so many places that the extra checks seem warranted.
	va_list ap;
	va_start(ap, aFormat);
	// Must use _vsnprintf() not _snprintf() because of the way va_list is handled:
	int result = _vsnprintf(aBuf, aBufSize, aFormat, ap); // "returns the number of characters written, not including the terminating null character, or a negative value if an output error occurs"
	aBuf[aBufSize - 1] = '\0'; // Confirmed through testing: Must terminate at this exact spot because _vsnprintf() doesn't always do it.
	// Fix for v1.0.34: If result==aBufSize, must reduce result by 1 to return an accurate result to the
	// caller.  In other words, if the line above turned the last character into a terminator, one less character
	// is now present in aBuf.
	if (result == aBufSize)
		--result;
	return result > -1 ? result : aBufSize - 1; // Never return a negative value.  See comment under function definition, above.
}



int strlicmp(char *aBuf1, char *aBuf2, UINT aLength1 = UINT_MAX, UINT aLength2 = UINT_MAX)
// Case insensitive version.  See strlcmp() comments above.
{
	if (!aBuf1 || !aBuf2) return 0;
	if (aLength1 == UINT_MAX) aLength1 = (UINT)strlen(aBuf1);
	if (aLength2 == UINT_MAX) aLength2 = (UINT)strlen(aBuf2);
	UINT least_length = aLength1 < aLength2 ? aLength1 : aLength2;
	int diff;
	for (UINT i = 0; i < least_length; ++i)
		if (   diff = (int)((UCHAR)toupper(aBuf1[i]) - (UCHAR)toupper(aBuf2[i]))   )
			return diff;
	return (int)(aLength1 - aLength2);
}	



ResultType RegReadString(HKEY aRootKey, char *aSubkey, char *aValueName, char *aBuf, size_t aBufSize)
{
	*aBuf = '\0'; // Set default output parameter.  Some callers rely on this being set even if failure occurs.
	HKEY hkey;
	if (RegOpenKeyEx(aRootKey, aSubkey, 0, KEY_QUERY_VALUE, &hkey) != ERROR_SUCCESS)
		return FAIL;
	DWORD buf_size = (DWORD)aBufSize; // Caller's value might be a constant memory area, so need a modifiable copy.
	LONG result = RegQueryValueEx(hkey, aValueName, NULL, NULL, (LPBYTE)aBuf, &buf_size);
	RegCloseKey(hkey);
	return (result == ERROR_SUCCESS) ? OK : FAIL;
}



void GetAppData(bool aGetCommon, char *aBuf)
// Caller must ensure that buf is at least MAX_PATH in size.
{
	*aBuf = '\0'; // Set default.
	if (aGetCommon)
		RegReadString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
			, "Common AppData", aBuf, MAX_PATH);
	if (!*aBuf) // Either the above failed or we were told to get the user/private dir instead.
		RegReadString(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"
			, "AppData", aBuf, MAX_PATH);
}



char *strcasestr(const char *phaystack, const char *pneedle)
	// To make this work with MS Visual C++, this version uses tolower/toupper() in place of
	// _tolower/_toupper(), since apparently in GNU C, the underscore macros are identical
	// to the non-underscore versions; but in MS the underscore ones do an unconditional
	// conversion (mangling non-alphabetic characters such as the zero terminator).  MSDN:
	// tolower: Converts c to lowercase if appropriate
	// _tolower: Converts c to lowercase

	// Return the offset of one string within another.
	// Copyright (C) 1994,1996,1997,1998,1999,2000 Free Software Foundation, Inc.
	// This file is part of the GNU C Library.

	// The GNU C Library is free software; you can redistribute it and/or
	// modify it under the terms of the GNU Lesser General Public
	// License as published by the Free Software Foundation; either
	// version 2.1 of the License, or (at your option) any later version.

	// The GNU C Library is distributed in the hope that it will be useful,
	// but WITHOUT ANY WARRANTY; without even the implied warranty of
	// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	// Lesser General Public License for more details.

	// You should have received a copy of the GNU Lesser General Public
	// License along with the GNU C Library; if not, write to the Free
	// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
	// 02111-1307 USA.

	// My personal strstr() implementation that beats most other algorithms.
	// Until someone tells me otherwise, I assume that this is the
	// fastest implementation of strstr() in C.
	// I deliberately chose not to comment it.  You should have at least
	// as much fun trying to understand it, as I had to write it :-).
	// Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de

	// Faster looping by precalculating bl, bu, cl, cu before looping.
	// 2004 Apr 08	Jose Da Silva, digital@joescat@com
{
	register const unsigned char *haystack, *needle;
	register unsigned bl, bu, cl, cu;
	
	haystack = (const unsigned char *) phaystack;
	needle = (const unsigned char *) pneedle;

	bl = tolower (*needle);
	if (bl != '\0')
	{
		// Scan haystack until the first character of needle is found:
		bu = toupper (bl);
		haystack--;				/* possible ANSI violation */
		do
		{
			cl = *++haystack;
			if (cl == '\0')
				goto ret0;
		}
		while ((cl != bl) && (cl != bu));

		// See if the rest of needle is a one-for-one match with this part of haystack:
		cl = tolower (*++needle);
		if (cl == '\0')  // Since needle consists of only one character, it is already a match as found above.
			goto foundneedle;
		cu = toupper (cl);
		++needle;
		goto jin;
		
		for (;;)
		{
			register unsigned a;
			register const unsigned char *rhaystack, *rneedle;
			do
			{
				a = *++haystack;
				if (a == '\0')
					goto ret0;
				if ((a == bl) || (a == bu))
					break;
				a = *++haystack;
				if (a == '\0')
					goto ret0;
shloop:
				;
			}
			while ((a != bl) && (a != bu));

jin:
			a = *++haystack;
			if (a == '\0')  // Remaining part of haystack is shorter than needle.  No match.
				goto ret0;

			if ((a != cl) && (a != cu)) // This promising candidate is not a complete match.
				goto shloop;            // Start looking for another match on the first char of needle.
			
			rhaystack = haystack-- + 1;
			rneedle = needle;
			a = tolower (*rneedle);
			
			if (tolower (*rhaystack) == (int) a)
			do
			{
				if (a == '\0')
					goto foundneedle;
				++rhaystack;
				a = tolower (*++needle);
				if (tolower (*rhaystack) != (int) a)
					break;
				if (a == '\0')
					goto foundneedle;
				++rhaystack;
				a = tolower (*++needle);
			}
			while (tolower (*rhaystack) == (int) a);
			
			needle = rneedle;		/* took the register-poor approach */
			
			if (a == '\0')
				break;
		} // for(;;)
	} // if (bl != '\0')
foundneedle:
	return (char*) haystack;
ret0:
	return 0;
}



char *StrReplace(char *aBuf, char *aOld, char *aNew, bool aCaseSensitive)
// Replaces first occurrence of aOld with aNew in string aBuf.  Caller must ensure that
// all parameters are non-NULL (though they can be the empty string).  It must also ensure that
// aBuf has enough allocated space for the replacement since no check is made for this.
// Finally, aOld/aNew should not be inside the same memory area as aBuf.
// The empty string ("") is found at the beginning of every string.
// Returns NULL if aOld was not found.  Otherwise, returns address of first location
// behind where aNew was inserted.  This is useful for multiple replacements (though performance
// would be bad since length of old/new and Buf has to be recalculated with strlen() for each
// call, rather than just once for all calls).
{
	// Nothing to do if aBuf is blank.  If aOld is blank, that is not supported because it
	// would be an infinite loop.
	if (!*aBuf || !*aOld)
		return NULL;
	char *found = (aCaseSensitive ? strstr(aBuf, aOld) : strcasestr(aBuf, aOld));
	if (!found)
		return NULL;
	size_t aOld_length = strlen(aOld);
	size_t aNew_length = strlen(aNew);
	char *the_part_of_aBuf_to_remain_unaltered = found + aNew_length;
	// The check below can greatly improve performance if old and new strings happen to be same length
	// (especially if this function is called thousands of times in a loop to replace multiple strings):
	if (aOld_length != aNew_length)
		// Since new string can't fit exactly in place of old string, adjust the target area to
		// accept exactly the right length so that the rest of the string stays unaltered:
		memmove(the_part_of_aBuf_to_remain_unaltered, found + aOld_length, strlen(found + aOld_length) + 1); // +1 to include the terminator.
	memcpy(found, aNew, aNew_length); // Perform the replacement.
	return the_part_of_aBuf_to_remain_unaltered;
}



int StrReplaceAllSafe(char *aBuf, size_t aBufSize, char *aOld, char *aNew, bool aCaseSensitive)
// Similar to above but checks to ensure that the size of the buffer isn't exceeded.
// Returns how many replacements were done.
{
	// Nothing to do if aBuf is blank.  If aOld is blank, that is not supported because it
	// would be an infinite loop.
	if (!*aBuf || !*aOld)
		return 0;
	char *ptr;
	int replacment_count;
	int length_increase = (int)(strlen(aNew) - strlen(aOld));  // Can be negative.
	for (replacment_count = 0, ptr = aBuf;; )
	{
		if (length_increase > 0) // Make sure there's enough room in aBuf first.
			if ((int)(aBufSize - strlen(aBuf) - 1) < length_increase)
				break;  // Not enough room to do the next replacement.
		if (   !(ptr = StrReplace(ptr, aOld, aNew, aCaseSensitive))   )
			break;
		// Otherwise, it did actually replace one item, so increment:
		++replacment_count;
	}
	return replacment_count;
}



// Small inline to make LoadIncludedFile() code cleaner.
inline ResultType CloseAndReturn(FILE *fp, UCHAR *aBuf, ResultType aReturnValue)
{
	// aBuf is unused in this case.
	fclose(fp);
	return aReturnValue;
}



void SetWorkingDir(char *aNewDir)
// This function was added in v1.0.45.01 for the reasons commented further below.
// It's similar to the one in the AutoHotkey.exe source, so maintain them together.
{
	if (!SetCurrentDirectory(aNewDir)) // Caused by nonexistent directory, permission denied, etc.
		return;

	// Otherwise, the change to the working directory *apparently* succeeded (but is confirmed below for root drives
	// and also because we want the absolute path in cases where aNewDir is relative).
	char actual_working_dir[MAX_PATH];

	// GetCurrentDirectory() is called explicitly, to confirm the change, in case aNewDir is a relative path.
	// We want to store the absolute path:
	if (!GetCurrentDirectory(sizeof(actual_working_dir), actual_working_dir)) // Might never fail in this case, but kept for backward compatibility.
		return;

	// Otherwise, GetCurrentDirectory() succeeded, so it's appropriate to compare what we asked for to what
	// was received.
	if (aNewDir[0] && aNewDir[1] == ':' && !aNewDir[2] // Root with missing backslash. Relies on short-circuit boolean order.
		&& _stricmp(aNewDir, actual_working_dir)) // The root directory we requested didn't actually get set. See below.
	{
		// There is some strange OS behavior here: If the current working directory is C:\anything\...
		// and SetCurrentDirectory() is called to switch to "C:", the function reports success but doesn't
		// actually change the directory.  However, if you first change to D: then back to C:, the change
		// works as expected.  Presumably this is for backward compatibility with DOS days; but it's 
		// inconvenient and seems desirable to override it in this case, especially because:
		// v1.0.45.01: Since A_ScriptDir omits the trailing backslash for roots of drives (such as C:),
		// and since that variable probably shouldn't be changed for backward compatibility, provide
		// the missing backslash to allow SetWorkingDir %A_ScriptDir% (and others) to work in the root
		// of a drive.
		char buf_temp[8];
		sprintf(buf_temp, "%s\\", aNewDir); // No danger of buffer overflow in this case.
		SetCurrentDirectory(buf_temp); // Failure doesn't matter much because the origial Set() higher above succeeded. So for simplicity, do nothing on failure.
	}
}



//
// CODE BASED ON AUTOHOTKEY
//

#define MAX_VAR_NAME_LENGTH (UCHAR_MAX - 2)

struct FuncLibrary
{
	LPSTR path;
	DWORD_PTR length;
};

LPSTR FindFuncInLibrary(LPSTR aFuncName, size_t aFuncNameLength)
// Caller must ensure that aFuncName doesn't already exist as a defined function.
// If aFuncNameLength is 0, the entire length of aFuncName is used.
{
	int i;
	LPSTR last_backslash, terminate_here;
	DWORD attr;

	#define FUNC_LIB_EXT ".ahk"
	#define FUNC_LIB_EXT_LENGTH (_countof(FUNC_LIB_EXT) - 1)
	#define FUNC_LOCAL_LIB "\\Lib\\" // Needs leading and trailing backslash.
	#define FUNC_LOCAL_LIB_LENGTH (_countof(FUNC_LOCAL_LIB) - 1)
	#define FUNC_USER_LIB "\\AutoHotkey\\Lib\\" // Needs leading and trailing backslash.
	#define FUNC_USER_LIB_LENGTH (_countof(FUNC_USER_LIB) - 1)
	#define FUNC_STD_LIB "Lib\\" // Needs trailing but not leading backslash.
	#define FUNC_STD_LIB_LENGTH (_countof(FUNC_STD_LIB) - 1)

	#define FUNC_LIB_COUNT 3
	static FuncLibrary sLib[FUNC_LIB_COUNT] = {0};

	if (!sLib[0].path) // Allocate & discover paths only upon first use because many scripts won't use anything from the library. This saves a bit of memory and performance.
	{
		for (i = 0; i < FUNC_LIB_COUNT; ++i)
			if (   !(sLib[i].path = (LPSTR) malloc(MAX_PATH))   ) // Need MAX_PATH for to allow room for appending each candidate file/function name.
				return NULL; // Due to rarity, simply pass the failure back to caller.

		FuncLibrary *this_lib;

		// DETERMINE PATH TO "LOCAL" LIBRARY:
		this_lib = sLib; // For convenience and maintainability.
		this_lib->length = strlen(g_ScriptDir);
		if (this_lib->length < MAX_PATH-FUNC_LOCAL_LIB_LENGTH)
		{
			strcpy(this_lib->path, g_ScriptDir);
			strcpy(this_lib->path + this_lib->length, FUNC_LOCAL_LIB);
			this_lib->length += FUNC_LOCAL_LIB_LENGTH;
		}
		else // Insufficient room to build the path name.
		{
			*this_lib->path = '\0'; // Mark this library as disabled.
			this_lib->length = 0;   //
		}

		// DETERMINE PATH TO "USER" LIBRARY:
		this_lib++; // For convenience and maintainability.
		if ( SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, this_lib->path) == S_OK
			&& (this_lib->length = strlen(this_lib->path)) < MAX_PATH-FUNC_USER_LIB_LENGTH )
		{
			strcpy(this_lib->path + this_lib->length, FUNC_USER_LIB);
			this_lib->length += FUNC_USER_LIB_LENGTH;
		}
		else // Insufficient room to build the path name.
		{
			*this_lib->path = '\0'; // Mark this library as disabled.
			this_lib->length = 0;   //
		}

		// DETERMINE PATH TO "STANDARD" LIBRARY:
		this_lib++; // For convenience and maintainability.
		GetModuleFileName(NULL, this_lib->path, MAX_PATH); // The full path to the currently-running AutoHotkey.exe.
		last_backslash = strrchr(this_lib->path, '\\'); // Should always be found, so failure isn't checked.
		*last_backslash = '\0';
		if ( (last_backslash = strrchr(this_lib->path, '\\'))
			&& (this_lib->length = last_backslash + 1 - this_lib->path) < MAX_PATH-FUNC_STD_LIB_LENGTH )
		{
			strcpy(this_lib->path + this_lib->length, FUNC_STD_LIB);
			this_lib->length += FUNC_STD_LIB_LENGTH;
		}
		else // Insufficient room to build the path name.
		{
			*this_lib->path = '\0'; // Mark this library as disabled.
			this_lib->length = 0;   //
		}

		for (i = 0; i < FUNC_LIB_COUNT; ++i)
		{
			attr = GetFileAttributes(sLib[i].path); // Seems to accept directories that have a trailing backslash, which is good because it simplifies the code.
			if (attr == 0xFFFFFFFF || !(attr & FILE_ATTRIBUTE_DIRECTORY)) // Directory doesn't exist or it's a file vs. directory. Relies on short-circuit boolean order.
			{
				*sLib[i].path = '\0'; // Mark this library as disabled.
				sLib[i].length = 0;   //
			}
		}
	}
	// Above must ensure that all sLib[].path elements are non-NULL (but they can be "" to indicate "no library").

	if (!aFuncNameLength) // Caller didn't specify, so use the entire string.
		aFuncNameLength = strlen(aFuncName);

	CHAR *dest, *first_underscore, class_name_buf[MAX_VAR_NAME_LENGTH + 1];
	LPTSTR naked_filename = aFuncName;               // Set up for the first iteration.
	size_t naked_filename_length = aFuncNameLength; //

	for (int second_iteration = 0; second_iteration < 2; ++second_iteration)
	{
		for (i = 0; i < FUNC_LIB_COUNT; ++i)
		{
			if (!*sLib[i].path) // Library is marked disabled, so skip it.
				continue;

			if (sLib[i].length + naked_filename_length >= MAX_PATH-FUNC_LIB_EXT_LENGTH)
				continue; // Path too long to match in this library, but try others.
			dest = (LPSTR) memcpy(sLib[i].path + sLib[i].length, naked_filename, naked_filename_length); // Append the filename to the library path.
			strcpy(dest + naked_filename_length, FUNC_LIB_EXT); // Append the file extension.

			attr = GetFileAttributes(sLib[i].path); // Testing confirms that GetFileAttributes() doesn't support wildcards; which is good because we want filenames containing question marks to be "not found" rather than being treated as a match-pattern.
			if (attr == 0xFFFFFFFF || (attr & FILE_ATTRIBUTE_DIRECTORY)) // File doesn't exist or it's a directory. Relies on short-circuit boolean order.
				continue;

			// Since above didn't "continue", a file exists whose name matches that of the requested function.
			// Before loading/including that file, set the working directory to its folder so that if it uses
			// #Include, it will be able to use more convenient/intuitive relative paths.  This is similar to
			// the "#Include DirName" feature.
			// Call SetWorkingDir() vs. SetCurrentDirectory() so that it succeeds even for a root drive like
			// C: that lacks a backslash (see SetWorkingDir() for details).
			terminate_here = sLib[i].path + sLib[i].length - 1; // The trailing backslash in the full-path-name to this library.
			*terminate_here = '\0'; // Temporarily terminate it for use with SetWorkingDir().
			SetWorkingDir(sLib[i].path); // See similar section in the #Include directive.
			*terminate_here = '\\'; // Undo the termination.

			return sLib[i].path;

		} // for() each library directory.

		// Now that the first iteration is done, set up for the second one that searches by class/prefix.
		// Notes about ambiguity and naming collisions:
		// By the time it gets to the prefix/class search, it's almost given up.  Even if it wrongly finds a
		// match in a filename that isn't really a class, it seems inconsequential because at worst it will
		// still not find the function and will then say "call to nonexistent function".  In addition, the
		// ability to customize which libraries are searched is planned.  This would allow a publicly
		// distributed script to turn off all libraries except stdlib.
		if (   !(first_underscore = strchr(aFuncName, '_'))   ) // No second iteration needed.
			break; // All loops are done because second iteration is the last possible attempt.
		naked_filename_length = first_underscore - aFuncName;
		if (naked_filename_length >= _countof(class_name_buf)) // Class name too long (probably impossible currently).
			break; // All loops are done because second iteration is the last possible attempt.
		naked_filename = class_name_buf; // Point it to a buffer for use below.
		memcpy(naked_filename, aFuncName, naked_filename_length);
		naked_filename[naked_filename_length] = '\0';
	} // 2-iteration for().

	// Since above didn't return, no match found in any library.
	return NULL;
}

//
// END CODE BASED ON AUTOHOTKEY
//



// Prototype to allow mutual recursion:
ResultType LoadIncludedFile(FILE *aTarget, HWND aStatusBar, EXEArc_Write &oWrite
	, char *aFileSpec, bool aAllowDuplicateInclude, bool aIgnoreLoadFailure);
inline ResultType IsPreprocessorDirective(FILE *aTarget, HWND aStatusBar, EXEArc_Write &oWrite
	, char *aBuf, bool &aIsInclude)
// THIS ONLY HAS A SUBSET of the preprocessor directives.  The others do not need to be handled
// until load-time (when the EXE is run), and thus are handled in the main source code and are
// just ignored here (no attempt is made to report errors for misspelled directives, for example).
// aBuf must be a modifiable string since this function modifies it in the case of "#Include %A_ScriptDir%"
// changes it.  It must also be large enough to accept the replacement of %A_ScriptDir% with a larger string.
// Returns CONDITION_TRUE, CONDITION_FALSE, or FAIL.
// Note: Don't assume that every line in the script that starts with '#' is a directive
// because hotkeys can legitimately start with that as well.  i.e., the following line should
// not be unconditionally ignored, just because it starts with '#', since it is a valid hotkey:
// #y::run, notepad
{
	char end_flags[] = {' ', '\t', g_delimiter, '\0'}; // '\0' must be last.
	char *cp, *directive_end;

	aIsInclude = false;  // Set default;

	// Use strnicmp() so that a match is found as long as aBuf starts with the string in question.
	// e.g. so that "#SingleInstance, on" will still work too, but
	// "#a::run, something, "#SingleInstance" (i.e. a hotkey) will not be falsely detected
	// due to using a more lenient function such as strcasestr().  UPDATE: Using strlicmp() now so
	// that overlapping names, such as #MaxThreads and #MaxThreadsPerHotkey won't get mixed up:
	if (   !(directive_end = StrChrAny(aBuf, end_flags))   )
		directive_end = aBuf + strlen(aBuf); // Point it to the zero terminator.
	#define IS_DIRECTIVE_MATCH(directive) (!strlicmp(aBuf, directive, (UINT)(directive_end - aBuf)))

	// For the below series, it seems okay to allow the comment flag to contain other reserved chars,
	// such as DerefChar, since comments are evaluated, and then taken out of the game at an earlier
	// stage than DerefChar and the other special chars.
	if (IS_DIRECTIVE_MATCH("#CommentFlag"))
	{
		#define RETURN_IF_NO_CHAR \
		if (!*directive_end)\
			return CONDITION_TRUE;\
		if (   !*(cp = omit_leading_whitespace(directive_end))   )\
			return CONDITION_TRUE;\
		if (*cp == g_delimiter)\
		{\
			++cp;\
			if (   !*(cp = omit_leading_whitespace(cp))   )\
				return CONDITION_TRUE;\
		}
		RETURN_IF_NO_CHAR
		if (!*(cp + 1))  // i.e. the length is 1
		{
			// Don't allow '#' since it's the preprocessor directive symbol being used here.
			// Seems ok to allow "." to be the comment flag, since other constraints mandate
			// that at least one space or tab occur to its left for it to be considered a
			// comment marker.
			if (*cp == '#' || *cp == g_DerefChar || *cp == g_EscapeChar || *cp == g_delimiter)
				return CONDITION_TRUE;  // Just ignore errors like this, letting the main program catch them instead.
			// Exclude hotkey definition chars, such as ^ and !, because otherwise
			// the following example wouldn't work:
			// User defines ! as the comment flag.
			// The following hotkey would never be in effect since it's considered to
			// be commented out:
			// !^a::run,notepad
			if (*cp == '!' || *cp == '^' || *cp == '+' || *cp == '$' || *cp == '~' || *cp == '*'
				|| *cp == '<' || *cp == '>')
				// Note that '#' is already covered by the other stmt. above.
				return CONDITION_TRUE;  // Just ignore errors like this, letting the main program catch them instead.
		}
		strlcpy(g_CommentFlag, cp, MAX_COMMENT_FLAG_LENGTH + 1);
		g_CommentFlagLength = strlen(g_CommentFlag);  // Keep this in sync with above.
		return CONDITION_TRUE;
	}
	if (IS_DIRECTIVE_MATCH("#EscapeChar"))
	{
		RETURN_IF_NO_CHAR
		// Don't allow '.' since that can be part of literal floating point numbers:
		if (   *cp == '#' || *cp == g_DerefChar || *cp == g_delimiter || *cp == '.'
			|| (g_CommentFlagLength == 1 && *cp == *g_CommentFlag)   )
			return CONDITION_TRUE;  // Just ignore errors like this, letting the main program catch them instead.
		g_EscapeChar = *cp;
		return CONDITION_TRUE;
	}
	if (IS_DIRECTIVE_MATCH("#DerefChar"))
	{
		RETURN_IF_NO_CHAR
		if (   *cp == '#' || *cp == g_EscapeChar || *cp == g_delimiter || *cp == '.'
			|| (g_CommentFlagLength == 1 && *cp == *g_CommentFlag)   )
			return CONDITION_TRUE;  // Just ignore errors like this, letting the main program catch them instead.
		g_DerefChar = *cp;
		return CONDITION_TRUE;
	}
	if (IS_DIRECTIVE_MATCH("#Delimiter"))
	{
		// This macro will skip over any leading delimiter than may be present, e.g. #Delimiter, ^
		// This should be okay since the user shouldn't be attempting to change the delimiter
		// to what it already is, and even if this is attempted, it would just be ignored:
		RETURN_IF_NO_CHAR
		if (   *cp == '#' || *cp == g_EscapeChar || *cp == g_DerefChar || *cp == '.'
			|| (g_CommentFlagLength == 1 && *cp == *g_CommentFlag)   )
			return CONDITION_TRUE;  // Just ignore errors like this, letting the main program catch them instead.
		g_delimiter = *cp;
		return CONDITION_TRUE;
	}

	bool include_again = false; // Set default in case of short-circuit boolean.
	if (IS_DIRECTIVE_MATCH("#Include") || (include_again = IS_DIRECTIVE_MATCH("#IncludeAgain")))
	{
		aIsInclude = true;
		if (   !*(cp = omit_leading_whitespace(directive_end))   )
			return CONDITION_TRUE;  // Just ignore these types of rare syntax errors.
		if (*cp == g_delimiter)
		{
			++cp;
			if (   !*(cp = omit_leading_whitespace(cp))   )
				return CONDITION_TRUE;  // Just ignore these types of rare syntax errors.
		}
		// v1.0.32:
		bool ignore_load_failure = (cp[0] == '*' && toupper(cp[1]) == 'I'); // Relies on short-circuit boolean order.
		if (ignore_load_failure)
		{
			cp += 2;
			if (IS_SPACE_OR_TAB(*cp)) // Skip over at most one space or tab, since others might be a literal part of the filename.
				++cp;
		}

		if (cp[0] == '<')
		{
			char *cp_end = strchr(cp + 1, '>');
			if (cp_end && !cp_end[1])
			{
				char *lib_file = FindFuncInLibrary(cp + 1, cp_end - (cp + 1));
				if (lib_file)
					return (LoadIncludedFile(aTarget, aStatusBar, oWrite, lib_file, include_again, ignore_load_failure) == FAIL) ? FAIL : CONDITION_TRUE;  // It will have already displayed any error.
			}
		}
		// The following section exists in ahk2exe but not the main program because the main program's
		// GetLine() method resolves escaped semicolon (`;) to be semicolon prior to this stage.  That
		// same thing can't be done by this one's GetLine() because it would cause the compressed script
		// to get written without the escape chars that the SC version needs to see when it loads the script.
		// Although the #Include line itself will be commented out inside the compiled script (since it's
		// contents will be directly substituted), it's more correct to write it out with any escaped
		// semicolons intact in case anyone decompiles the script and uncomments the line to get things back.
		// The same holds true for replacement of %A_ScriptDir%: Do it only internally, not in the actual
		// text that will be written into the compiled script. Therefore, make a copy:
		char filename[MAX_PATH];
		strlcpy(filename, cp, MAX_PATH);
		char escaped_comment_flag[MAX_COMMENT_FLAG_LENGTH + 2]; // +1 for terminator and +1 for escape char.
		*escaped_comment_flag = g_EscapeChar;
		strcpy(escaped_comment_flag + 1, g_CommentFlag);
		StrReplaceAllSafe(filename, MAX_PATH, escaped_comment_flag, g_CommentFlag, true); // Case sensitive. "Safe" mode isn't necessary but is used because that function is a smaller amount of code to include.
		// The above isn't exactly correct because it prevents an include filename from ever containing the
		// literal string "`;".  This is because attempts to escape the accent via "``;" are not supported.
		// This is documented here as a known limitation because fixing it would probably break existing
		// scripts that rely on the fact that accents do not need to be escaped inside #Include.  Also,
		// the likelihood of "`;" appearing literally in a legitimate #Include file seems vanishingly small.

		StrReplace(filename, "%A_ScriptDir%", g_ScriptDir, false); // v1.0.35.11.  Maximum of one replacement. Caller has ensured string is writable.
		char buf[MAX_PATH];
		if (strcasestr(filename, "%A_AppData%"))  // v1.0.45.04: This and the next were requested by Tekl to make it easier to customize scripts on a per-user basis.
		{
			GetAppData(false, buf);
			StrReplace(filename, "%A_AppData%", buf, false); // Overflow is too unlikely, so it isn't checked.
		}
		if (strcasestr(filename, "%A_AppDataCommon%")) // v1.0.45.04.
		{
			GetAppData(true, buf);
			StrReplace(filename, "%A_AppDataCommon%", buf, false); // Overflow is too unlikely, so it isn't checked.
		}
		DWORD attr = GetFileAttributes(filename);
		if (attr != 0xFFFFFFFF && (attr & FILE_ATTRIBUTE_DIRECTORY)) // File exists and its a directory (possibly A_ScriptDir set above).
		{
			// v1.0.35.11 allow changing of load-time directory to increase flexibility.  This feature has
			// been asked for directly or indirectly several times.
			// If a filename every wants to use the string "%A_ScriptDir%" literally in an include's filename,
			// that would not work.  But that seems too rare to worry about.
			// v1.0.45.01: Call SetWorkingDir() vs. SetCurrentDirectory() so that it succeeds even for a root
			// drive like C: that lacks a backslash (see SetWorkingDir() for details).
			SetWorkingDir(filename);
			return CONDITION_TRUE;
		}
		// Since above didn't return, it's a file (or non-existent file, in which case the below will display
		// the error).
		return (LoadIncludedFile(aTarget, aStatusBar, oWrite, filename, include_again, ignore_load_failure) == FAIL) ? FAIL : CONDITION_TRUE;  // It will have already displayed any error.
	}

	// Otherwise:
	return CONDITION_FALSE;
}



size_t GetLine(char *aBuf, int aMaxCharsToRead, FILE *fp, bool aInContinuationSection)
{
	size_t aBuf_length = 0;
	if (!aBuf || !fp) return -1;
	if (aMaxCharsToRead <= 0) return 0;
	if (feof(fp)) return -1; // Previous call to this function probably already read the last line.
	if (fgets(aBuf, aMaxCharsToRead, fp) == NULL) // end-of-file or error
	{
		*aBuf = '\0';  // Reset since on error, contents added by fgets() are indeterminate.
		return -1;
	}
	aBuf_length = strlen(aBuf);
	if (!aBuf_length)
		return 0;
	if (aBuf[aBuf_length-1] == '\n')
		aBuf[--aBuf_length] = '\0';
	if (aBuf[aBuf_length-1] == '\r')  // In case there are any, e.g. a Macintosh or Unix file?
		aBuf[--aBuf_length] = '\0';

	// v1.0.32: Trimming and omission of comments is done only when this line isn't inside a
	// continuation section.  This is because things that look like comments really aren't
	// when inside a continuation section.
	if (!aInContinuationSection)
	{
		rtrim(aBuf); // No ltrim() so that leading whitespace is retained in case of decompile.
		if (!strncmp(aBuf, g_CommentFlag, g_CommentFlagLength)) // Case sensitive.
		{
			*aBuf = '\0';
			return 0;
		}
		
		// These are always allowed, since compiling of .aut scripts is not supported:
		//if (g_AllowSameLineComments)
		//{
		char *cp, *prevp;
		for (cp = strstr(aBuf, g_CommentFlag); cp; cp = strstr(cp + g_CommentFlagLength, g_CommentFlag))
		{
			// If no whitespace to its left, it's not a valid comment.
			// We insist on this so that a semi-colon (for example) immediately after
			// a word (as semi-colons are often used) will not be considered a comment.
			prevp = cp - 1;
			if (prevp < aBuf) // should never happen because we already checked above.
			{
				*aBuf = '\0';
				return 0;
			}
			if (IS_SPACE_OR_TAB_OR_NBSP(*prevp)) // consider it to be a valid comment flag
			{
				*prevp = '\0';
				rtrim_with_nbsp(aBuf, prevp - aBuf); // Since it's our responsibility to return a fully trimmed string.
				break; // Once the first valid comment-flag is found, nothing after it can matter.
			}
			// Must not do the following because it would remove the escape character from the line
			// being saved into the compressed script.  Then when the script is executed by the SC version,
			// it would see that there is no escape character anymore and treat it as a comment:
			//else // No whitespace to the left.
			//	if (*prevp == g_EscapeChar) // Remove the escape char.
			//	{
			//		memmove(prevp, prevp + 1, strlen(prevp + 1) + 1);  // +1 for the terminator.
			//		// Then continue looking for others.
			//	}
				// else there wasn't any whitespace to its left, so keep looking in case there's
				// another further on in the line.
		}
	}

	return strlen(aBuf);  // Return an updated length due to trim().
}



void ConvertEscapeSequences(char *aBufToConvert, char *aLiteralMap)
{
	// Caller has ensured that aLiteralMap is at least LINE_SIZE in size.
	ZeroMemory(aLiteralMap, LINE_SIZE);  // Must be fully zeroed for this purpose.
	char c;
	int i;
	for (i = 0; ; ++i)  // Increment to skip over the symbol just found by the inner for().
	{
		for (; aBufToConvert[i] && aBufToConvert[i] != g_EscapeChar; ++i);  // Find the next escape char.
		if (!aBufToConvert[i]) // end of string.
			break;
		c = aBufToConvert[i + 1];
		switch (c)
		{
			case 'a': aBufToConvert[i + 1] = '\a'; break;  // alert (bell) character
			case 'b': aBufToConvert[i + 1] = '\b'; break;  // backspace
			case 'f': aBufToConvert[i + 1] = '\f'; break;  // formfeed
			case 'n': aBufToConvert[i + 1] = '\n'; break;  // newline
			case 'r': aBufToConvert[i + 1] = '\r'; break;  // carriage return
			case 't': aBufToConvert[i + 1] = '\t'; break;  // horizontal tab
			case 'v': aBufToConvert[i + 1] = '\v'; break;  // vertical tab
		}
		MoveMemory(aBufToConvert + i, aBufToConvert + i + 1, strlen(aBufToConvert + i + 1) + 1);
		aLiteralMap[i] = 1;  // In the map, mark this char as literal.
	}
}



ResultType LoadIncludedFile(FILE *aTarget, HWND aStatusBar, EXEArc_Write &oWrite
	, char *aFileSpec, bool aAllowDuplicateInclude, bool aIgnoreLoadFailure)
// Returns the number of non-comment lines that were loaded, or LOADING_FAILED on error.
#define HOTKEY_FLAG "::"
{
	if (!aFileSpec || !*aFileSpec) return FAIL;
	if (sSourceFileCount >= MAX_SCRIPT_FILES)
	{
		Util_ShowError("Too many included files.");
		return FAIL;
	}

	// Keep this var on the stack due to recursion, which allows newly created lines to be given the
	// correct file number even when some #include's have been encountered in the middle of the script:
	int source_file_index = sSourceFileCount;

	// Get the full path in case aFileSpec has a relative path.  This is done so that duplicates
	// can be reliably detected (we only want to avoid including a given file more than once):
	char full_path[MAX_PATH];
	char *filename_marker;
	GetFullPathName(aFileSpec, sizeof(full_path) - 1, full_path, &filename_marker);
	// Check if this file was already included.  If so, it's not an error because we want
	// to support automatic "include once" behavior.  So just ignore repeats:
	if (!aAllowDuplicateInclude)
		for (int f = 0; f < source_file_index; ++f)  // Here, source_file_index==sSourceFileCount
			if (!_stricmp(sSourceFile[f], full_path)) // Case insensitive like the file system.
				return OK;

	UCHAR *script_buf = NULL;  // Init for the case when the buffer isn't used (non-standalone mode).
	ULONG nDataSize = 0;

	// <buf> should be no larger than LINE_SIZE because some later functions rely upon that:
	char buf[LINE_SIZE], *file_install_pos, *action_end, *action_end_fi, *cp;
	size_t buf_length, is_label;

	FILE *fp = fopen(aFileSpec, "r");
	if (!fp)
	{
		if (aIgnoreLoadFailure)
			return OK;
		char msg_text[MAX_PATH + 256];
		snprintf(msg_text, sizeof(msg_text), "%s file \"%s\" cannot be opened." // -1 since that's how _snprintf() operates.
			, sSourceFileCount > 0 ? "#include" : "Script", aFileSpec);
		Util_ShowError(msg_text);
		return FAIL;
	}
	// v1.0.40.11: Otherwise, check if the first three bytes of the file are the UTF-8 BOM marker (and if
	// so omit them from further consideration).  Apps such as Notepad, WordPad, and Word all insert this
	// marker if the file is saved in UTF-8 format.
	// NOTE: To save code size, any UTF-8 BOM bytes completely omitted because our caller may already have
	// written some data to the output file (such as a comment indicating compiler version), and thus writing
	// the UTF-8 BOM marker would incorrectly put it after the the comment line.  Doing it here also saves a
	// little code size and slightly improves performance because the main program (AutoHotkeySC.bin) doesn't
	// have to check for the marker.
	// This section also serves to strip out UTF-8 BOM markers in #Include files since otherwise they would
	// incorrectly be written in the middle of the file.
	if (fgets(buf, 4, fp)) // Success (the fourth character is the terminator).
	{
		if (strcmp(buf, "﻿"))  // UTF-8 BOM marker is NOT present.
			rewind(fp);  // Go back to the beginning so that the first three bytes aren't omitted during loading.
	}
	//else file read error or EOF, let a later section handle it.

	// This is done only after the file has been successfully opened in case aIgnoreLoadFailure==true:
	if (sSourceFile[source_file_index] = _strdup(full_path)) // _strdup() is very tiny and basically just calls strlen+malloc+strcpy.
		++sSourceFileCount;
	//else out of memory. Due to extreme rarity, simply don't store it.

	// File is now open, read lines from it.

	UINT action_name_length;
	bool in_comment_section = false, in_continuation_section = false;

	LineNumberType line_number = 0;

	for (;;)
	{
		if (   -1 == (buf_length = GetLine(buf, (int)(sizeof(buf) - 1), fp, in_continuation_section))   )
			break;

		++line_number; // A local copy on the stack to help with recursion.

		if (!buf_length)
		{
			// Seems best to put a blank line in place of comments and genuine blank lines,
			// in case the script is ever decompiled:
			fputc('\n', aTarget);
			continue;
		}

		char *line_start = omit_leading_whitespace(buf); // ahk2exe
		if (in_comment_section) // Look for the uncomment-flag.
		{
			if (!strncmp(line_start, "*/", 2))
			{
				in_comment_section = false;
				memmove(line_start, line_start + 2, strlen(line_start) - 2 + 1);  // +1 to include the string terminator.
				ltrim(line_start); // Get rid of any whitespace that was between the comment-end and remaining text.
				if (!*line_start) // The rest of the line is empty, so it was just a naked comment-end.
					continue;
				buf_length = strlen(buf);
			}
			else
				continue;
		}
		else // Not currently inside a comment section, so it is valid to check for start/end of a continuation section.
		{
			// v1.0.32: Otherwise, check if it's the start of a continuation section so that anything
			// that appears to be a comment inside it will be retained rather than omitted.
			if (in_continuation_section) // Note that within a continuation section, lines that start with open-parenthesis are treated as literal, not as nested parentheses.
			{
				if (*line_start == ')') // Note that an escaped close-parenthesis will be properly detected as a literal and not the end of this continuation section.
					in_continuation_section = false;
				// Due to the extreme rarity of use, the following is not supported because it doesn't seem worth
				// the cost in terms of code complexity (it would be difficult to detect reliably whether the word
				// FileInstall is literal or a command in such cases):
				// Any FileInstall command that occurs after the close-parenthesis.  For example:
				// IfWinActive,
				// (
				// WinTitle
				// ),, FileInstall, ...
				fputs(buf, aTarget);
				fputc('\n', aTarget);
				continue; // Don't want the section further below to process FileInstall and other words because they should be literal inside a continuation section.
			}
			else // not in a continuation section, so check if this line is the start of a continuation section.
			{
				if (*line_start == '(')
				{
					in_continuation_section = true;
					fputs(buf, aTarget);
					fputc('\n', aTarget);
					continue;
				}
				else if (!strncmp(line_start, "/*", 2))
				{
					in_comment_section = true;
					continue; // It's now commented out, so the rest of this line is ignored.
				}
				//else do nothing.
			}
		}

		// Since the above didn't continue, we're not currently inside a continuation section.

		// PARSE LABELS TOO SO THAT A LABEL SUCH AS "#Label:" can still work even though
		// it starts with the precompiler symbol:
		// Note that there may be an action following the HOTKEY_FLAG (on the same line).
		if (strstr(buf, HOTKEY_FLAG)) // It's a label and a hotkey.
		{
			fputs(buf, aTarget);
			fputc('\n', aTarget);
			continue;
		}

		// Otherwise, not a hotkey.  Check if it's a generic, non-hotkey label:
		if (buf[buf_length - 1] == ':') // Labels must end in a colon (buf was previously rtrimmed).
		{
			//if (buf_length == 1) // v1.0.41.01: Properly handle the fact that this line consists of only a colon.
			//{
			//	ScriptError(ERR_UNRECOGNIZED_ACTION, buf);
			//	return CloseAndReturn(fp, script_buf, FAIL);
			//}

			// Labels (except hotkeys) must contain no whitespace, delimiters, or escape-chars.
			// This is to avoid problems where a legitimate action-line ends in a colon,
			// such as "WinActivate SomeTitle" and "#Include c:".
			// We allow hotkeys to violate this since they may contain commas, and since a normal
			// script line (i.e. just a plain command) is unlikely to ever end in a double-colon:
			for (cp = buf, is_label = true; *cp; ++cp)
				if (IS_SPACE_OR_TAB(*cp) || *cp == g_delimiter || *cp == g_EscapeChar)
				{
					is_label = false;
					break;
				}
			if (is_label) // It's a generic, non-hotkey/non-hotstring label.
			{
				fputs(buf, aTarget);
				fputc('\n', aTarget);
				continue; // The checking isn't as rigorous as in the main source code.
			}
		}

		// It's not a label.
		if (*line_start == '#')
		{
			bool is_include = false;
			switch(IsPreprocessorDirective(aTarget, aStatusBar, oWrite, line_start, is_include))
			{
			case CONDITION_TRUE:
				if (is_include)
				{
					if (!g_OmitFurtherIncludeComments)
					{
						// Comment out the #include since it was already processed by us (i.e. don't process it again
						// at runtime, but retain the #include as a comment in case script is ever decompiled):
						fputs(g_CommentFlag, aTarget);
						fputc(' ', aTarget);
						fputs(buf, aTarget);
						fputc('\n', aTarget);
					}
				}
				else
				{
					fputs(buf, aTarget);
					fputc('\n', aTarget);
				}
				continue;
			case FAIL:
				return CloseAndReturn(fp, script_buf, FAIL); // It already reported the error.
			// Otherwise it's CONDITION_FALSE.  Do nothing.
			}
		}
		// Otherwise it's just a normal script line.

		// Since FileInstall commands can appear on the same line as an IF command, need a
		// more complex detection method.  Although this method should work 99.99% of the time,
		// it might falsely detect a FileInstall in rare cases such as this:
		// IfMsgBox, Yes, MsgBox, FileInstall is a word used in a literal string.
		// However, even if it does it seems fairly inconsequently in most/all cases since
		// the command will simply give an error when it tries to include the file since 
		// the cmd's source file won't be found in the file system.  To workaround this,
		// the user can put the string FileInstall into a variable rather than using it
		// literally.  But keep in mind such a thing is expected to extremely rarely if at all.
		action_end_fi = NULL;  // The pos of the first char after the FileInstall.  Set default.
		if (file_install_pos = strcasestr(line_start, "FileInstall")) // Assign
		{
			// Change this value EVERY TIME in case g_delimiter changed dynamically during load:
			char end_flags[] = {' ', '\t', g_delimiter, '\0'}; // '\0' must be last.
			if (file_install_pos > line_start) // "FileInstall" is not the first phrase in the line.
			{
				if (action_end = StrChrAny(line_start, end_flags)) // Assign
				{
					action_name_length = (UINT)(action_end - line_start);
					// Check if it's on the same line as an if-command that allows such.
					// strlicmp() is used vs. strcasestr() to enforce that a qualified command
					// must occur as the very first phrase in the line:
					if (strlicmp(line_start, "IfEqual", action_name_length))
					if (strlicmp(line_start, "IfNotEqual", action_name_length))
					if (strlicmp(line_start, "IfGreater", action_name_length))
					if (strlicmp(line_start, "IfGreaterOrEqual", action_name_length))
					if (strlicmp(line_start, "IfLess", action_name_length))
					if (strlicmp(line_start, "IfLessOrEqual", action_name_length))
					if (strlicmp(line_start, "IfWinExist", action_name_length))
					if (strlicmp(line_start, "IfWinNotExist", action_name_length))
					if (strlicmp(line_start, "IfWinActive", action_name_length))
					if (strlicmp(line_start, "IfWinNotActive", action_name_length))
					if (strlicmp(line_start, "IfInString", action_name_length))
					if (strlicmp(line_start, "IfNotInString", action_name_length))
					if (strlicmp(line_start, "IfExist", action_name_length))
					if (strlicmp(line_start, "IfNotExist", action_name_length))
					if (strlicmp(line_start, "IfMsgBox", action_name_length))
						file_install_pos = NULL; // The command is not one that permits other cmds on the same line.
				}
			}
			if (file_install_pos)
			{
				if (action_end_fi = StrChrAny(file_install_pos, end_flags)) // Assign
				{
					// Now action_end_fi is position of the space, tab, or delimiter following the command name.
					action_end_fi = omit_leading_whitespace(action_end_fi);
					if (*action_end_fi == g_delimiter)
						action_end_fi = omit_leading_whitespace(++action_end_fi);
					if (!*action_end_fi)
						action_end_fi = NULL;
				}
			}
		}

		if (action_end_fi) // Process the FileInstall command.
		{
			// Now action_end_fi is the position of the first character of the first parameter.
			char literal_map[LINE_SIZE], converted_line[LINE_SIZE];
			// Make a copy so that the original isn't affected.  We want the original
			// line to be written into the EXE so that when the compiled script is
			// launched, the escape sequences can be reparsed again so that it knows'
			// which chars are literal and which are not:
			strcpy(converted_line, action_end_fi);
			// Do a full conversion in case user had something like:
			// FileInstall, Process at 60`%.txt
			// FileInstall, Process`, log`, and completion.txt
			ConvertEscapeSequences(converted_line, literal_map);
			// Find the first non-literal delimiter:
			int j;
			for (j = 0; converted_line[j] && (converted_line[j] != g_delimiter || literal_map[j]); ++j);
			if (converted_line[j]) // It was found.
				converted_line[j] = '\0';
			// Fix for v1.0.46.13: The second below used to be indented so that it belongs to
			// the IF-statement above, but that prevented FileInstall's second parameter from
			// being put on a separate line beneath the command (by means of starting with a
			// a continuation-comma).  Although this fix solves that, it doesn't allow *both*
			// the first and second paramters to be beneath the FileInstall command (only the second).
			rtrim(converted_line);
			// converted_line is now the name of the source file, which is all we need.
			// AutoIt3 section:
			// Show the user what file we are working on (show the full path in lowercase)

			// Show the user what file we are working on (show the full path in lowercase)
			char szBuffer[MAX_PATH + 512];
			snprintf(szBuffer, sizeof(szBuffer), "Compressing and adding: %s", converted_line);
			SendMessage(aStatusBar, SB_SETTEXT, 0, (LPARAM)szBuffer);
			
			char			szTempFilename[_MAX_PATH+1];
			char			*szFilePart;
			GetFullPathName(converted_line, _MAX_PATH, szTempFilename, &szFilePart);
			_strlwr(szTempFilename);

			// Add the file: (Filename, FileID)
			if ( oWrite.FileAdd(szTempFilename, converted_line) != HS_EXEARC_E_OK )
			{
				snprintf(szBuffer, sizeof(szBuffer), "Error adding FileInstall file:\n\n%s", converted_line);
				Util_ShowError(szBuffer);		
				// Caller handles this: oWrite.Close();

				// Reset statusbar
				char szText[127+1];
				if (!LoadString(g_hInstance, IDS_READY, szText, 127))
					*szText = '\0';
				SendMessage(aStatusBar, SB_SETTEXT, 0, (LPARAM)szText);
				return FAIL;
			}
		} // Process the FileInstall command.

		// Write all lines to the target file except #includes themselves (which were handled above)
		// and anything commented out (also handled above).  Same-line comments are not saved
		// because it seems better to minimize the size of the compiled EXE rather than preserve
		// the ability to decompile with comments:
		fputs(buf, aTarget);
		fputc('\n', aTarget);
	} // for()

	fclose(fp);
	return OK;
}



///////////////////////////////////////////////////////////////////////////////
// ScriptRead()
// True = success
//
// This function uses a modified AutotIt script object to read in all the scripts
// and includes.  Unlike in the main AutoIt leading whitespaces aren't trimmed
// but trailing whitespace and newlines are - so we we take this into account
// when we write out the script again.
//
///////////////////////////////////////////////////////////////////////////////

bool App::ScriptRead(const char *szFile, FILE *fScript, EXEArc_Write &oWrite, char *aOutputScriptFilename) // AutoHotkey: added last two params.
{
	AutoIt_ScriptFile	oFile;
//	const char			*szScriptLine;
	char				szAut2Exe[_MAX_PATH+1];
	char				szVersion[43+1];
//	int					nAutScriptLine;
	int					nScriptLine = 1;		// 1 is the first line

	// AutoHotkey:
	// Use the scriptfile object to read the file and do any includes/line continuations
	//if (oFile.LoadScript(szFile) == false)
	//{
	//	fclose(fScript);
	//	return false;
	//}

	// Get version of this compiler and add it as the first line of the script
	GetModuleFileName(NULL, szAut2Exe, _MAX_PATH);
	Util_GetFileVersion(szAut2Exe, szVersion);
	fputs("; <COMPILER: v", fScript);
	fputs(szVersion, fScript);
	fputs(">\n", fScript);

	// AutoHotkey:
	//while ( (szScriptLine = oFile.GetLine(nScriptLine++, nAutScriptLine)) != NULL)
	//{
	//	fputs(szScriptLine, fScript);
	//	fputs("\n", fScript);						// Remember, we stripped these before...
	//}

	// AutoHotkey: Compresses the FileInstall files first, rather than last like AutoIt3:
	for (int i = 0; i < sSourceFileCount; ++i) // In case user has previously pressed the Compile button and now wants to compile something else, reset list of included files.
		free(sSourceFile[i]);
	sSourceFileCount = 0;

	if (LoadIncludedFile(fScript, m_hWndStatusbar, oWrite, (char *)szFile, false, false) != OK)
	{
		fclose(fScript); // Caller expects it to be closed upon return.
		return false;
	}

	// v1.0.47: Support auto-including functions from stdlib/userlib.
	StatusbarWrite("Auto-including any functions called from a library...");

	// Get a temporary filename for the file into which AutoHotkey.exe will write any necessary #include directives:
	char output_include_filename[_MAX_PATH+1], szTempPath[_MAX_PATH+1];
	GetTempPath(_MAX_PATH, szTempPath);
	GetTempFileName(szTempPath, "ahk", 0, output_include_filename); // This creates the file too.

	fclose(fScript); // Close the file so that AutoHotkey.exe can read it.  Also, caller expects it to be closed upon return.

	// Invoke AutoHotkey.exe because it has the detailed parsing necessary to resolve function-calls:
	char szCmdLine[_MAX_PATH*4]; // v1.0.47: Increased from _MAX_PATH+1 to allow room for three full-paths with extra switches/options.
	// The /ErrorStdOut switch is specified to avoid showing syntax-error dialogs because it would cause
	// the temp files to be held open and partially complete while waiting for the dialog to be dismissed.
	// Also, it might disrupt backward compatibility for those who automate the compiling of scripts via
	// command-line, etc.  Therefore, any syntax errors caught by AutoHotkey.exe won't be shown, but they
	// might cause some library function not to get included that should be.  Even so, the problem will
	// become apparent to the user when the compiled script or uncompiled script is run.
	sprintf(szCmdLine, "\"%s..\\AutoHotkey.exe\" /ErrorStdOut /iLib \"%s\" \"%s\""
		, m_szAut2ExeDir // Contains a trailing backslash.
		, output_include_filename
		, aOutputScriptFilename);
	if (!Util_Run(szCmdLine, m_szAut2ExeDir, SW_HIDE, true)) // Success.
	{
		FILE *fp;
		if (fp = fopen(aOutputScriptFilename, "a")) // Reopen the output script privately, since caller wants the public handle closed upon return.
		{
			g_OmitFurtherIncludeComments = true; // For privacy reasons, don't write comments containing the full paths of auto-included files into the compiled script (in case the author distributes it publicly and someone decompiles it, which would reveal the author's directory names/structure containing the userlib). They're also omitted because they're not technically/explicitly includes.
			// To minimize compiled-script size (and because there will often be nothing below the following line), this isn't done:
			//fputs("\n; EVERYTHING BELOW THIS POINT WAS AUTO-INCLUDED FROM A FUNCTION LIBRARY.\n\n", fp);
			LoadIncludedFile(fp, m_hWndStatusbar, oWrite, output_include_filename, true, true); // Tell it to allow dupes (since this can't be a dupe), and ignore failure (in case file doesn't exist, or is empty etc.)
			fclose(fp);
		}
		//else reopening the output script failed.  Too rare, and its tangible consequences minimal, so just ignore it.
		DeleteFile(output_include_filename);
	}
	//else the launch failed. It seems best to tolerate this in case AutoHotkey.exe is absent (to maximize portability).
	StatusbarWrite(IDS_READY); // Reset statusbar

	return true;
} // ScriptRead()



///////////////////////////////////////////////////////////////////////////////
// CompileScript()
// Adds the script file and any files specified in FileInstall functions
// to the destination exe file
///////////////////////////////////////////////////////////////////////////////

bool App::CompileScript(char *szScript, char *szDest, char *szPass, EXEArc_Write &oWrite
	, char *aInternalScriptName) // AutoHotkey: added last 2 params.
{
	char			szBuffer[AUT_MAX_LINESIZE+1];

	// AutoHotkey: The file will be opened by the caller instead, so that it
	// can add the FileInstall files during the parsing of the script and its
	// includes, rather than afterward like AutoIt3:

	//HS_EXEArc_Write	oWrite;

	// First, add our script to the archive with a special File ID
	strcpy(szBuffer, "Compressing and adding: Master Script");
	StatusbarWrite(szBuffer);

	// AutoHotkey:
	if ( oWrite.FileAdd(szScript, aInternalScriptName) != HS_EXEARC_E_OK )
	{
		strcpy(szBuffer, "Error adding script file:\n\n");
		strcat(szBuffer, szScript);
		Util_ShowError(szBuffer);
		// AutoHotkey: This is now done by caller:
		//oWrite.Close();
		return false;
	}

	StatusbarWrite(IDS_READY);					// Reset statusbar
	
	return true;

} // CompileScript()


