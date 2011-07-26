#ifndef __HS_EXEARC_WRITE_H
#define __HS_EXEARC_WRITE_H

//
// exearc_write.h
// (c)2002 Jonathan Bennett (jon@hiddensoft.com)
//
// Mostly rewritten by Lexikos to replace EXEarc with Win32 Resource API.
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


class EXEArc_Write
{
private:
	HANDLE			 m_res; // x64
public:
	EXEArc_Write() : m_res(NULL) {}

	int Open(const char *szEXEArcFile, UINT nCompressionLevel);
	int FileAdd(const char *szFileName, const char *szFileID);
	int Save(const char *szEXEArcFile);
	void Close();

	~EXEArc_Write();
};


#endif
