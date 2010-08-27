#ifndef __CHANGEICON_H
#define __CHANGEICON_H

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
// changeicon.h
//
// Same helper lib for changing the icon of an exe.
//
///////////////////////////////////////////////////////////////////////////////

#define ICON_E_OK				0
#define ICON_E_READEXE			1
#define ICON_E_WRITEEXE			2
#define ICON_E_READICON			3
#define ICON_E_NOMATCHINGICON	4

int Icon_Replace(const char *szExe, const char *szIcon);

///////////////////////////////////////////////////////////////////////////////

#endif

