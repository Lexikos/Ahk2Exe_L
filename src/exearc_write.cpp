#include "StdAfx.h"
#ifndef _MSC_VER
	#include <stdio.h>
	#include <windows.h>
#endif
#include "lib\exearc_write.h"


int EXEArc_Write::Open(const char *szEXEArcFile, const char *szPwd, UINT nCompressionLevel)
{
	FILE		*farc;

	if ( !(farc = fopen(szEXEArcFile, "rb")) )
		return HS_EXEARC_E_OPENEXE;

	IMAGE_DOS_HEADER dosHeader;
	IMAGE_NT_HEADERS ntHeaders;
	if (fread(&dosHeader, sizeof(dosHeader), 1, farc) != 1
		|| fseek(farc, dosHeader.e_lfanew, SEEK_SET) != 0
		|| fread(&ntHeaders, sizeof(ntHeaders), 1, farc) != 1)
	{
		fclose(farc);
		return HS_EXEARC_E_NOTARC;
	}
	fclose(farc);
	
	if (ntHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		// Safe to use HS_EXEArc_Write on this binary.
		if (m_arc = new HS_EXEArc_Write)
		{
			return m_arc->Open(szEXEArcFile, szPwd, nCompressionLevel);
		}
		return HS_EXEARC_E_MEMALLOC;
	}
	else
	{
		HANDLE res;
		// Store script as a resource on x64 since ExeArc is 32-bit.
		if (res = BeginUpdateResource(szEXEArcFile, FALSE))
		{
			m_res = res;
			return HS_EXEARC_E_OK;
		}
		return HS_EXEARC_E_OPENEXE;
	}
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
				success = (UpdateResource(m_res, RT_RCDATA, szTempFileID, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), file_data, file_size) == TRUE);
			}
			delete [] file_data;
		}

		fclose(fp);
		if (success)
			return HS_EXEARC_E_OK;
	}
	return -1;
}



void EXEArc_Write::Close()
{
	if (m_res)
	{
		EndUpdateResource(m_res, FALSE);
		m_res = NULL;
	}
	if (m_arc)
		m_arc->Close();
}



EXEArc_Write::~EXEArc_Write()
{
	if (m_res)
		EndUpdateResource(m_res, TRUE);
}