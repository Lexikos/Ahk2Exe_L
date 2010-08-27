#ifndef __CMDLINE_H
#define __CMDLINE_H

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
// cmdline.h
//
// A standalone class to make reading command line options a little easier.
//
///////////////////////////////////////////////////////////////////////////////


#define	CMDLINE_MAXPARAMS	64					// Max number of parameters
#define CMDLINE_MAXLEN		1024				// Each parameter can be this many characters

class CmdLine
{
public:
	CmdLine();									// Constructor
	~CmdLine();									// Destructor

	// Functions
	void	SetCmdLine(char *szCmdLine);		// Call this with the full cmd line
	int		GetNumParams(void){return m_nNumParameters;}
	bool	GetParam(int nParam, char *szParam);
	bool	GetNextParam(char *szParam);
	void	GetNextParamReset(void){m_nCurrentParam = 0;}

private:
	// Variables
	int		m_nNumParameters;					// Number of space separated paramters on the cmdline
	int		m_nCurrentParam;					// Current param to return for GetNextParam()
	char	*m_szParams[CMDLINE_MAXPARAMS];		// List of pointers to each parameter

	// Functions
	void	StoreParam(char *szParam);
	void	Reset(void);						// Frees up memory ready for reuse

};


///////////////////////////////////////////////////////////////////////////////

#endif
