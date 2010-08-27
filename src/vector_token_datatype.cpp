
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
// vector_token_datatype.cpp
//
// The class for a token vector datatype.  Not using STL because of the
// code bloat.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
#endif

#include "vector_token_datatype.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor()
///////////////////////////////////////////////////////////////////////////////

VectorToken::VectorToken()
{
	m_nItems	= 0;
	m_lpFirst	= NULL;
	m_lpLast	= NULL;

} // VectorToken()


///////////////////////////////////////////////////////////////////////////////
// Copy Constructor()
///////////////////////////////////////////////////////////////////////////////

VectorToken::VectorToken(const VectorToken &vSource)
{
	VectorTokenNode	*lpTemp;

	m_nItems	= 0;
	m_lpFirst	= NULL;
	m_lpLast	= NULL;

	lpTemp = vSource.m_lpFirst;
	while(lpTemp != NULL)
	{
		push_back(lpTemp->tItem);
		lpTemp = lpTemp->lpNext;
	}
}


///////////////////////////////////////////////////////////////////////////////
// Destructor()
///////////////////////////////////////////////////////////////////////////////

VectorToken::~VectorToken()
{
	VectorTokenNode	*lpTemp, *lpTemp2;

	lpTemp = m_lpFirst;

	while(lpTemp != NULL)
	{
		lpTemp2 = lpTemp->lpNext;
		delete lpTemp;
		lpTemp = lpTemp2;
	}

} // ~VectorToken()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator[]()
///////////////////////////////////////////////////////////////////////////////

Token VectorToken::operator[](unsigned int nIndex) const
{
	// Check bounds
	if (nIndex >= m_nItems)
	{
		Token tTemp;
		tTemp.m_nType = TOK_END;
		return tTemp;							// return an END token
	}


	// Loop through and return the correct token
	unsigned int	i;
	VectorTokenNode	*lpTemp;

	lpTemp = m_lpFirst;

	for (i=0; i<nIndex; i++)
		lpTemp = lpTemp->lpNext;

	return lpTemp->tItem;

} // operator[]()


///////////////////////////////////////////////////////////////////////////////
// push_back()
// Pushes a value onto the end of the vector
///////////////////////////////////////////////////////////////////////////////

void VectorToken::push_back(const Token &tItem)
{
	VectorTokenNode	*lpTemp;

	// Create a new node
	lpTemp			= new VectorTokenNode;
	lpTemp->tItem	= tItem;
	lpTemp->lpNext	= NULL;

	// Add it to the end of the vector
	if (m_lpLast)
	{
		m_lpLast->lpNext	= lpTemp;
		m_lpLast			= lpTemp;
	}
	else
	{
		// First entry
		m_lpFirst = m_lpLast = lpTemp;
	}

	m_nItems++;

} // push_back()


///////////////////////////////////////////////////////////////////////////////
// empty()
// Returns true if the stack is empty
///////////////////////////////////////////////////////////////////////////////

bool VectorToken::empty(void) const
{
	if (m_nItems)
		return false;
	else
		return true;

} // empty()


///////////////////////////////////////////////////////////////////////////////
// clear()
// Clears all items in the vector
///////////////////////////////////////////////////////////////////////////////

void VectorToken::clear(void)
{
	VectorTokenNode	*lpTemp, *lpTemp2;

	lpTemp = m_lpFirst;

	while(lpTemp != NULL)
	{
		lpTemp2 = lpTemp->lpNext;
		delete lpTemp;
		lpTemp = lpTemp2;
	}

	m_nItems	= 0;
	m_lpFirst	= NULL;
	m_lpLast	= NULL;

} // clear()



