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
// Helper lib for changing the icon of an exe.
//
///////////////////////////////////////////////////////////////////////////////


#include "ResourceEditor.h"

bool Icon_DoReplace(CResourceEditor* re, WORD wIconId, const char* szIcon, const char *szExe);

///////////////////////////////////////////////////////////////////////////////

#endif

