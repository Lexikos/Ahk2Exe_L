
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
// variant_datatype.cpp
//
// The standalone class for a variant datatype.  
//
///////////////////////////////////////////////////////////////////////////////


// Includes
#include "StdAfx.h"								// Pre-compiled headers

#ifndef _MSC_VER								// Includes for non-MS compilers
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <windows.h>
	#include <limits.h>
#endif

#include "variant_datatype.h"


///////////////////////////////////////////////////////////////////////////////
// Constructor()
///////////////////////////////////////////////////////////////////////////////

Variant::Variant()
{

	//MessageBox(NULL, "Constructor called.", "Variant", MB_OK);

	m_nVarType		= VAR_DOUBLE;				// Type of this variant
	m_fValue		= 0.0;						// Value of float
	m_bfValueAvail	= true;

	m_bnValueAvail	= false;
	m_bszValueAvail	= false;

	m_szValue		= NULL;
	m_pValue		= NULL;	

	// Clear all the array stuff
	m_nArrayElements		= 0;
	m_nArrayDimensions		= 0;				// Number of dimensions/subscripts
	m_nArrayDimensionsCur	= 0;				// Current number of dimensions
	m_ArrayData				= NULL;

} // Variant()


///////////////////////////////////////////////////////////////////////////////
// Copy constructor
///////////////////////////////////////////////////////////////////////////////

Variant::Variant(const Variant &vOp2)
{
	unsigned int	i;

	//MessageBox(NULL, "Copy constructor called.", "Variant", MB_OK);

	m_nVarType	= vOp2.m_nVarType;
	m_nValue	= vOp2.m_nValue;
	m_fValue	= vOp2.m_fValue;
	m_pValue	= vOp2.m_pValue;

	m_bfValueAvail	= vOp2.m_bfValueAvail;
	m_bnValueAvail	= vOp2.m_bnValueAvail;
	m_bszValueAvail	= vOp2.m_bszValueAvail;


	// Copy the string value if required
	if (m_bszValueAvail == true)
	{
		m_szValue = new char[strlen(vOp2.m_szValue)+1];
		strcpy(m_szValue, vOp2.m_szValue);
	}
	else
		m_szValue = NULL;

	// Clear all the array stuff
	m_nArrayElements		= 0;
	m_nArrayDimensions		= 0;
	m_nArrayDimensionsCur	= 0;
	m_ArrayData				= NULL;

	// If the variant is an array, then we have some serious copying to do here!
	if (vOp2.m_ArrayData)
	{
		// Copy the required dimension sizes
		m_nArrayDimensionsCur	= vOp2.m_nArrayDimensions;
		for (i=0; i<m_nArrayDimensionsCur; i++)
			m_ArraySubscriptCur[i] = vOp2.m_ArraySubscript[i];

		// Allocate this array (based on _current_ subscript)
		ArrayDim();

		// Now copy the individual variant elements (each array element is a pointer to a variant)
		for (i=0; i<m_nArrayElements; i++)
		{
			if (vOp2.m_ArrayData[i] != NULL)
			{
				m_ArrayData[i] = new Variant;	// Allocate a variant for this spot
				*(m_ArrayData[i]) = *(vOp2.m_ArrayData[i]);
			}
		}
	}

}


///////////////////////////////////////////////////////////////////////////////
// Destructor()
///////////////////////////////////////////////////////////////////////////////

Variant::~Variant()
{
	//MessageBox(NULL, "Destructor called.", "Variant", MB_OK);

	// Free up the char string if required
	if (m_szValue)
		delete [] m_szValue;

	// Free any array stuff
	ArrayFree();

} // ~Variant()


///////////////////////////////////////////////////////////////////////////////
// ReInit()
// This clears all data/arrays from a variant and resets to base values
///////////////////////////////////////////////////////////////////////////////

void Variant::ReInit(void)
{
	// Free up the char string if required
	if (m_szValue)
	{
		delete [] m_szValue;
		m_szValue	= NULL;
	}
	m_bszValueAvail	= false;


	// Free any array stuff (doesn't destroy current subscript request)
	ArrayFree();

	m_nVarType		= VAR_DOUBLE;				// Type of this variant
	m_fValue		= 0.0;						// Value of float
	m_bfValueAvail	= true;

	m_bnValueAvail	= false;

	m_pValue		= NULL;

} // ReInit()


