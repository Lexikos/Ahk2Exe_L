#ifndef __VARIANT_H
#define __VARIANT_H

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
// variant_datatype.h
//
// The standalone class for a variant datatype.
//
//
// A variant can be one of these main types:
//  - VAR_STRING (a string)
//  - VAR_DOUBLE (a C double)
//
// The value of a variant can be read by:
//  - .fValue() for the double value
//  - .szValue() for the string value
//  - .nValue() for a 32bit integer version
//
// Any of these values can be read at any time and a suitable conversion will
// be performed.  E.g. if a variant is the STRING value "10" you can use
// .fValue to read 10.0 and .nValue to read 10.  The data conversion is cached
// to speed multiple reads.
///////////////////////////////////////////////////////////////////////////////


// Define the types of variants that we allow
#define VAR_DOUBLE			1					// Double float or 53bit Integer
#define VAR_STRING			2					// String
#define VAR_ARRAY			3					// Array of variants
#define VAR_REFERENCE		4					// Reference to another variant

#define VAR_ITOA_MAX		33					// Maximum returned length of an itoa operation
#define VAR_SUBSCRIPT_MAX	8					// Maximum number of subscripts for an array


class Variant
{
public:
	// Functions
	Variant();									// Constructor
	Variant(const Variant &vOp2);				// Copy constructor
	~Variant();									// Destructor

	Variant&	operator=(const Variant &vOp2);	// Overloaded = for variants
	Variant&	operator=(int nOp2);			// Overloaded = for integers
	Variant&	operator=(double fOp2);			// Overloaded = for floats
	Variant&	operator=(const char *szOp2);	// Overloaded = for strings
	Variant&	operator=(Variant *pOp2);		// Overloaded = for pointers/references
	Variant&	operator+=(Variant &vOp2);		// Overloaded += (addition/concatenation)
	Variant&	operator-=(Variant &vOp2);		// Overloaded -= (subtraction)
	Variant&	operator*=(Variant &vOp2);		// Overloaded *= (multiplication)
	Variant&	operator/=(Variant &vOp2);		// Overloaded /= (division)
	Variant&	operator&=(Variant &vOp2);		// Overloaded &= Bitwise AND
	Variant&	operator|=(Variant &vOp2);		// Overloaded |= Bitwise OR
	Variant&	operator!();						// Overloaded !  logical NOT

	friend bool	operator==(Variant &vOp1, Variant &vOp2);	// Overloaded == for Variants (case insensitive)
	friend bool	operator>(Variant &vOp1, Variant &vOp2);	// Overloaded > for Variants
	friend bool	operator<(Variant &vOp1, Variant &vOp2);	// Overloaded < for Variants
	friend bool operator&&(Variant &vOp1, Variant &vOp2);	// Overloaded && for variants
	friend bool operator||(Variant &vOp1, Variant &vOp2);	// Overloaded || for variants

	bool		StringCompare(Variant &vOp2);	// Compare two strings with case sense

	void		Concat(Variant &vOp2);			// Concats two variants (forces string if possible)
	void		ChangeToDouble(void);			// Convert variant to a FLOAT
	void		ChangeToString(void);			// Convert variant to a STRING

	// Array functions
	void		ArraySubscriptClear(void);					// Reset the current subscript
	bool		ArraySubscriptSetNext(unsigned int iSub);	// Set next subscript
	bool		ArrayDim(void);								// Allocate memory for array
	void		ArrayFree(void);							// Releases all memory in the array and resets
	Variant*	ArrayGetRef(void);							// Returns a pointer to cur array element
	int			ArrayGetBound(unsigned int iSub);			// Returns size of dimension.  returns -1 if not defined


	// Properties
	int			type(void) { return m_nVarType; }	// Returns variant type
	const char	*szValue(void);						// Returns string value
	double		fValue(void);						// Returns float (double) value
	int			nValue(void);						// Returns int value
	Variant		*pValue(void) { return m_pValue;}	// Returns a variant pointer
	bool		isTrue(void);						// Returns true if variant is non-zero

	bool		isInt(void);						// Returns true if an int (whole number)
	bool		isFloat(void);						// Returns true if a float (fractional number)
	bool		isString(void);						// Returns true if a string
	bool		isArray(void);						// Returns true if an array


private:
	// Variables
	int				m_nVarType;					// Type of this variant

	// Single variant variables
	int				m_nValue;					// Value of integer
	bool			m_bnValueAvail;				// nValue has been computed

	double			m_fValue;					// Value of float
	bool			m_bfValueAvail;				// fValue has been computed
	
	char			*m_szValue;					// Value of string
	bool			m_bszValueAvail;			// szValue has been computed

	Variant			*m_pValue;					// Value of pointer (for VAR_REFERENCE)

	// Variant array variables
	unsigned int	m_nArrayElements;						// Actual number of elements in array ([10][10] = 100 elements)
	unsigned int	m_nArrayDimensions;						// Number of dimensions/subscripts
	unsigned int	m_ArraySubscript[VAR_SUBSCRIPT_MAX];	// Subscript details

	unsigned int	m_nArrayDimensionsCur;					// Current number of dimensions
	unsigned int	m_ArraySubscriptCur[VAR_SUBSCRIPT_MAX];	// Current subscript
	Variant			**m_ArrayData;							// Memory area for the array (array of Variant pointers)

	// Functions
	void		ReInit(void);					// Reset a variant to initial values
	void		GenDoubleValue(void);			// Generate internal values as required
	void		GenStringValue(void);			// Generate internal values as required
	void		GenIntegerValue(void);			// Generate internal values as required
	void		InvalidateStringValue(void);	// Invalidate the cached string value
	bool		ArrayBoundsCheck(void);			// Checks if requested subscript is in range
};

///////////////////////////////////////////////////////////////////////////////

#endif
