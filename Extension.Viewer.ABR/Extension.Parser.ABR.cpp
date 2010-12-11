/*
	 copyright (c) 1996 - 2008 Ivan Varzar. lusores@gmail.com
	 All rights reserved.

	 Redistribution and use in source and binary forms, with or without
	 modification, are permitted provided that the following conditions
	 are met:
	 1. Redistributions of source code must retain the above copyright
	    notice, this list of conditions and the following disclaimer.
	 2. Redistributions in binary form must reproduce the above copyright
	    notice, this list of conditions and the following disclaimer in the
	    documentation and/or other materials provided with the distribution.
	
	 THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS
	 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	 DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT,
	 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
	 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
	 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
	 IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	 POSSIBILITY OF SUCH DAMAGE.
*/
#include "stdafx.h"
#include "AdobeBrushesPresetParser.h"
#include "Extension.Parser.ABR.h"

namespace BigBrotherAndy {

	typedef struct tagABRWORKERTHREADPARAMS {
		void*						pStream;
		AdobeBrushDataStreamType	nStreamType;
		AdobeBrushDataType			nDataTypeRequested;
		PABRCALLBACK				pCallback;
		HANDLE						hWaitHandle;
		void*						pParam;
	} ABRWORKERTHREADPARAMS, *PABRWORKERTHREADPARAMS;

	DWORD WINAPI ReaderProc( LPVOID lpParam )  { 
		PABRWORKERTHREADPARAMS pParams = (PABRWORKERTHREADPARAMS) lpParam;

		if ( pParams->nStreamType == AdobeBrushDataTypeFile ) {
			CAdobeBrushesPresetParser Reader ( (WCHAR*) pParams->pStream, pParams->nDataTypeRequested, pParams->pCallback, pParams->pParam );
			
			SetEvent ( pParams->hWaitHandle );
			Reader.ReadBrushes();
		
		} else if ( pParams->nStreamType == AdobeBrushDataTypeIStream ) {
			CAdobeBrushesPresetParser Reader ( (IStream*) pParams->pStream, pParams->nDataTypeRequested, pParams->pCallback, pParams->pParam );
		
			SetEvent ( pParams->hWaitHandle );
			Reader.ReadBrushes();

		} else {

			SetEvent ( pParams->hWaitHandle );
		};

		return 0; 
	}

	IAdobeBrushes* ReadAdobePhotoshopBrushFile ( void* pStream, AdobeBrushDataStreamType nStreamType /*= AdobeBrushDataTypeFile*/, AdobeBrushDataType nDataTypeRequested /*= AdobeBrushDataTypeDIB*/, PABRCALLBACK pCallback /*= 0*/, void* pParam /*= 0*/ ) {
		if ( pCallback ) {
			DWORD dwThreadID;

			ABRWORKERTHREADPARAMS parameters;
			parameters.pStream				= pStream;
			parameters.nStreamType			= nStreamType;
			parameters.nDataTypeRequested	= nDataTypeRequested;
			parameters.pCallback			= pCallback;
			parameters.pParam				= pParam;

			if ( parameters.hWaitHandle = CreateEvent ( 0, 0, 0, 0 ) ) {
				if ( HANDLE hThread = CreateThread ( 0, 0, ReaderProc, &parameters, 0, &dwThreadID ) ) {
					if ( WaitForSingleObject ( parameters.hWaitHandle, 60000 ) == WAIT_OBJECT_0 ) {
						CloseHandle ( parameters.hWaitHandle );
						CloseHandle ( hThread );
						return 0;
					}
					CloseHandle ( hThread );
				};
				CloseHandle ( parameters.hWaitHandle );
			};

			ABRPARSER_JOB jobInfo;

			jobInfo.dwBrushesTotal	= 0;
			jobInfo.dwCurrentBrush	= 0;
			jobInfo.bJobFinished	= TRUE;
			jobInfo.pBrushes		= 0;

			(pCallback)(&jobInfo);

		} else {

			if ( nStreamType == AdobeBrushDataTypeFile ) {
				CAdobeBrushesPresetParser Reader ( (WCHAR*) pStream, nDataTypeRequested );
				return Reader.ReadBrushes();
			} else if ( nStreamType == AdobeBrushDataTypeIStream ) {
				CAdobeBrushesPresetParser Reader ( (IStream*) pStream, nDataTypeRequested );
				return Reader.ReadBrushes();
			}

		}

		return 0;
	}

