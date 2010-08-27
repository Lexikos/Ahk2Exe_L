
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
// astring_datatype.cpp
//
// The standalone class for a String datatype.  Not using STL because of the
// code bloat.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
//	#include <stdio.h>
//	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <windows.h>
#endif

#include "astring_datatype.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor()
// Default constructor used when no params or a char * given
///////////////////////////////////////////////////////////////////////////////

AString::AString(const char szStr[])
{
	m_length	= strlen(szStr);
	m_allocated = m_length + 1;
	m_szText	= new char[m_allocated];
	strcpy(m_szText, szStr);

} // AString()


///////////////////////////////////////////////////////////////////////////////
// Constructor()
// With pre-allocation of the string size to speed up multiple += ops
///////////////////////////////////////////////////////////////////////////////

AString::AString(unsigned int iLen)
{
	m_length	= iLen;
	m_allocated = m_length + 1;
	m_szText	= new char[m_allocated];
	m_szText[0]	= '\0';

} // AString()


///////////////////////////////////////////////////////////////////////////////
// Copy Constructor()
///////////////////////////////////////////////////////////////////////////////

AString::AString(const AString &sSource)
{
	m_length	= sSource.m_length;
	m_allocated	= m_length + 1;
	m_szText	= new char[m_allocated];
	strcpy(m_szText, sSource.m_szText);
}


///////////////////////////////////////////////////////////////////////////////
// Destructor()
///////////////////////////////////////////////////////////////////////////////

AString::~AString()
{
	delete [] m_szText;

} // ~AString()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator[]()
///////////////////////////////////////////////////////////////////////////////

