#include "StdAfx.h"
#ifndef _MSC_VER
	#include <stdio.h>
	#include <windows.h>
#endif
#include "lib\exearc_write.h"


int EXEArc_Write::Open(const char *szEXEArcFile, UINT nCompressionLevel)
{
	HANDLE res;
	if (res = BeginUpdateResource(szEXEArcFile, FALSE))
	{
		m_res = res;
		return HS_EXEARC_E_OK;
	}
	return HS_EXEARC_E_OPENEXE;
}


int EXEArc_Write::FileAdd(const char *szFileName, const char *szFileID)
{
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
}



EXEArc_Write::~EXEArc_Write()
{
	if (m_res)
		EndUpdateResource(m_res, TRUE);
}