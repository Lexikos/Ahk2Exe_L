#ifndef __HS_EXEARC_WRITE_H
#define __HS_EXEARC_WRITE_H

//
// exearc_write.h
// (c)2002 Jonathan Bennett (jon@hiddensoft.com)
//
// Version: v3
//
// STANDALONE CLASS
//

#define HS_EXEARC_FILEVERSION	3				// File wrapper version

// Error codes
#define HS_EXEARC_E_OK				0
#define HS_EXEARC_E_OPENEXE			1
#define HS_EXEARC_E_OPENINPUT		2		
#define HS_EXEARC_E_NOTARC			3
#define HS_EXEARC_E_BADVERSION		4
#define HS_EXEARC_E_BADPWD			5
#define HS_EXEARC_E_FILENOTFOUND	6
#define HS_EXEARC_E_OPENOUTPUT		7
#define HS_EXEARC_E_MEMALLOC		8

#define HS_EXEARC_MAXPWDLEN			256


// Table of contents linked list
typedef struct HS_EXEArc_TOC
{
	char	szFileID[_MAX_PATH+1];					// File ID (usually filename)
	struct 	HS_EXEArc_TOC *lpNext;					// Next entry in linked list

} _HS_EXEArc_TOC;


class HS_EXEArc_Write
{
public:
	// Functions
	int		Open(const char *szEXEArcFile, const char *szPwd, UINT nCompressionLevel);
	void	Close(void);
	int		FileAdd(const char *szFileName, const char *szFileID);

private:
	// Variables
	FILE	*m_fEXE;
	ULONG	m_nArchivePtr;
	char	m_szPwd[HS_EXEARC_MAXPWDLEN+1];
	UINT	m_nPwdHash;
	UINT	m_nCompressionLevel;

	HS_EXEArc_TOC	*m_lpTOC;					// Pointer to the table of contents

	// Functions
	void	Encrypt(UCHAR *bData, UINT nLen, UINT nSeed);
	bool	FileGetTime(const char *szFileName, FILETIME &ftCreated, FILETIME &ftModified);

};


#include "ResourceEditor.h"

// Abstraction for x64:
class EXEArc_Write
{
private:
	HS_EXEArc_Write *m_arc; // x86
	CResourceEditor *m_res; // x64
	BYTE *m_buf;
public:
	EXEArc_Write() : m_arc(NULL), m_res(NULL), m_buf(NULL) {}

	int Open(const char *szEXEArcFile, const char *szPwd, UINT nCompressionLevel);
	int FileAdd(const char *szFileName, const char *szFileID);
	int Save(const char *szEXEArcFile);
	void Close();

	~EXEArc_Write();
};


#endif
