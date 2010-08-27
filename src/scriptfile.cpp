
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
// scriptfile.cpp
//
// The script file object.  This object handles all requests to the script file
// that was read in.
//
// Modified version of the AutoIt scriptfile object, not all whitespace is
// stripped just trailing whitespace/newline.  Leading whitespace is stored
// so that when the script is decompiled the formatting is OK.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
	#include <windows.h>
#endif

#include "Aut2Exe.h"
#include "scriptfile.h"
#include "util.h"
#include "resources\resource.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor()
///////////////////////////////////////////////////////////////////////////////

AutoIt_ScriptFile::AutoIt_ScriptFile()
{
	m_lpScript		= NULL;						// Start of the linked list
	m_lpScriptLast	= NULL;						// Last node of the list
	m_nScriptLines	= 0;						// Number of lines in the list

	// Zero our include IDs
	m_nNumIncludes	= 0;

} // AutoIt_ScriptFile()


///////////////////////////////////////////////////////////////////////////////
// AddLine()
///////////////////////////////////////////////////////////////////////////////

void AutoIt_ScriptFile::AddLine(int nLineNum,  const char *szLine, int nIncludeID)
{
	LARRAY	*lpTemp;
	char	*szTemp;

//	if (szLine[0] == '\0')
//		return;									// Empty, don't bother storing unless Aut2Exe

	// Do we need to start linked list?
	if ( m_lpScript == NULL )
	{
		m_lpScript				= new LARRAY;
		m_lpScriptLast			= m_lpScript;
	}
	else
	{
		lpTemp					= new LARRAY;
		m_lpScriptLast->lpNext	= lpTemp;			// Next
		m_lpScriptLast			= lpTemp;
	}

	m_lpScriptLast->lpNext		= NULL;				// Next

	// Store our line
	szTemp = new char[strlen(szLine)+1];
	strcpy(szTemp, szLine);
	m_lpScriptLast->szLine		= szTemp;
	m_lpScriptLast->nLineNum	= nLineNum;
	m_lpScriptLast->nIncludeID	= nIncludeID;

	// Increase the number of lines
	m_nScriptLines++;

} // AddLine()


///////////////////////////////////////////////////////////////////////////////
// AppendLastLine()
//
// Same as AddLine except it adds the text onto the LAST line added
///////////////////////////////////////////////////////////////////////////////

void AutoIt_ScriptFile::AppendLastLine(const char *szLine)
{
	char	*szTemp;
	size_t	CombinedLen;

//	if (szLine[0] == '\0')
//		return;									// Empty, don't bother storing unless Aut2Exe

	// How big are both lines added together?
	CombinedLen = strlen(m_lpScriptLast->szLine) + strlen(szLine);

	// Create a big enough space for the combined line and copy it there
	szTemp = new char[CombinedLen+1];
	strcpy(szTemp, m_lpScriptLast->szLine);
	strcat(szTemp, szLine);

	// The appending may have gone over the max line size, so just do a dirty hack and 
	// enforce the line size by inserting a \0
	if (strlen(szTemp) > AUT_MAX_LINESIZE)
		szTemp[AUT_MAX_LINESIZE] = '\0';

	// Now free the existing line and replace with the new one
	delete [] m_lpScriptLast->szLine;
	m_lpScriptLast->szLine = szTemp;	

} // AppendLastLine()


///////////////////////////////////////////////////////////////////////////////
// AddIncludeName()
///////////////////////////////////////////////////////////////////////////////

int AutoIt_ScriptFile::AddIncludeName(const char *szFileName)
{
	char	szFullPath[_MAX_PATH+1];
	char	*szTemp;

	if (m_nNumIncludes >= AUT_MAX_INCLUDE_IDS)
		return -1;
	
	GetFullPathName(szFileName, _MAX_PATH, szFullPath, &szTemp);

	// Does this file already exist?
	for (int i=0; i < m_nNumIncludes; ++i)
	{
		if (!stricmp(m_szIncludeIDs[i], szFullPath))
			return i;
	}


	// New entry
	szTemp = new char[strlen(szFullPath)+1];
	strcpy(szTemp, szFullPath);

	m_szIncludeIDs[m_nNumIncludes] = szTemp;

	++m_nNumIncludes;

	return m_nNumIncludes-1;

} // AddIncludeName()


