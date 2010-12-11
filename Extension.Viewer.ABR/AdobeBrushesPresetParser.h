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
#ifndef _ADOBEBRUSHESPRESETPARSER_H_INCLUDED_
#define _ADOBEBRUSHESPRESETPARSER_H_INCLUDED_

#pragma once

#include "AdobeBrush.h"

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	// 1-byte align
	#pragma pack(push, 1)

	typedef enum tagAdobeBlockSignature {
		AdobeBlockSignatureTail	= 0x4d494238,
		AdobeBlockSignatureSamp	= 0x706d6173,
		AdobeBlockSignatureDesc	= 0x63736564,
	} AdobeBlockSignature;

	typedef struct tagAdobeBlockHeader {
		AdobeBlockSignature	Block;
		AdobeBlockSignature	Type;
		DWORD				Size;
	} AdobeBlockHeader;

	typedef struct tagAdobeBlock {
		AdobeBlockHeader	Header;
		CAutoArrayPtr<BYTE>	Data;
	} AdobeBlock;

	typedef struct tagAdobeRawBlock {
		DWORD dwSize;
		CAutoArrayPtr<BYTE>	Data;
	} AdobeRawBlock;

	#pragma pack(pop)

	//////////////////////////////////////////////////////////////////////////
	//
	class CAdobeBrushesPresetParser {
	public:
		CAdobeBrushesPresetParser  ( WCHAR* roFileName, AdobeBrushDataType nDataTypeRequested, PABRCALLBACK pCallback = 0, void* pParam = 0 );
		CAdobeBrushesPresetParser  ( IStream* pStream, AdobeBrushDataType nDataTypeRequested, PABRCALLBACK pCallback = 0, void* pParam = 0 );
		~CAdobeBrushesPresetParser ();

	public:
		IAdobeBrushes* ReadBrushes ();

	private:
		// Open brushes preset file
		bool Open();
		// Read brushes preset file header
		DWORD ReadHeader();
		// Read from file and returns adobe block
		bool ReadBlock ( AdobeBlock* pBlock );
		bool ReadRawBlock ( AdobeRawBlock* pBlock, DWORD dwSizeRequiested );
		bool CreateBitmapSection(CAdobeBrush* pBrush);

		// Read and process all brushes
		IAdobeBrushes* ReadBrushesV6 ();
		IAdobeBrushes* ReadBrushesV1 ();

		BOOL ShouldTerminate ( DWORD dwTotal, DWORD dwCurrent, BOOL bJobFinished, IAdobeBrushes* pBrushes );

		HANDLE						m_oFile;	// Handle to brush file
		IStream*					m_pStream;	// Handle to stream

		bool						m_bUseFile;	// Fucking shit!

		CAdobeBrushes*				m_oBrushes;

		CAutoArrayPtr<WCHAR>		m_roFileName;	// Filename
		DWORD						m_dwVersion;

		AdobeBrushDataType			m_nDataTypeRequested;

		PABRCALLBACK				m_pCallback;
		ABRPARSER_JOB				m_oJobInfo;
		void*						m_pParam;
	};
};

#endif