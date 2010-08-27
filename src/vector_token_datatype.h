#ifndef __VECTOR_TOKEN_H
#define __VECTOR_TOKEN_H

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
// vector_token_datatype.h
//
// The class for a token vector datatype.  Not using STL because of the
// code bloat.
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "token_datatype.h"


// Structs for stack node
typedef struct _VectorTokenNode
{
	Token					tItem;			// The item
	struct _VectorTokenNode	*lpNext;		// Next node (or NULL)

} VectorTokenNode;


class VectorToken
{
public:
	// Functions
	VectorToken();							// Constructor
	VectorToken(const VectorToken &vSource);// Copy Constructor
	~VectorToken();							// Destructor

	void			push_back(const Token &tItem);	// Push item onto end of vector
	void			clear(void);					// Deletes all items


	// Properties
	unsigned int	size(void) const { return m_nItems; }	// Return number of items on stack
	bool			empty(void) const;						// Tests if stack empty
	unsigned int	begin(void) const { return 0; }			// Subscript of first
	unsigned int	end(void) const { return m_nItems; }	// Subscript succeeding last item


	// Overloads
	Token			operator[](unsigned int nIndex) const;	// Overloaded []

private:
	// Variables
	unsigned int	m_nItems;				// Number of items on stack
	VectorTokenNode	*m_lpFirst;				// Pointer to first node
	VectorTokenNode	*m_lpLast;				// Pointer to last node

};

///////////////////////////////////////////////////////////////////////////////

#endif