	IAdobeProperty* SearchForProperty ( IAdobeProperty* pRoot, WCHAR* wcsPath, DWORD dwPath, AdobePropertyType PropType ) {
		IAdobeProperty* pResult = 0;
		if ( pRoot && wcsPath && dwPath ) {
			// Create local copy of name
			CAutoArrayPtr<WCHAR> wcsName;
			DWORD dwName = dwPath;

			wcsName.reset ( new WCHAR[dwName] );
			memcpy ( wcsName.get(), wcsPath, dwName * sizeof(WCHAR) );

			DWORD dwChild = dwName;

			// Search for level delimiter
			WCHAR* wcsChild = wcsName.get();
			while ( *wcsChild && *wcsChild != '/' ) { 
				wcsChild++;
				dwChild--;
			};

			// If delimiter was found
			if ( *wcsChild == '/' ) { 
				wcsChild++;
				dwName -= dwChild;
				dwChild--;
			} else {
				wcsChild = 0;
				dwChild = 0;
				dwName = dwPath;
			};

			for ( DWORD dwValue = 0; dwValue< pRoot->Count(); dwValue++ ) {
				if ( IAdobeProperty* pChild = pRoot->GetProperty(dwValue) ) {
					if ( CompareString ( LOCALE_NEUTRAL, 0, pChild->Name(), -1, wcsName, dwName ) == CSTR_EQUAL )  {
						if ( wcsChild && pChild->HasChild() ) {
							if ( pResult = SearchForProperty ( pChild, wcsChild, dwChild, PropType ) ) {
								pChild->Release();
								break;
							};
						} else {
							if ( PropType == PropertyTypeIgnr || PropType == pChild->Type() ) {
								pResult = pChild;
								break;
							};
						};
					};
					pChild->Release();
				};
			};
		};
		return pResult;
	}

	//////////////////////////////////////////////////////////////////////////
	//

	DWORD WINAPI WriterProc( LPVOID lpParam )  { 
		PABRWORKERTHREADPARAMS pParams = (PABRWORKERTHREADPARAMS) lpParam;

/*
				if ( pParams->nStreamType == AdobeBrushDataTypeFile ) {
					CAdobeBrushesPresetParser Reader ( (WCHAR*) pParams->pStream, pParams->nDataTypeRequested, pParams->pCallback, pParams->pParam );
		
					SetEvent ( pParams->hWaitHandle );
					Reader.ReadBrushes();
		
				} else if ( pParams->nStreamType == AdobeBrushDataTypeIStream ) {
					CAdobeBrushesPresetParser Reader ( (IStream*) pParams->pStream, pParams->nDataTypeRequested, pParams->pCallback, pParams->pParam );
		
					SetEvent ( pParams->hWaitHandle );
					Reader.ReadBrushes();
		
				} else*/
		 {

			SetEvent ( pParams->hWaitHandle );
		};

		return 0; 
	}

	DWORD SaveAdobePhotoshopBrushFile ( void* pStream, IAdobeBrushes* pBrushes, AdobeBrushDataStreamType nStreamType /*= AdobeBrushDataTypeFile*/, PABRCALLBACK pCallback /*= 0*/, void* pParam /*= 0*/ ) {
		if ( !pBrushes ) {
			if ( pCallback ) {
				ABRPARSER_JOB jobInfo;

				jobInfo.dwBrushesTotal	= 0;
				jobInfo.dwCurrentBrush	= 0;
				jobInfo.bJobFinished	= TRUE;
				jobInfo.pBrushes		= 0;

				(pCallback)(&jobInfo);
			};
			return 0;
		};

		DWORD dwBrushesTotal = pBrushes->Count();

		if ( pCallback ) {
			DWORD dwThreadID;

			ABRWORKERTHREADPARAMS parameters;
			parameters.pStream				= pStream;
			parameters.nStreamType			= nStreamType;
			parameters.pCallback			= pCallback;
			parameters.pParam				= pParam;

			if ( parameters.hWaitHandle = CreateEvent ( 0, 0, 0, 0 ) ) {
				if ( HANDLE hThread = CreateThread ( 0, 0, WriterProc, &parameters, 0, &dwThreadID ) ) {
					if ( WaitForSingleObject ( parameters.hWaitHandle, 60000 ) == WAIT_OBJECT_0 ) {
						CloseHandle ( parameters.hWaitHandle );
						CloseHandle ( hThread );
						return 0;
					}
					CloseHandle ( hThread );
				};
				CloseHandle ( parameters.hWaitHandle );
			};

			ABRPARSER_JOB jobInfo;

			jobInfo.dwBrushesTotal	= dwBrushesTotal;
			jobInfo.dwCurrentBrush	= 0;
			jobInfo.bJobFinished	= TRUE;
			jobInfo.pBrushes		= 0;

			(pCallback)(&jobInfo);

		} else {

/*
			if ( nStreamType == AdobeBrushDataTypeFile ) {
				CAdobeBrushesPresetParser Reader ( (WCHAR*) pStream, nDataTypeRequested );
				return Reader.ReadBrushes();
			} else if ( nStreamType == AdobeBrushDataTypeIStream ) {
				CAdobeBrushesPresetParser Reader ( (IStream*) pStream, nDataTypeRequested );
				return Reader.ReadBrushes();
			}
*/

		}

		return 0;
	}
}

#ifndef EXTENSION_PARSER_ABR_STATIC
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved) {
	UNREFERENCED_PARAMETER(lpvReserved);
	UNREFERENCED_PARAMETER(dwReason);
	return TRUE;
}
#endif