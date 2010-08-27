#ifndef __SCRIPTFILE_H
#define __SCRIPTFILE_H

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
// scriptfile.h
//
// The script file object.  This object handles all requests to the script file
// that was read in.
//
///////////////////////////////////////////////////////////////////////////////


// Includes


// Define our structure for holding each line of text from the script
typedef struct larray
{
	char	*szLine;							// Text of the script line
	int 	nLineNum;							// The line number (from the .aut file)
	int		nIncludeID;							// ID of the file this line is from
	struct 	larray *lpNext;						// Next entry in linked list

} LARRAY;

// Define our structure for keeping track of unique include names
#define AUT_MAX_INCLUDE_IDS		64				// Max number of Include filenames 


class AutoIt_ScriptFile
{
public:
	// Functions
	AutoIt_ScriptFile();						// Constructor

	bool			LoadScript(const char *szFile);	// Loads a script into memory
	void			UnloadScript(void);				// Removes a script from memory

	const char *	GetLine(int nLineNum, int &nAutLineNum);		// Retrieve line x
	int				GetNumScriptLines() { return m_nScriptLines; }	// Returns number of lines in the script
	void			AddLine(int nLineNum,  const char *szLine, int nIncludeID);
	
	const char *	GetIncludeName(int nIncludeID);
	int				GetIncludeID(int nLineNum);


private:
	// Variables
	LARRAY			*m_lpScript;				// Start of the linked list
	LARRAY			*m_lpScriptLast;			// Last node of the list
	int				m_nScriptLines;				// Number of lines in the list

	char			*m_szIncludeIDs[AUT_MAX_INCLUDE_IDS];
	int				m_nNumIncludes;

	// Functions
	void			AppendLastLine(const char *szLine);
	bool			Include(const char *szFileName, int nIncludeID);
	int				AddIncludeName(const char *szFileName);
	void			StripLeading(char *szLine);
	void			StripTrailing(char *szLine);
	bool			IncludeParse(const char *szLine, char *szTemp);
};


///////////////////////////////////////////////////////////////////////////////

#endif