///////////////////////////////////////////////////////////////////////////////
// GetIncludeID()
///////////////////////////////////////////////////////////////////////////////

int AutoIt_ScriptFile::GetIncludeID(int nLineNum)
{
	int		i;
	LARRAY	*lpTemp = m_lpScript;

	// Do we have this many lines?
	if (nLineNum > m_nScriptLines || nLineNum <= 0)
		return -1;							// Nope

	for (i=0; i<nLineNum-1; i++)
		lpTemp = lpTemp->lpNext;

	return lpTemp->nIncludeID;

} // GetIncludeID()


///////////////////////////////////////////////////////////////////////////////
// GetIncludeName()
///////////////////////////////////////////////////////////////////////////////

const char * AutoIt_ScriptFile::GetIncludeName(int nIncludeID)
{
	if (nIncludeID >= AUT_MAX_INCLUDE_IDS || nIncludeID < 0)
		return NULL;
	else
		return m_szIncludeIDs[nIncludeID];

} // GetIncludeName()


///////////////////////////////////////////////////////////////////////////////
// UnloadScript()
///////////////////////////////////////////////////////////////////////////////

void AutoIt_ScriptFile::UnloadScript(void)
{
	LARRAY	*lpTemp, *lpTemp2;

	// Unloading the script is simply a matter of freeing all the memory
	// that we allocated in the linked lists

	lpTemp = m_lpScript;
	while (lpTemp != NULL)
	{
		lpTemp2 = lpTemp->lpNext;
		delete [] lpTemp->szLine;				// Free the string
		delete lpTemp;							// Free the node
		lpTemp = lpTemp2;
	}

	// Ensure everything is zeroed in case we load another script
	m_lpScript		= NULL;						// Start of the linked list
	m_lpScriptLast	= NULL;						// Last node of the list
	m_nScriptLines	= 0;						// Number of lines in the list

	// Delete all our includes
	for (int i=0; i<m_nNumIncludes; ++i)
		delete [] m_szIncludeIDs[i];

	m_nNumIncludes = 0;

} // UnloadScript()


///////////////////////////////////////////////////////////////////////////////
// GetLine()
//
// NOTE: Line 1 is the first line (not zero)
//
///////////////////////////////////////////////////////////////////////////////

const char * AutoIt_ScriptFile::GetLine(int nLineNum, int &nAutLineNum)
{
	int		i;
	LARRAY	*lpTemp = m_lpScript;

	// Do we have this many lines?
	if (nLineNum > m_nScriptLines)
		return NULL;							// Nope

	for (i=0; i<nLineNum-1; i++)
		lpTemp = lpTemp->lpNext;

	// Get the line and real line number (.aut file)
	nAutLineNum = lpTemp->nLineNum;

	return 	lpTemp->szLine;						// Return pointer to the line

} // GetLine()


///////////////////////////////////////////////////////////////////////////////
// LoadScript()
//
// NATIVE VERSION
//
///////////////////////////////////////////////////////////////////////////////

bool AutoIt_ScriptFile::LoadScript(const char *szFile)
{

	// Read in the script and any include files
    return Include(szFile, AddIncludeName(szFile));

} // LoadScript()


///////////////////////////////////////////////////////////////////////////////
// Include()
//
///////////////////////////////////////////////////////////////////////////////

