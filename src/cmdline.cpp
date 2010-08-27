
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
// cmdline.cpp
//
// A standalone class to make reading command line options a little easier.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
//	#include <windows.h>
#endif

#include "cmdline.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor()
///////////////////////////////////////////////////////////////////////////////

CmdLine::CmdLine()
{
	int i;

	m_nNumParameters	= 0;					// Number of space separated paramters on the cmdline
	m_nCurrentParam		= 0;					// Current param to return for GetNextParam()

	// Ensure all pointers are NULL
	for (i=0; i<CMDLINE_MAXPARAMS; i++)
		m_szParams[i] = NULL;

}


///////////////////////////////////////////////////////////////////////////////
// Destructor()
///////////////////////////////////////////////////////////////////////////////

CmdLine::~CmdLine()
{
	Reset();

}


///////////////////////////////////////////////////////////////////////////////
// SetCmdLine()
///////////////////////////////////////////////////////////////////////////////

void CmdLine::SetCmdLine(char *szCmdLine)
{
	int i;

	Reset();									// Reset any previous command lines


	// Command line parameters are separated by spaces
	// If spaces are required in a parameter, it should be surrounded by quotes
	// If quotes are required, they must be doubled up eg "" is one quote

	i			= 0;							// In string position
	int			iParam = 0;						// Out param position
	bool		bQuote = false;					// Quoting is inactive
	char		szParam[CMDLINE_MAXLEN+1];		// Store our temp parameter
	char		ch;

	// Whenever we hit a space and we are NOT in quote mode
	// - Store param
	// - Skip spaces

	// Skip leading spaces
	while ( (ch = szCmdLine[i]) == ' ' )
		i++;

	while ( ((ch = szCmdLine[i++]) != '\0')  &&  (iParam < CMDLINE_MAXLEN) )
	{
		if ( ch == ' ')
		{
			// Param separator found - are we in quote mode?
			if ( bQuote == true )
			{
				szParam[iParam++] = ch;			// Store as normal - not param separator
			}
			else
			{
				szParam[iParam] = '\0';
				StoreParam(szParam);
				iParam = 0;
				// We are starting a param, skip all spaces
				while ( (ch = szCmdLine[i]) == ' ' )
					i++;
			}
		}
		else
		{
			if ( ch == '"' )
			{
				if (szCmdLine[i] == '"')
				{
					szParam[iParam++] = '"';
					i++;
				}
				else
					bQuote = !bQuote;					// Quote found - toggle quote mode
			}
			else
				szParam[iParam++] = ch;				// Simply store character
		}

	} // End While

	// Check iParam, if there was a construct in progress, finish and store
	if ( iParam != 0 )
	{
		szParam[iParam] = '\0';
		StoreParam(szParam);
	}


} // SetCmdLine()


///////////////////////////////////////////////////////////////////////////////
// StoreParam()
///////////////////////////////////////////////////////////////////////////////

void CmdLine::StoreParam(char *szParam)
{
	// Create enough space to store our line + \0
	m_szParams[m_nNumParameters] = (char *)malloc((strlen(szParam)+1) * sizeof(char));
	strcpy(m_szParams[m_nNumParameters], szParam);

	m_nNumParameters++;

} // StoreParam()

///////////////////////////////////////////////////////////////////////////////
// Reset()
///////////////////////////////////////////////////////////////////////////////

void CmdLine::Reset(void)
{
	int i;

	// Ensure all pointers are NULL
	for (i=0; i<m_nNumParameters; i++)
	{
		if (m_szParams[i] != NULL)
		{
			free(m_szParams[i]);
			m_szParams[i] = NULL;
		}
	}

	m_nNumParameters	= 0;		
	m_nCurrentParam		= 0;

} // Reset()


///////////////////////////////////////////////////////////////////////////////
// GetParam()
///////////////////////////////////////////////////////////////////////////////

bool CmdLine::GetParam(int nParam, char *szParam)
{
	// Ensure we don't pass back crap if there is an error
	szParam[0] = '\0';

	if (nParam >= m_nNumParameters)
		return false;							// Invalid request
	else
	{
		if (m_szParams[nParam] == NULL)
			return false;
		else
		{
			strcpy(szParam, m_szParams[nParam]);
			return true;
		}
	}

} // GetParam()


///////////////////////////////////////////////////////////////////////////////
// GetNextParam()
///////////////////////////////////////////////////////////////////////////////

bool CmdLine::GetNextParam(char *szParam)
{
	// Ensure we don't pass back crap if there is an error
	szParam[0] = '\0';

	if (m_nCurrentParam >= m_nNumParameters)
		return false;							// Invalid request
	else
	{
		if (m_szParams[m_nCurrentParam] == NULL)
			return false;
		else
		{
			strcpy(szParam, m_szParams[m_nCurrentParam]);
			m_nCurrentParam++;
			return true;
		}
	}

} // GetNextParam()
