
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
// lexer.cpp
//
// Provides lexing (string into tokens).  Part of application.cpp
// NOTE: This code is taken from the main AutoIt source and slightly modified
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
//	#include <stdlib.h>
//	#include <string.h>
	#include <windows.h>
//	#include <ctype.h>
#endif

#include "Aut2Exe.h"
#include "application.h"


// Keyword values (must match the order as in lexer.cpp)
// This is taken from the main AutoIt source and should match
enum
{
	K_AND, K_OR, K_NOT,
	K_IF, K_THEN, K_ELSE, K_ENDIF,
	K_WHILE, K_WEND,
	K_DO, K_UNTIL, 
	K_FOR, K_NEXT, K_TO, K_STEP,
	K_EXITLOOP, K_CONTINUELOOP,
	K_SELECT, K_CASE, K_ENDSELECT, 
	K_DIM,
	K_FUNC, K_ENDFUNC, K_RETURN,
	K_EXIT,
	K_BYREF,
	K_MAX
};

char * App::m_szKeywords[K_MAX] =	{
	"And", "Or", "Not",
	"If", "Then", "Else", "EndIf",
	"While", "Wend",
	"Do", "Until",
	"For", "Next", "To", "Step",
	"ExitLoop", "ContinueLoop",
	"Select", "Case", "EndSelect",
	"Dim",
	"Func", "EndFunc", "Return",
	"Exit",
	"ByRef"
};


///////////////////////////////////////////////////////////////////////////////
// Lexer()
//
// Converts the current line of script into tokens
//
// Uses static members to reduce object creation time (significant in large loops)
// Two tokens are used, one for storing "easy" tokens and the other for
// strings (when you store a variant the variant reallocates mem = slow)
//
///////////////////////////////////////////////////////////////////////////////

