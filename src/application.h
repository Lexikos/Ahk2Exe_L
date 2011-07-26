#ifndef __APPLICATION_H
#define __APPLICATION_H

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
// application.h
//
// The main application object.
//
///////////////////////////////////////////////////////////////////////////////


// Include
#include "vector_token_datatype.h"
#include "lib\exearc_write.h" // AutoHotkey: Added.


class App
{
public:
	// Variables

	// Objects

	// Functions
	App();										// Constructor

	BOOL		Create(int nCmdShow);

private:
	// Variables
	HWND		m_hWndStatusbar;				// Status bar window
	int			m_nCmdShow;						// Initial window state

	char		m_szLastScriptDir[_MAX_PATH+1];
	char		m_szLastExeDir[_MAX_PATH+1];
	char		m_szLastIconDir[_MAX_PATH+1];
	char		m_szLastIcon[_MAX_PATH+1];
	DWORD		m_dwLastCompression;

	// Lexing variable
	static char		*m_szKeywords[];			// Valid keywords

	// Useful directories and filenames
	char		m_szAut2ExeDir[_MAX_PATH+1];
	char		m_szAutoItSC[_MAX_PATH+1];

	// Objects

	// Functions
	BOOL		RegisterClass(void);
	void		RegisterCommonControls(void);

	BOOL		StatusbarCreate(void);
	void		StatusbarWrite(UINT nID);
	void		StatusbarWrite(char *szText);

	static LRESULT CALLBACK WndProc (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT		WndProcHandler (HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK AboutProc (HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

	void		MenuSelect(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void		Command(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	void		LoadSettings(void);
	void		SaveSettings(void);

	bool		Convert(char *szSource, char *szDest, char *szIcon);
	bool		ScriptRead(const char *szFile, FILE *fScript, EXEArc_Write &oWrite, char *aOutputScriptFilename); // AutoHotkey: added last two params.
	bool		CompileScript(char *szScript, char *szDest, EXEArc_Write &oWrite, char *aInternalScriptName); // AutoHotkey: added last 2 params.

	bool		ConvertCheckFilenames(const char *szSource, char *szDest);

	bool		CmdLineMode(void);


	// Lexer functions (lexer.cpp)
	void		Lexer(const char *szLine, VectorToken &vLineToks);	// Convert a string into tokens
	void		Lexer_String(const char *szLine, uint &iPos, char *szTemp);
	void		Lexer_Number(const char *szLine, uint &iPos, Token &rtok, char *szTemp);
	void		Lexer_KeywordOrFunc(const char *szLine, uint &iPos, Token &rtok, char *szTemp);
};

///////////////////////////////////////////////////////////////////////////////

#endif

