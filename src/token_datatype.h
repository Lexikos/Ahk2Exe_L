#ifndef __TOKEN_H
#define __TOKEN_H

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
// token_datatype.h
//
// The class for a token datatype (requires variant_datatype).
// Functions are called via operator overloading!
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "variant_datatype.h"


// Token types
#define TOK_KEYWORD			0					// If, else...
#define TOK_FUNCTION		1					// chr, user, send, winwait...
#define TOK_VARIANT			2					// 10.0, 10, string ("string")
#define TOK_ASSIGNMENT		3					// =
#define TOK_EQUAL			TOK_ASSIGNMENT		// =
#define TOK_GREATER			4					// >
#define TOK_LESS			5					// <
#define TOK_NOTEQUAL		6					// <>
#define TOK_GREATEREQUAL	7					// >=
#define TOK_LESSEQUAL		8					// <=
#define TOK_LEFTPAREN		9					// (
#define TOK_RIGHTPAREN		10					// )
#define TOK_PLUS			11					// +
#define TOK_MINUS			12					// -
#define TOK_DIV				13					// /
#define TOK_MULT			14					// *
#define TOK_VARIABLE		15					// var ($var)
#define TOK_END				16					// End of tokens
#define TOK_USERFUNCTION	17					// User defined function
#define TOK_COMMA			18					// ,
#define TOK_CONCAT			19					// &
#define TOK_LEFTSUBSCRIPT	20					// [
#define TOK_RIGHTSUBSCRIPT	21					// ]
#define TOK_MACRO			22					// predefined var (@var)
#define TOK_EQUALCASE		23					// ==


class Token
{
public:
	// Functions
	Token();									// Constructor
	Token(const Token &vOp2);					// Copy constructor
	~Token();									// Destructor

	Token&		operator=(const Token &vOp2);	// Overloaded = for tokens

	// Variables
	int			m_nType;						// Token type
	Variant		m_Variant;						// Variant
	int			m_nCol;							// Column number this token came from
};

///////////////////////////////////////////////////////////////////////////////

#endif