///////////////////////////////////////////////////////////////////////////////
// szValue()
///////////////////////////////////////////////////////////////////////////////

const char * Variant::szValue(void) 
{
	if (m_bszValueAvail == false)
		GenStringValue();

	return m_szValue;

} // szvalue()


///////////////////////////////////////////////////////////////////////////////
// fValue()
///////////////////////////////////////////////////////////////////////////////

double	Variant::fValue(void)
{
	if (m_bfValueAvail == false)
		GenDoubleValue();

	return m_fValue;

} // fValue()


///////////////////////////////////////////////////////////////////////////////
// nValue()
///////////////////////////////////////////////////////////////////////////////
	
int	Variant::nValue(void) 
{
	if (m_bnValueAvail == false)
		GenIntegerValue();

	return m_nValue;

} // nValue()


///////////////////////////////////////////////////////////////////////////////
// GenDoubleValue()
//
// This function generates internal representations of values.
//
///////////////////////////////////////////////////////////////////////////////

void Variant::GenDoubleValue(void)
{
	// Do we need to generate or does a value already exist?
	if (m_bfValueAvail == true)
		return;

	if (m_nVarType == VAR_STRING)
	{
		m_fValue = atof(m_szValue);
		m_bfValueAvail = true;
	}

} // GenDoubleValue()


///////////////////////////////////////////////////////////////////////////////
// GenStringValue()
//
// This function generates internal representations of values.
//
///////////////////////////////////////////////////////////////////////////////

void Variant::GenStringValue(void)
{
	char	szTemp[1024];						// It is unclear just how many 0000 the sprintf function can add...
	int		iLastDigit, i, iDecimal;

	// Do we need to generate or does a value already exist?
	if (m_bszValueAvail == true)
		return;

	if (m_nVarType == VAR_DOUBLE)
	{
		// Work out the string representation of the number, don't print trailing zeros
		sprintf(szTemp, "%.15f", m_fValue);		// Have at least 15 digits after the . for precision (default is 6)

		// Find the decimal point
		i=0;
		while(szTemp[i] != '.' && szTemp[i] != '\0')
			i++;

		if (szTemp[i] == '.')
		{
			// Found a decimal point (think always will but whatever)
			iDecimal	= i++;
			iLastDigit	= iDecimal;		

			while(szTemp[i] != '\0')
			{
				if (szTemp[i] != '0')
					iLastDigit = i;

				i++;
			}

			// If the decimal point was the last non-zero digit then remove it, otherwise, terminate after
			// the last non-zero digit
			if (iDecimal == iLastDigit)
				szTemp[iDecimal] = '\0';
			else
				szTemp[iLastDigit+1] = '\0';
		}

		
		m_szValue = new char[strlen(szTemp)+1];
		strcpy(m_szValue, szTemp);
	}
	else
	{
		// Oh dear, someone is requesting the szValue() of an array or reference, throw them a blank string
		m_szValue = new char[1];
		m_szValue[0] = '\0';
	}

	m_bszValueAvail = true;

} // GenStringValue()


///////////////////////////////////////////////////////////////////////////////
// GenIntegerValue()
//
// This function generates internal representations of values.
//
///////////////////////////////////////////////////////////////////////////////

void Variant::GenIntegerValue(void)
{
	// Do we need to generate or does a value already exist?
	if (m_bnValueAvail == true)
		return;

	switch (m_nVarType)
	{
		case VAR_DOUBLE:
			m_nValue = (int)m_fValue;
			m_bnValueAvail = true;
			break;

		case VAR_STRING:
			m_nValue = atoi(m_szValue);
			m_bnValueAvail = true;
			break;
	}

} // GenIntegerValue()


///////////////////////////////////////////////////////////////////////////////
// InvalidateStringValue()
//
///////////////////////////////////////////////////////////////////////////////

