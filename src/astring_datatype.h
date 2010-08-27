#ifndef __ASTRING_H
#define __ASTRING_H

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
// astring_datatype.h
//
// The standalone class for a String datatype.  Not using STL because of the
// code bloat.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include <string.h>


class AString
{
public:
	// Functions
	AString(const char szStr[] = "");			// Constructor
	AString(unsigned int iLen);					// Constructor with pre-allocation
	AString(const AString &sSource);			// Copy constructor
	~AString();									// Destructor

	void			erase(void);									// Erase entire string
	void			erase(unsigned int nStart, unsigned int nEnd);	// Erase a range from the string
	void			erase(unsigned int nStart);						// Erase a from start to end

	const char*		c_str(void) const { return m_szText; }			// Return C style string
	void			tolower(void);									// Convert string to lowercase
	void			toupper(void);									// Convert string to uppercase
	void			assign(const AString &sStr,
						unsigned int nStart, unsigned int nEnd);	// Assign chars from one str to another

	unsigned int	find_first_not_of(const char *szInput) const;			// Find first char not in input
	unsigned int	find_last_not_of(const char *szInput) const;			// Find last char not in input
	unsigned int	find_first_of(const char *szInput) const;				// Find first char in input
	unsigned int	find_last_of(const char *szInput) const;				// Find last char in input
	unsigned int	find_str(const char *szInput, bool bCaseSense) const;	// Find first string in input

	void			strip_leading(const char *szInput);				// Strip leading chars
	void			strip_trailing(const char *szInput);			// Strip trailing chars
	int				strcmp(const AString &sStr) const;				// Use C strcmp() routine (compare with AString)
	int				strcmp(const char *szStr) const;				// Use C strcmp() routine (compare with C string)
	int				stricmp(const AString &sStr) const;				// Use C stricmp() routine (compare with AString)
	int				stricmp(const char *szStr) const;				// Use C stricmp() routine (compare with C string)


	// Properties
	unsigned int	size(void) const { return (unsigned int)m_length; }		// Return string length
	unsigned int	length(void) const { return (unsigned int)m_length; }	// Return string length
	unsigned int	begin(void) const { return 0; }							// Index of first element of a string
	unsigned int	end(void) const { return (unsigned int)m_length; }		// Index succeeding last element of a string (The terminator \0)
	bool			empty(void) const;										// Tests if string empty


	// Overloads
	char&		operator[](unsigned int nIndex);	// Overloaded []
	AString&	operator=(const AString &sOp2);		// Overloaded = for AString
	AString&	operator=(const char *szOp2);		// Overloaded = for C string
	AString&	operator=(const char ch);			// Overloaded = for a single char
	AString&	operator+=(const AString &sOp2);	// Overloaded += for AString (append)
	AString&	operator+=(const char *szOp2);		// Overloaded += for C string (append)
	AString&	operator+=(const char ch);			// Overloaded += for a single char (append)

	friend bool	operator==(const AString &sOp1, const AString &sOp2);	// Overloaded == for AStrings (case sensitive)
	friend bool	operator!=(const AString &sOp1, const AString &sOp2);	// Overloaded != for AStrings (case sensitive)


private:
	// Variables
	char		*m_szText;						// The actual string of characters
	size_t		m_length;						// String length
	size_t		m_allocated;					// Number of bytes allocated for this string
};

///////////////////////////////////////////////////////////////////////////////

#endif