void App::Lexer(const char *szLine, VectorToken &vLineToks)
{
	uint			iPos, iPosTemp;				// Position in the string
	static Token	tok;						// Token variable (don't use for simple TOK_ types)
	static Token	tokVariant;					// Token variable (use for variants)
	static	char	szTemp[AUT_MAX_LINESIZE+1];	// Static and preallocated to speed things up
	char			ch;


	// Clear the line tokens vector
	vLineToks.clear();

	// Get start and end positions of the string
	iPos = 0;

	// The main lex loop
	while (szLine[iPos] != '\0')
	{
		// Skip whitespace	
		while (szLine[iPos] == ' ' || szLine[iPos] == '\t')
			++iPos;

		// Reached the end?
		if (szLine[iPos] == '\0')
			break;

		// Save the column number of the token (used in error output)
		tok.m_nCol = iPos;
		tokVariant.m_nCol = iPos;

		// Zero our temp string position
		iPosTemp = 0;

		switch (szLine[iPos])
		{
			case ';':
				// End when we reach a comment
				tok.m_nType = TOK_END;
				vLineToks.push_back(tok);
				return;

			case '$':							// Variable ($var)
				++iPos;							// Skip $

				ch = szLine[iPos];
				while ( (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
						(ch >= '0' && ch <= '9') || ch == '_')
				{
					szTemp[iPosTemp++] = ch;
					++iPos;
					ch = szLine[iPos];
				}
				szTemp[iPosTemp] = '\0';		// Terminate

				tokVariant.m_nType = TOK_VARIABLE;
				tokVariant.m_Variant = szTemp;
				vLineToks.push_back(tokVariant);
				break;

			case '@':							// Macro variable (@var)
				++iPos;

				ch = szLine[iPos];
				while ( (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
						(ch >= '0' && ch <= '9') || ch == '_')
				{
					szTemp[iPosTemp++] = ch;
					++iPos;
					ch = szLine[iPos];
				}
				szTemp[iPosTemp] = '\0';		// Terminate

				tokVariant.m_nType = TOK_MACRO;
				tokVariant.m_Variant = szTemp;
				vLineToks.push_back(tokVariant);
				break;

			case '"':
				Lexer_String(szLine, iPos, szTemp);
				tokVariant.m_nType = TOK_VARIANT;
				tokVariant.m_Variant = szTemp;
				vLineToks.push_back(tokVariant);
				break;

			case '+':
				tok.m_nType = TOK_PLUS;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '-':
				tok.m_nType = TOK_MINUS;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '/':
				tok.m_nType = TOK_DIV;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '*':
				tok.m_nType = TOK_MULT;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '(':
				tok.m_nType = TOK_LEFTPAREN;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case ')':
				tok.m_nType = TOK_RIGHTPAREN;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '=':
				++iPos;
				ch = szLine[iPos];
				switch (ch)
				{
					case '=':
						tok.m_nType = TOK_EQUALCASE;
						++iPos;
						break;

					default:
						tok.m_nType = TOK_EQUAL;
						break;
				}
				vLineToks.push_back(tok);
				break;

			case ',':
				tok.m_nType = TOK_COMMA;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '&':
				tok.m_nType = TOK_CONCAT;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '[':
				tok.m_nType = TOK_LEFTSUBSCRIPT;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case ']':
				tok.m_nType = TOK_RIGHTSUBSCRIPT;
				vLineToks.push_back(tok);
				++iPos;
				break;

			case '<':
				++iPos;
				ch = szLine[iPos];
				switch (ch)
				{
					case '>':
						tok.m_nType = TOK_NOTEQUAL;
						++iPos;
						break;

					case '=':
						tok.m_nType = TOK_LESSEQUAL;
						++iPos;
						break;

					default:
						tok.m_nType = TOK_LESS;
						break;
				}
				vLineToks.push_back(tok);
				break;

			case '>':
				++iPos;
				ch = szLine[iPos];
				switch (ch)
				{
					case '=':
						tok.m_nType = TOK_GREATEREQUAL;
						++iPos;
						break;

					default:
						tok.m_nType = TOK_GREATER;
						break;
				}
				vLineToks.push_back(tok);
				break;


			default:
				// If nothing else matched must be the start of a number OR keyword/func
				ch = szLine[iPos];
				if ( (ch >= '0' && ch <= '9') || ch == '.' )
				{
					Lexer_Number(szLine, iPos, tokVariant, szTemp);
					vLineToks.push_back(tokVariant);
				}
				else
				{
					// If it is not valid text for a keyword, ignore (should really error here)
					if ( (ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z') &&
						(ch < '0' || ch > '9') && ch != '_') 
					{
						++iPos;					// Ignore
					}
					else
					{
						Lexer_KeywordOrFunc(szLine, iPos, tokVariant, szTemp);
						vLineToks.push_back(tokVariant);
					} 
				}

				break;
		}

	} // End while

	// Add the "end" token
	tok.m_nType = TOK_END;
	tok.m_nCol = (int)strlen(szLine);
	vLineToks.push_back(tok);

} // Lexer()


///////////////////////////////////////////////////////////////////////////////
// Lexer_String()
///////////////////////////////////////////////////////////////////////////////

void App::Lexer_String(const char *szLine, uint &iPos, char *szResult)
{
	uint	iPosTemp = 0;

	// Found a ", skip to the next " (ignore double quotes)

	++iPos;

	while (szLine[iPos] != '\0')
	{
		if (szLine[iPos] == '"')
		{
			if ( szLine[iPos+1] != '"')
			{
				++iPos;
				break;								// String is terminated
			}
			else
			{
				// Double quote, just store a single quote
				szResult[iPosTemp++] = szLine[iPos];
				iPos = iPos + 2;
			}
		}
		else
		{
			szResult[iPosTemp++] = szLine[iPos];
			++iPos;
		}
	}

	// Terminate
	szResult[iPosTemp] = '\0';

} // Lexer_String()


///////////////////////////////////////////////////////////////////////////////
// Lexer_Number()
///////////////////////////////////////////////////////////////////////////////

void App::Lexer_Number(const char *szLine, uint &iPos, Token &rtok, char *szTemp)
{
	uint	iPosTemp = 0;
	bool	bFloat = false;
	//char	*stop = NULL;
	int		nTemp;

	if ( (szLine[iPos] == '0') && (szLine[iPos+1] == 'x' || szLine[iPos+1] == 'X') )
	{
		// Hex number
		iPos += 2;								// Skip "0x"
		while ( (szLine[iPos] >= '0' && szLine[iPos] <= '9') || (szLine[iPos] >= 'a' && szLine[iPos] <= 'f') || (szLine[iPos] >= 'A' && szLine[iPos] <= 'F') )
			szTemp[iPosTemp++] = szLine[iPos++];

		szTemp[iPosTemp] = '\0';					// Terminate
		rtok.m_nType = TOK_VARIANT;
		//rtok.m_Variant = (int)strtol(szTemp, &stop, 16);
		sscanf(szTemp, "%x", &nTemp);			// strtol doesn't cope with 0xffffffff = -1 (although sscanf adds 4KB to the code! :()
		rtok.m_Variant = nTemp;
	}
	else
	{
		// float or integer (stored as doubles in either case)
		while ( (szLine[iPos] >= '0' && szLine[iPos] <= '9') || szLine[iPos] == '.')
			szTemp[iPosTemp++] = szLine[iPos++];

		szTemp[iPosTemp] = '\0';					// Terminate

		rtok.m_nType	= TOK_VARIANT;
		rtok.m_Variant	= atof(szTemp);
	}

} // Lexer_Number()


///////////////////////////////////////////////////////////////////////////////
// Lexer_KeywordOrFunc()
///////////////////////////////////////////////////////////////////////////////

void App::Lexer_KeywordOrFunc(const char *szLine, uint &iPos, Token &rtok, char *szTemp)
{
	int		i;
	char	ch;
	uint	iPosTemp = 0;

	ch = szLine[iPos];
	while ( (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
			(ch >= '0' && ch <= '9') || ch == '_')
	{
		szTemp[iPosTemp++] = ch;
		++iPos;
		ch = szLine[iPos];
	}

	szTemp[iPosTemp] = '\0';					// Terminate

	// Is it a valid keyword?
	i = 0;
	while (i < K_MAX)
	{
		if (!stricmp(m_szKeywords[i], szTemp) )
			break;

		i++;
	}

	if (i != K_MAX)
	{
		rtok.m_nType = TOK_KEYWORD;
		rtok.m_Variant = i;						// Order of enum in script.h must match order of list
	}
	else
	{
		// THIS IS THE BIT MODIFIED FROM THE MAIN AUTOIT SOURCE
		// User defined function -- store the string of the function name
			rtok.m_nType = TOK_FUNCTION;
			rtok.m_Variant = szTemp;
		// END OF MODIFICATION
	}

} // Lexer_KeywordOrFunc()