void Variant::InvalidateStringValue(void)
{
	if (m_bszValueAvail == true)
	{
		m_bszValueAvail = false;
		delete [] m_szValue;
		m_szValue		= NULL;
	}

} // InvalidateIntegerValue()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for variants
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator=(const Variant &vOp2)
{
	unsigned int	i;

	if (this == &vOp2)							// Stop copies to self!
		return *this;

	// ReInit this variant (clears all arrays/strings, resets to default)
	ReInit();

	// First copy the single variant stuff
	// Assign a single variant
	m_nVarType	= vOp2.m_nVarType;
	m_nValue	= vOp2.m_nValue;
	m_fValue	= vOp2.m_fValue;
	m_pValue	= vOp2.m_pValue;

	m_bfValueAvail	= vOp2.m_bfValueAvail;
	m_bnValueAvail	= vOp2.m_bnValueAvail;
	m_bszValueAvail	= vOp2.m_bszValueAvail;


	// Copy the string value if required
	if (m_bszValueAvail == true)
	{
		m_szValue = new char[strlen(vOp2.m_szValue)+1];
		strcpy(m_szValue, vOp2.m_szValue);
	}

	// If the variant is an array, then we have some serious copying to do here!
	if (vOp2.m_ArrayData)
	{
		// Copy the required dimension sizes
		m_nArrayDimensionsCur	= vOp2.m_nArrayDimensions;
		for (i=0; i<m_nArrayDimensionsCur; i++)
			m_ArraySubscriptCur[i] = vOp2.m_ArraySubscript[i];

		// Allocate this array (based on _current_ subscript)
		ArrayDim();

		// Now copy the individual variant elements (each array element is a pointer to a variant)
		for (i=0; i<m_nArrayElements; i++)
		{
			if (vOp2.m_ArrayData[i] != NULL)
			{
				m_ArrayData[i] = new Variant;	// Allocate a variant for this spot
				*(m_ArrayData[i]) = *(vOp2.m_ArrayData[i]);
			}
		}
	}

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for integers
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator=(int nOp2)
{
	// Free any local array data / zero array variables
	ReInit();

	m_nVarType	= VAR_DOUBLE;
	m_nValue	= nOp2;
	m_fValue	= (double)nOp2;

	m_bfValueAvail	= true;
	m_bnValueAvail	= true;
	m_bszValueAvail	= false;

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for floats
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator=(double fOp2)
{
	// Free any local array data / zero array variables
	ReInit();

	m_nVarType	= VAR_DOUBLE;
	m_fValue	= fOp2;

	m_bfValueAvail	= true;
	m_bnValueAvail	= false;
	m_bszValueAvail	= false;

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for strings
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator=(const char *szOp2)
{
	// Free any local array data / zero array variables
	ReInit();

	m_nVarType	= VAR_STRING;

	// Copy the string
	m_szValue = new char[strlen(szOp2)+1];
	strcpy(m_szValue, szOp2);

	m_bfValueAvail	= false;
	m_bnValueAvail	= false;
	m_bszValueAvail	= true;

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator=() for pointers
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator=(Variant *pOp2)
{
	// Free any local array data / zero array variables
	ReInit();

	m_nVarType	= VAR_REFERENCE;
	m_pValue	= pOp2;

	return *this;								// Return this object that generated the call

} // operator=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator+=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator+=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	m_fValue = m_fValue + vOp2.fValue();	// FLOAT
	
	m_bnValueAvail = false;					// Invalidate integer version
	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator+=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator-=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator-=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	m_fValue = m_fValue - vOp2.fValue();	// FLOAT

	m_bnValueAvail = false;					// Invalidate integer version
	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator-=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator*=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator*=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	m_fValue = m_fValue * vOp2.fValue();	// FLOAT

	m_bnValueAvail = false;					// Invalidate integer version
	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator*=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator/=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator/=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	m_fValue = m_fValue / vOp2.fValue();	// FLOAT

	m_bnValueAvail = false;					// Invalidate integer version
	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator/=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator&=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator&=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	// Do bitwise AND
	m_nValue	= (int)m_fValue & (int)vOp2.fValue();
	m_fValue	= (double)m_nValue;

	m_bnValueAvail = true;					// We have integer version

	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator&=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator|=()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator|=(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	// Do bitwise OR
	m_nValue	= (int)m_fValue | (int)vOp2.fValue();
	m_fValue	= (double)m_nValue;

	m_bnValueAvail = true;					// We have integer version

	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator|=()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator!()
///////////////////////////////////////////////////////////////////////////////

Variant& Variant::operator!()
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || m_nVarType == VAR_REFERENCE)
		return *this;

	// Must be of double type first
	ChangeToDouble();

	m_nValue	= !((int)m_fValue);
	m_fValue	= (double)m_nValue;

	m_bnValueAvail = true;					// We have integer version

	InvalidateStringValue();				// Invalidate string version

	return *this;

} // operator!()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator==() for Variants
//
// Is case insensitive with strings
///////////////////////////////////////////////////////////////////////////////

bool operator==(Variant &vOp1, Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (vOp1.m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		vOp1.m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	if (vOp1.m_nVarType == VAR_STRING && vOp2.m_nVarType == VAR_STRING)
	{
		// Do string conparision
		if (!lstrcmpi(vOp1.m_szValue, vOp2.m_szValue) )
			return true;
		else
			return false;
	}
	else
	{
		// FLOAT comparision
		if (vOp1.fValue() == vOp2.fValue())
			return true;
		else
			return false;
	}

} // operator==()


///////////////////////////////////////////////////////////////////////////////
// StringCompare()
//
// Compares 2 strings and is case sensitive
//
///////////////////////////////////////////////////////////////////////////////

bool Variant::StringCompare(Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	// Compare the two string portions - even if they aren't string variants
	// Do string comparision
	if (!lstrcmp(szValue(), vOp2.szValue()) )
		return true;
	else
		return false;

} // StringCompare()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator>() for Variants
///////////////////////////////////////////////////////////////////////////////

bool operator>(Variant &vOp1, Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (vOp1.m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		vOp1.m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	if (vOp1.m_nVarType == VAR_STRING && vOp2.m_nVarType == VAR_STRING)
	{
		// Do string conparision
		if (lstrcmpi(vOp1.m_szValue, vOp2.m_szValue) > 0)
			return true;
		else
            return false;
	}
	else
	{
		// FLOAT comparision
		if (vOp1.fValue() > vOp2.fValue())
			return true;
		else
			return false;
	}

} // operator>()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator<() for Variants
///////////////////////////////////////////////////////////////////////////////

bool operator<(Variant &vOp1, Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (vOp1.m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		vOp1.m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	if (vOp1.m_nVarType == VAR_STRING && vOp2.m_nVarType == VAR_STRING)
	{
		// Do string conparision
		if (lstrcmpi(vOp1.m_szValue, vOp2.m_szValue) < 0)
			return true;
		else
            return false;
	}
	else
	{
		// FLOAT comparision
		if (vOp1.fValue() < vOp2.fValue())
			return true;
		else
			return false;
	}

} // operator<()



///////////////////////////////////////////////////////////////////////////////
// Overloaded operator&&() for Variants
///////////////////////////////////////////////////////////////////////////////

bool operator&&(Variant &vOp1, Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (vOp1.m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		vOp1.m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	if (vOp1.fValue() && vOp2.fValue())
		return true;
	else
		return false;

} // operator&&()


///////////////////////////////////////////////////////////////////////////////
// Overloaded operator||() for Variants
///////////////////////////////////////////////////////////////////////////////

bool operator||(Variant &vOp1, Variant &vOp2)
{
	// Do nothing if any of the operands are an array or a reference
	if (vOp1.m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		vOp1.m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return false;

	if (vOp1.fValue() || vOp2.fValue())
		return true;
	else
		return false;

} // operator||()


///////////////////////////////////////////////////////////////////////////////
// isTrue()
// Returns true if the variant is non-zero
///////////////////////////////////////////////////////////////////////////////

bool Variant::isTrue(void) 
{
	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || m_nVarType == VAR_REFERENCE)
		return false;

	switch (m_nVarType)
	{
		case VAR_DOUBLE:
			if (m_fValue)
				return true;
			break;

		case VAR_STRING:
			if (m_szValue[0] != '\0')
				return true;
			break;
	}

	return false;

} // isTrue()


///////////////////////////////////////////////////////////////////////////////
// isInt()
//
// Returns true if this variant is an integer (whole number)
//
///////////////////////////////////////////////////////////////////////////////

bool Variant::isInt(void) 
{
	__int64 nTemp;
	double	fTemp;

	// Return if not a number
	if (m_nVarType != VAR_DOUBLE)
		return false;

	// Convert to an __int64 and back to a double and compare
	nTemp = (__int64)m_fValue;
	fTemp = (double)nTemp;

	if (fTemp == m_fValue)
		return true;				
	else
		return false;

} // isInt()


///////////////////////////////////////////////////////////////////////////////
// isFloat()
//
// Returns true if this variant is not an integer (fractional)
//
///////////////////////////////////////////////////////////////////////////////

bool Variant::isFloat(void) 
{
	__int64 nTemp;
	double	fTemp;

	// Return if not a number
	if (m_nVarType != VAR_DOUBLE)
		return false;

	// Convert to an __int64 and back to a double and compare
	nTemp = (__int64)m_fValue;
	fTemp = (double)nTemp;

	if (fTemp != m_fValue)
		return true;				
	else
		return false;

} // isFloat()


///////////////////////////////////////////////////////////////////////////////
// isString()
//
// Returns true if this variant is a string
//
///////////////////////////////////////////////////////////////////////////////

bool Variant::isString(void) 
{
	if (m_nVarType == VAR_STRING)
		return true;
	else
		return false;

} // isString()


///////////////////////////////////////////////////////////////////////////////
// isArray()
//
// Returns true if this variant is an array
//
///////////////////////////////////////////////////////////////////////////////

bool Variant::isArray(void) 
{
	if (m_nVarType == VAR_ARRAY)
		return true;
	else
		return false;

} // isArray()


///////////////////////////////////////////////////////////////////////////////
// ChangeToDouble()
///////////////////////////////////////////////////////////////////////////////

void Variant::ChangeToDouble(void)
{
	// Operation only valid for VAR_STRING
	if (m_nVarType != VAR_STRING)
		return;

	// Generate a double value for this string, and then delete the string
	GenDoubleValue();
	m_nVarType		= VAR_DOUBLE;

	// Invalidate our string and int versions
	InvalidateStringValue();
	m_bnValueAvail	= false;

} // ChangeToDouble()


///////////////////////////////////////////////////////////////////////////////
// ChangeToString()
///////////////////////////////////////////////////////////////////////////////

void Variant::ChangeToString(void)
{
	// Operation only valid for VAR_DOUBLE
	if (m_nVarType != VAR_DOUBLE)
		return;

	GenStringValue();
	m_nVarType		= VAR_STRING;
	m_bfValueAvail	= false;
	m_bnValueAvail	= false;

} // ChangeToString()


///////////////////////////////////////////////////////////////////////////////
// Concat()
///////////////////////////////////////////////////////////////////////////////

void Variant::Concat(Variant &vOp2)
{
	char	*szTempString;
	size_t	CombinedLen;

	// Do nothing if any of the operands are an array or a reference
	if (m_nVarType == VAR_ARRAY || vOp2.m_nVarType == VAR_ARRAY ||
		m_nVarType == VAR_REFERENCE || vOp2.m_nVarType == VAR_REFERENCE)
		return;

	// This must be a string type 
	ChangeToString();

	CombinedLen		= strlen(m_szValue) + strlen(vOp2.szValue());
	
	szTempString	= new char[CombinedLen+1];

	strcpy(szTempString, m_szValue);
	strcat(szTempString, vOp2.szValue());

	delete [] m_szValue;
	m_szValue	= szTempString;

	// Invalidate our number values
	m_bfValueAvail	= false;
	m_bnValueAvail	= false;


} // Concat()


///////////////////////////////////////////////////////////////////////////////
// ArraySubscriptClear()
///////////////////////////////////////////////////////////////////////////////

void Variant::ArraySubscriptClear(void)
{
	// Reset the current subscript details
	m_nArrayDimensionsCur	= 0;

} // ArraySubscriptClear()


///////////////////////////////////////////////////////////////////////////////
// ArraySubscriptSetNext()
///////////////////////////////////////////////////////////////////////////////

bool Variant::ArraySubscriptSetNext(unsigned int iSub)
{
	// Any room for another subscript?
	if (m_nArrayDimensionsCur >= VAR_SUBSCRIPT_MAX)
		return false;

	m_ArraySubscriptCur[m_nArrayDimensionsCur] = iSub;
	m_nArrayDimensionsCur++;

	return true;

} // ArraySubscriptSetNext()


///////////////////////////////////////////////////////////////////////////////
// ArrayFree()
///////////////////////////////////////////////////////////////////////////////

void Variant::ArrayFree(void)
{
	unsigned int	i;

	if (m_ArrayData)							// Only delete if neccessary
	{
		// Delete all the individual variants' in the array
		for (i=0; i<m_nArrayElements; i++)
			delete m_ArrayData[i];

		// Delete the array
		delete [] m_ArrayData;
	}

	// Zero everything for possible reuse
	m_nArrayElements		= 0;
	m_nArrayDimensions		= 0;
	m_ArrayData				= NULL;
	//m_nArrayDimensionsCur	= 0;				// DO NOT UNCOMMENT

} // ArrayFree()


///////////////////////////////////////////////////////////////////////////////
// ArrayDim()
//
// Create an array based on the current subscript details
///////////////////////////////////////////////////////////////////////////////

bool Variant::ArrayDim(void)
{
	unsigned int	i;

	// ReInit the variant (also frees array data without messing up current subscript)
	ReInit();

	// Copy the subscripts required and work out the total number of elements
	m_nArrayDimensions	= m_nArrayDimensionsCur;
	m_nArrayElements	= 1;
	for (i=0; i<m_nArrayDimensions; i++)
	{
		m_ArraySubscript[i] = m_ArraySubscriptCur[i];
		m_nArrayElements = m_nArrayElements * m_ArraySubscript[i];

		// Check if the array is too big (4096*4096 elements is 64MB JUST FOR THE TABLE!)
		if ( m_nArrayElements > (4096*4096) )
			return false;						// Abort
	}

	// Create space for the array (effectively an array of Variant POINTERS)
	m_ArrayData = new Variant *[m_nArrayElements];

	// We will allocate array entries when required, so just NULL for now
	for (i=0; i<m_nArrayElements; i++)
		m_ArrayData[i] = NULL;

	// Change type to ARRAY
	m_nVarType = VAR_ARRAY;

	return true;

} // ArrayDim()


///////////////////////////////////////////////////////////////////////////////
// ArrayGetRef()
//
// Returns a pointer to the current array element
//
//[2][2][2]
// a  b  c
// n = 3
//
// s1 s2 s3
// 0  0  0		element 0
// 0  0  1		element 1
// 0  1  0 		element 2
// 0  1  1		element 3
// 1  0  0		element 4
// 1  0  1		element 5
// 1  1  0 		element 6
// 1  1  1		element 7
//
// index  = 0
// index += s1 * b * c
// index += s2 * c
// index += s3
//
// e.g. [1][0][1]
// index  = 0
// index += 1 * 2 * 2
// index += 0 * 2
// index += 1
// index equals element 5
//
///////////////////////////////////////////////////////////////////////////////

Variant* Variant::ArrayGetRef(void)
{
	unsigned int	i, j;
	unsigned int	index, mult;

	// Do we have array data?
	if (m_nVarType != VAR_ARRAY)
		return NULL;

	// Get for correct number of subscripts/size etc
	if (ArrayBoundsCheck() == false)
		return NULL;

	// Convert our multidimensional array to an element in our internal single dimension array
	index = 0;
	for (i=0; i<m_nArrayDimensions; i++)
	{
		mult = 1;
		for (j=i+1; j<m_nArrayDimensions; j++)
			mult = mult * m_ArraySubscript[j];

		mult = mult * m_ArraySubscriptCur[i];
		index += mult;
	}

	// index is the entry we need to return, if required, allocate the entry
	// otherwise return previously allocated entry

	if (m_ArrayData[index] == NULL)
		m_ArrayData[index] = new Variant;

	return m_ArrayData[index];

} // ArrayGet()


///////////////////////////////////////////////////////////////////////////////
// ArrayBoundsCheck()
///////////////////////////////////////////////////////////////////////////////

bool Variant::ArrayBoundsCheck(void)
{
	unsigned int	i;

	if (m_nArrayDimensions != m_nArrayDimensionsCur)
		return false;

	for (i=0; i<m_nArrayDimensions; i++)
	{
		if (m_ArraySubscriptCur[i] >= m_ArraySubscript[i])
			return false;
	}

	// If we get here then the requested subscript is in bounds
	return true;

} // ArrayBoundsCheck()


///////////////////////////////////////////////////////////////////////////////
// ArrayGetBounds(int)
///////////////////////////////////////////////////////////////////////////////

int Variant::ArrayGetBound(unsigned int iSub)
{
	if (iSub == 0)								// If asks for 0 return number of dimensions
		return int(m_nArrayDimensions);
	else if (m_nArrayDimensions < iSub)
		return 0;								// failure
	else
		return m_ArraySubscript[iSub-1];

} // ArrayBoundsCheck()

