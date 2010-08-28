#ifndef __UTIL_H
#define __UTIL_H

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
// util.h
//
// Util functions.
//
///////////////////////////////////////////////////////////////////////////////


void	Util_ShowError(char *szText);
void	Util_ShowErrorIDS(unsigned int iErr);
void	Util_ShowInfoIDS(unsigned int iErr);
void	Util_GetFullPath(char *szInput);
int		Util_Run(const char *szCmd, const char *szDir, int nShow, bool bWait);
bool	Util_GetFileVersion(char *szFile, char *szVersion);
WORD	Util_GetFileMachine(const char *szFile);

///////////////////////////////////////////////////////////////////////////////

#endif // __UTIL_H





