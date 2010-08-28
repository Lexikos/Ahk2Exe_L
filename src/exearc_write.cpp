#include "StdAfx.h"
#ifndef _MSC_VER
	#include <stdio.h>
	#include <windows.h>
#endif
#include "lib\exearc_write.h"


int EXEArc_Write::Open(const char *szEXEArcFile, const char *szPwd, UINT nCompressionLevel)
{
	// Based partly on ChangeResources().
	BYTE		*pbuf;
	FILE		*farc;
	struct stat	ST;

	if ( !(farc = fopen(szEXEArcFile, "rb")) )
		return HS_EXEARC_E_OPENEXE;

	fstat(_fileno(farc), &ST);
	if (ST.st_size == 0)
	{
		fclose(farc);
		return HS_EXEARC_E_NOTARC;
	}

	// Allocate enough memory to hold this file
	if ( !(pbuf = new BYTE[ST.st_size]) )
	{
		fclose(farc);
		return HS_EXEARC_E_MEMALLOC;
	}

	fread(pbuf, 1, ST.st_size, farc);
	fclose(farc);

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)pbuf;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(pbuf + dosHeader->e_lfanew);
	
	if (ntHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		// Safe to use HS_EXEArc_Write on this binary.
		if (m_arc = new HS_EXEArc_Write)
		{
			delete [] pbuf; // No longer needed.
			return m_arc->Open(szEXEArcFile, szPwd, nCompressionLevel);
		}
	}
	else
	{
		CResourceEditor *pres;
		// Store script as a resource on x64 since ExeArc is 32-bit.
		if (pres = new CResourceEditor)
		{
			// Init our resource editor with a pointer to our data.
			if (pres->Init(pbuf, ST.st_size))
			{
				m_res = pres;
				m_buf = pbuf;
				return HS_EXEARC_E_OK;
			}
			delete pres;
			delete [] pbuf;
			return HS_EXEARC_E_NOTARC;
		}
	}

	delete [] pbuf;
	return HS_EXEARC_E_MEMALLOC;
}


int EXEArc_Write::FileAdd(const char *szFileName, const char *szFileID)
{
	if (m_arc)
	{
		return m_arc->FileAdd(szFileName, szFileID);
	}
	if (m_res)
	{
		char szTempFileID[MAX_PATH];
		if (strlen(szFileID) >= sizeof(szTempFileID))
			return HS_EXEARC_E_OPENINPUT;
		strcpy(szTempFileID, szFileID);
		_strupr(szTempFileID);

		FILE *fp;
		if ( !(fp = fopen(szFileName, "rb")) )
			return HS_EXEARC_E_OPENINPUT;

		struct stat st;
		BYTE *file_data;
		size_t file_size;
		bool success = false;

		fstat(_fileno(fp), &st);
		file_size = st.st_size;

		if (file_data = new BYTE[file_size])
		{
			if (file_size == fread(file_data, 1, file_size, fp))
			{
				success = m_res->UpdateResource(RT_RCDATA, szTempFileID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), file_data, file_size);
			}
			delete [] file_data;
		}

		fclose(fp);
		if (success)
			return HS_EXEARC_E_OK;
	}
	return -1;
}


int EXEArc_Write::Save(const char *szEXEArcFile)
{
	if (m_res)
	{
		FILE *fp;
		BYTE *data;
		DWORD size;
		size_t written = 0;

		data = m_res->Save(size);
		delete [] m_buf;
		m_buf = data;
		if (fp = fopen(szEXEArcFile, "wb"))
		{
			written = fwrite(data, 1, size, fp);
			fclose(fp);
		}

		if (written != size)
			return HS_EXEARC_E_OPENOUTPUT;
	}
	return HS_EXEARC_E_OK;
}


void EXEArc_Write::Close()
{
	if (m_arc)
		m_arc->Close();
}



EXEArc_Write::~EXEArc_Write()
{
	if (m_res)
		delete m_res;
	if (m_buf)
		delete [] m_buf;
}