char& AString::operator[](unsigned int nIndex)
{
	if (nIndex <= m_length)
		return m_szText[nIndex];
	else
		return m_szText[m_length];				// return \0

} // operator[]()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for AStrings
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator=(const AString &sOp2)
{
	if ( this == &sOp2 )
		return *this;

	if (m_allocated < sOp2.m_allocated)
	{
		delete [] m_szText;
		m_allocated	= sOp2.m_allocated;
		m_szText	= new char[m_allocated];
	}

	m_length	= sOp2.m_length;
	strcpy(m_szText, sOp2.m_szText);

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for C strings
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator=(const char *szOp2)
{
	size_t	Op2Len;

	Op2Len = strlen(szOp2);

	if (m_allocated < (Op2Len+1) )
	{
		delete [] m_szText;
		m_allocated	= (Op2Len+1);
		m_szText	= new char[m_allocated];
	}

	m_length	= Op2Len;
	strcpy(m_szText, szOp2);

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for a single char
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator=(const char ch)
{
	if (m_allocated < 2 )
	{
		delete [] m_szText;
		m_allocated	= 2;
		m_szText	= new char[2];
	}

	m_length	= 1;
	m_szText[0]	= ch;
	m_szText[1]	= '\0';

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator+=() for AStrings
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator+=(const AString &sOp2)
{
	size_t	CombinedLen;

	if ( this == &sOp2 )
		return *this;

	CombinedLen = m_length + sOp2.m_length;

	if (m_allocated < (CombinedLen+1) )
	{
		m_allocated	= CombinedLen + 1;
		char *szTemp = new char[m_allocated];
		strcpy(szTemp, m_szText);
		delete [] m_szText;
		m_szText = szTemp;
	}

	m_length	= CombinedLen;
	strcat(m_szText, sOp2.m_szText);

	return *this;								// Return this object that generated the call

} // operator+=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator+=() for C strings
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator+=(const char *szOp2)
{
	size_t	CombinedLen;

	CombinedLen = m_length + strlen(szOp2);

	if (m_allocated < (CombinedLen+1) )
	{
		m_allocated	= CombinedLen + 1;
		char *szTemp = new char[m_allocated];
		strcpy(szTemp, m_szText);
		delete [] m_szText;
		m_szText = szTemp;
	}

	m_length	= CombinedLen;
	strcat(m_szText, szOp2);

	return *this;								// Return this object that generated the call

} // operator+=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator+=() for a single char
///////////////////////////////////////////////////////////////////////////////

AString& AString::operator+=(const char ch)
{
	size_t	CombinedLen;

	CombinedLen = m_length + 1;

	if (m_allocated < (CombinedLen+1) )
	{
		m_allocated	= CombinedLen + 1;
		char *szTemp = new char[m_allocated];
		strcpy(szTemp, m_szText);
		delete [] m_szText;
		m_szText = szTemp;
	}

	m_szText[m_length]		= ch;
	m_szText[m_length+1]	= '\0';
	m_length				= CombinedLen;

	return *this;								// Return this object that generated the call

} // operator+=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator==() for AStrings
//
// CASE SENSITIVE
///////////////////////////////////////////////////////////////////////////////

bool operator==(const AString &sOp1, const AString &sOp2)
{
	if ( !strcmp(sOp1.m_szText, sOp2.m_szText) )
		return true;
	else
		return false;

} // operator==()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator!=() for AStrings
//
// CASE SENSITIVE
///////////////////////////////////////////////////////////////////////////////

bool operator!=(const AString &sOp1, const AString &sOp2)
{
	if ( strcmp(sOp1.m_szText, sOp2.m_szText) )
		return true;
	else
		return false;

} // operator!=()


///////////////////////////////////////////////////////////////////////////////
// tolower()
///////////////////////////////////////////////////////////////////////////////

void AString::tolower(void)
{
	CharLower(m_szText);

} // tolower()


///////////////////////////////////////////////////////////////////////////////
// toupper()
///////////////////////////////////////////////////////////////////////////////

void AString::toupper(void)
{
	CharUpper(m_szText);

} // toupper()


///////////////////////////////////////////////////////////////////////////////
// empty()
// Returns true if a string is empty (first char is \0)
///////////////////////////////////////////////////////////////////////////////

bool AString::empty(void) const
{
	if (m_szText[0] == '\0')
		return true;
	else
		return false;

} // empty()


///////////////////////////////////////////////////////////////////////////////
// erase()
// Erase an entire string (and release memory)
///////////////////////////////////////////////////////////////////////////////

void AString::erase(void)
{
	delete [] m_szText;

	m_length	= 0;
	m_allocated	= 1;
	m_szText	= new char[m_allocated];
	m_szText[0]	= '\0';

} // erase()


///////////////////////////////////////////////////////////////////////////////
// erase()
// Erase a range from a string (erase from nStart to (but not including) nEnd)
///////////////////////////////////////////////////////////////////////////////

void AString::erase(unsigned int nStart, unsigned int nEnd)
{
	unsigned int i, j;

	if (nStart >= m_length || nEnd > m_length || nEnd <= nStart)
		return;

	i = nStart;
	j = nEnd;
	while (m_szText[j] != '\0')
	{
		m_szText[i] = m_szText[j];
		i++;
		j++;
	}

	m_szText[i] = '\0';							// Terminate

	m_length	= strlen(m_szText);

} // erase()


///////////////////////////////////////////////////////////////////////////////
// erase()
// Erase a range from a string (erase from nStart to end of string)
///////////////////////////////////////////////////////////////////////////////

void AString::erase(unsigned int nStart)
{
	if (nStart >= m_length)
		return;

	m_szText[nStart] = '\0';
	m_length	= strlen(m_szText);

} // erase()


///////////////////////////////////////////////////////////////////////////////
// assign()
// Assign a string with a range from another string
// (copy from nStart to (but not including) nEnd)
///////////////////////////////////////////////////////////////////////////////

void AString::assign(const AString &sStr, unsigned int nStart, unsigned int nEnd)
{
	unsigned int i, j;
	size_t	Len;

	Len = nEnd - nStart;						// Number of elements to copy

	// Valid ranges?
	if (nStart >= sStr.m_length || nEnd > sStr.m_length || nEnd <= nStart)
		return;

	// Does the existing string have enough space?
	if (m_allocated < (Len+1) )
	{
		m_allocated	= Len + 1;
		delete [] m_szText;
		m_szText = new char[m_allocated];
	}

	m_length = Len;

	i = 0;
	j = nStart;
	while (j < nEnd)
		m_szText[i++] = sStr.m_szText[j++];

	m_szText[i] = '\0';							// Terminate

} // assign()


///////////////////////////////////////////////////////////////////////////////
// find_first_not_of()
// Returns first element doesn't match the input (or returns AString.end() )
///////////////////////////////////////////////////////////////////////////////

unsigned int AString::find_first_not_of(const char *szInput) const
{
	unsigned int	i, j, nMatches;
	size_t			nCount;

	nCount	= m_length;
	i		= 0;
	while (nCount > 0)
	{
		nCount--;

		j			= 0;
		nMatches	= 0;
		while (szInput[j] != '\0')
		{
			if (m_szText[i] == szInput[j])
				nMatches++;

			j++;
		}

		// Did all the characters match?
		if (nMatches == 0)
			return i;							// This is the first pos not to match

		i++;
	}

	return i;									// i = AString.end()

} // find_first_not_of()


///////////////////////////////////////////////////////////////////////////////
// find_last_not_of()
// Returns first element doesn't match the input (or returns AString.end() )
///////////////////////////////////////////////////////////////////////////////

unsigned int AString::find_last_not_of(const char *szInput) const
{
	unsigned int	i, j, nMatches;
	size_t			nCount;

	nCount	= m_length;
	i		= (unsigned int)m_length - 1;
	while (nCount > 0)
	{
		nCount--;

		j			= 0;
		nMatches	= 0;
		while (szInput[j] != '\0')
		{
			if (m_szText[i] == szInput[j])
				nMatches++;

			j++;
		}

		// Did all the characters match?
		if (nMatches == 0)
			return i;							// This is the first pos not to match

		i--;
	}

	return end();								// i = AString.end()

} // find_last_not_of()


///////////////////////////////////////////////////////////////////////////////
// find_first_of()
// Returns first element to match any character in the input (or returns AString.end() )
///////////////////////////////////////////////////////////////////////////////

unsigned int AString::find_first_of(const char *szInput) const
{
	unsigned int	i, j;
	size_t			nCount;

	nCount	= m_length;
	i		= 0;
	while (nCount > 0)
	{
		nCount--;

		j			= 0;
		while (szInput[j] != '\0')
		{
			if (m_szText[i] == szInput[j])
				return i;

			j++;
		}

		i++;
	}

	return i;									// i = AString.end()

} // find_first_of()



///////////////////////////////////////////////////////////////////////////////
// find_last_of()
// Returns last element to match any character in the input (or returns AString.end() )
///////////////////////////////////////////////////////////////////////////////

unsigned int AString::find_last_of(const char *szInput) const
{
	unsigned int	i, j;
	size_t			nCount;
	
	nCount	= m_length;
	i		= (unsigned int)(m_length-1);
	while (nCount > 0)
	{
		nCount--;

		j			= 0;
		while (szInput[j] != '\0')
		{
			if (m_szText[i] == szInput[j])
				return i;

			j++;
		}

		i--;
	}

	return end();								// i = AString.end()

} // find_last_of()


///////////////////////////////////////////////////////////////////////////////
// find_str()
// Returns first element to match the input string (or returns AString.end() )
///////////////////////////////////////////////////////////////////////////////

unsigned int AString::find_str(const char *szInput, bool bCaseSense) const
{
	unsigned int	i, j, k, nMatches;
	size_t			nInputLen;

	i			= 0;
	nInputLen	= strlen(szInput);

	while (i < m_length)
	{
		j			= 0;
		k			= i;
		nMatches	= 0;
		while (szInput[j] != '\0' && m_szText[k] != '\0') 
		{
			if (bCaseSense)
			{
				if ( m_szText[k] != szInput[j] )
					break;						// Exit while loop
			}
			else
			{
				if ( ::tolower(m_szText[k]) != ::tolower(szInput[j]) )
					break;						// Exit while loop

			}

			nMatches++;
			k++;
			j++;
		}

		if (nMatches == nInputLen)
			return i;

		i++;
	}

	return i;									// i = AString.end()

} // find_str()


///////////////////////////////////////////////////////////////////////////////
// strip_leading()
// Strips leading characters that match the input
///////////////////////////////////////////////////////////////////////////////

void AString::strip_leading(const char *szInput)
{
	erase ( 0, find_first_not_of(szInput) );

} // strip_leading()


///////////////////////////////////////////////////////////////////////////////
// strip_trailing()
// Strips trailing characters that match the input
///////////////////////////////////////////////////////////////////////////////

void AString::strip_trailing(const char *szInput)
{
	unsigned int	nPos;

	nPos = find_last_not_of (szInput);
	if ( nPos == end() )						// =end when ALL matched
		erase();
	else
		erase ( nPos + 1 );

} // strip_leading()


///////////////////////////////////////////////////////////////////////////////
// strcmp()
// calls the C strcmp routine for two AStrings
///////////////////////////////////////////////////////////////////////////////

int AString::strcmp(const AString &sStr) const
{
	return ::strcmp(m_szText, sStr.m_szText);

} // strcmp()


///////////////////////////////////////////////////////////////////////////////
// strcmp()
// calls the C strcmp routine for AString and a C string
///////////////////////////////////////////////////////////////////////////////

int AString::strcmp(const char *szStr) const
{
	return ::strcmp(m_szText, szStr);

} // strcmp()


///////////////////////////////////////////////////////////////////////////////
// stricmp()
// calls the C strcmp routine for two AStrings
///////////////////////////////////////////////////////////////////////////////

int AString::stricmp(const AString &sStr) const
{
	return ::stricmp(m_szText, sStr.m_szText);

} // stricmp()


///////////////////////////////////////////////////////////////////////////////
// stricmp()
// calls the C strcmp routine for AString and a C string
///////////////////////////////////////////////////////////////////////////////

int AString::stricmp(const char *szStr) const
{
	return ::stricmp(m_szText, szStr);

} // stricmp()