bool AutoIt_ScriptFile::Include(const char *szFileName, int nIncludeID)
{
	char			szBuffer[65535+1];
	char			szTemp[65535+1];
	int				nLineNum = 1;				// Line# in .aut file
	FILE			*fptr;
	bool			bErr = true;
	bool			bNextLineAppend = false;	// If true the next line read should be appended
	bool			bContinuationFound;
	int				nLen;

	// Open a handle to the script file
	fptr = fopen(szFileName, "r");				// ASCII read
	if ( fptr == NULL  )						// File error
	{
		strcpy(szBuffer, "Error reading the file:\n\n");
		strcat(szBuffer, szFileName);
		Util_ShowError(szBuffer);		
		return false;
	}

	// Read in lines of text until EOF is reached or error occurs
	while ( fgets(szBuffer, 65535, fptr) && bErr == true)
	{
		// Enforce our maximum line length (must be smaller than szBuffer!)
		szBuffer[AUT_MAX_LINESIZE] = '\0';

		// Don't check for #include if we are continuing previous line
		if ( bNextLineAppend == false && IncludeParse(szBuffer, szTemp) == true )
		{
			// Include file
			strcpy(szBuffer, "; <INCLUDE START: ");
			strcat(szBuffer, szTemp);
			strcat(szBuffer, ">");
			AddLine(nLineNum, szBuffer, nIncludeID);

			bErr = Include(szTemp, AddIncludeName(szTemp));	// Get the include file
			
			strcpy(szBuffer, "; <INCLUDE END>");
			AddLine(nLineNum, szBuffer, nIncludeID);
		}
		else
		{
			//StripLeading(szBuffer);				// Only required in AutoIt
			StripTrailing(szBuffer);			// Strip trailing
			nLen = (int)strlen(szBuffer);

			// Check if the last character is requesting a continuation
			if (nLen && szBuffer[nLen-1] == '_')
			{
				szBuffer[nLen-1] = '\0';		// Erase the continuation char
				bContinuationFound = true;
			}
			else
				bContinuationFound = false;

			// Was previous line a continuation?
			if (bNextLineAppend == true)
			{
				StripLeading(szBuffer);			// Only required in Aut2Exe
				AppendLastLine(szBuffer);
			}
			else
				AddLine(nLineNum, szBuffer, nIncludeID );

			if (bContinuationFound == true)
				bNextLineAppend = true;
			else
				bNextLineAppend = false;
		}
		
		nLineNum++;								// Increment the line number

	} // End While

	// Close our script file
	fclose(fptr);

	return bErr;

} // Include()


///////////////////////////////////////////////////////////////////////////////
// IncludeParse()
//
// Checks a line of text to see if it is an #include directive, if so passes
// back the string changed to contain the filename of the file to include
///////////////////////////////////////////////////////////////////////////////

bool AutoIt_ScriptFile::IncludeParse(const char *szLine, char *szTemp)
{
	int i,j;
	int	nLen;

	nLen = (int)strlen(szLine);					// Get length
	i = 0;

	// Skip whitespace
	while (szLine[i] == ' ' || szLine[i] == '\t')
		i++;

	// Copy until next whitespace (should be the #include word)
	j = 0;
	while (szLine [i] != ' ' && szLine[i] != '\t' && szLine[i] != '\0')
		szTemp[j++] = szLine[i++];
	szTemp[j] = '\0';							// Terminate

	if (stricmp(szTemp, "#include"))
		return false;							// No include directive

	// Now find "
	while (szLine[i] != '\"' && szLine[i] != '\0')
		i++;

	if (szLine[i++] == '\0')
		return false;							// Didn't find "

	// Copy until next " (should be the filename)
	j = 0;
	while (szLine [i] != '"' && szLine[i] != '\0')
		szTemp[j++] = szLine[i++];
	szTemp[j] = '\0';							// Terminate

	if (szLine[i++] == '\0')
		return false;							// Didn't find "

	return true;

} // IncludeParse()


///////////////////////////////////////////////////////////////////////////////
// StripTrailing()
///////////////////////////////////////////////////////////////////////////////

void AutoIt_ScriptFile::StripTrailing(char *szLine)
{
	int i;
	int	nLen;

	nLen = (int)strlen(szLine);					// Get length
	
	if (nLen == 0)
		return;									// Nothing to do

	// Strip trailing whitespace and newline
	i = nLen - 1;
	while ( i >= 0 && (szLine[i] == ' ' || szLine[i] == '\t' || szLine[i] == '\n') )
		i--;

	// Remove trailing
	szLine[i+1] = '\0';

} // StripTrailing()


///////////////////////////////////////////////////////////////////////////////
// StripLeading()
///////////////////////////////////////////////////////////////////////////////

void AutoIt_ScriptFile::StripLeading(char *szLine)
{
	int i, j;

	i = 0;
	while ( szLine[i] == ' ' || szLine[i] == '\t' )
		i++;

	if (szLine[i] == '\0')
		return;									// Nothing to do

	j = 0;
	while (szLine[i] != '\0')
		szLine[j++] = szLine[i++];

	szLine[j] = '\0';							// Terminate

} // StripLeading()
