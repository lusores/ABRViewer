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
#include "StdAfx.h"
#include "AdobeBrushesPresetParser.h"
#include "AdobeBrush.h"
#include "Reveler.h"
#include "SectionParser.h"

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	CAdobeBrushesPresetParser::CAdobeBrushesPresetParser ( WCHAR* wsFileName, AdobeBrushDataType nDataTypeRequested, PABRCALLBACK pCallback /*= 0*/, void* pParam /*= 0*/ ) : m_oFile(NULL), m_pStream(0), m_pCallback(pCallback), m_pParam(pParam) {
		m_oBrushes = 0;
		m_nDataTypeRequested = nDataTypeRequested;
		
		m_bUseFile = true;
		m_roFileName.reset ( new WCHAR[MAX_PATH] );

		memset ( m_roFileName.get(), 0, sizeof(WCHAR) * MAX_PATH );

		if( wsFileName ) {
			memcpy ( m_roFileName.get(), wsFileName, sizeof(WCHAR) * wcslen(wsFileName) );
		};
	}

	// Ctor
	CAdobeBrushesPresetParser::CAdobeBrushesPresetParser ( IStream* pStream, AdobeBrushDataType nDataTypeRequested, PABRCALLBACK pCallback /*= 0*/, void* pParam /*= 0*/ ) : m_oFile(NULL), m_pStream(pStream), m_pCallback(pCallback), m_pParam(pParam) {
		m_oBrushes = 0;
		m_nDataTypeRequested = nDataTypeRequested;
		m_bUseFile = false;
		if ( pStream ) {
			pStream->AddRef();
		};
	}

	// Dtor
	CAdobeBrushesPresetParser::~CAdobeBrushesPresetParser() {
		if ( m_pStream ) {
			m_pStream->Release();
		};
		if ( m_oFile != INVALID_HANDLE_VALUE ) {
			CloseHandle(m_oFile);
		};
	}

	// Opens ABR file for reading
	bool CAdobeBrushesPresetParser::Open() {
		if ( m_bUseFile ) {
			if ( m_roFileName ) {
				m_oFile = CreateFile ( m_roFileName.get(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
				return m_oFile != INVALID_HANDLE_VALUE;
			};
		} else {
			return m_pStream != NULL;
		};
		return false;
	}

	// Read ABR header
	bool CAdobeBrushesPresetParser::ReadBlock ( AdobeBlock* pBlock ) {
		// Check if file was opened
		if ( pBlock ) {
			// Reset block
			pBlock->Data.reset();

			// Raw block storage
			AdobeRawBlock RawBlock;

			// Read block
			DWORD dwBytesReaded;

			// Read RAW block
			if ( ReadRawBlock ( &RawBlock, sizeof ( AdobeBlockHeader ) ) ) {
				// Copy memory
				CopyMemory ( &pBlock->Header, RawBlock.Data.get(), sizeof ( AdobeBlockHeader ) );
				// Check block signature
				if ( pBlock->Header.Block == AdobeBlockSignatureTail ) {
					// Retrieve and check block size
					pBlock->Header.Size = CParser::ToBigEndian ( pBlock->Header.Size );
					if ( pBlock->Header.Size ) {
						// Allocate memory
						pBlock->Data.reset ( new BYTE[pBlock->Header.Size] );
						// Retrieve entire block
						if ( pBlock->Data ) {
							if ( m_bUseFile ) {
								if ( ReadFile ( m_oFile, pBlock->Data.get(), pBlock->Header.Size, &dwBytesReaded, 0 ) && ( dwBytesReaded == pBlock->Header.Size ) ) {
									return true;
								};
							} else {
								if ( m_pStream ) {
									if ( ( m_pStream->Read ( pBlock->Data.get(), pBlock->Header.Size, &dwBytesReaded) == S_OK ) && ( dwBytesReaded == pBlock->Header.Size ) ) {
										return true;
									};
								};
							};
							// Deallocate memory
							pBlock->Data.reset();
						};
					} else {
						// Empty block is also block
						return true;
					};
				};
			};
		};
		return false;
	}

	// Read ABR header
	bool CAdobeBrushesPresetParser::ReadRawBlock ( AdobeRawBlock* pBlock, DWORD dwSizeRequiested ) {
		// Check if file was opened
		if ( pBlock && dwSizeRequiested ) {
			// Reset block
			pBlock->Data.reset(new BYTE[dwSizeRequiested]);
			if ( pBlock->Data.get() ) {
				if ( m_bUseFile ) {
					if ( ReadFile ( m_oFile, pBlock->Data.get(), dwSizeRequiested, &pBlock->dwSize, 0 ) && ( pBlock->dwSize == dwSizeRequiested ) ) {
						return true;
					};
				} else {
					if ( m_pStream ) {
						if ( ( m_pStream->Read ( pBlock->Data.get(), dwSizeRequiested, &pBlock->dwSize ) == S_OK ) && ( pBlock->dwSize == dwSizeRequiested ) ) {
							return true;
						};
					};
				};
				pBlock->Data.reset();
			};
		};
		return false;
	}

	// Read ABR header
	DWORD CAdobeBrushesPresetParser::ReadHeader() {
		AdobeRawBlock Block;
		if ( ReadRawBlock ( &Block, sizeof ( DWORD ) ) ) {
			return *(DWORD*)(BYTE*)Block.Data;
		};
		return false;
	}

	IAdobeBrushes* CAdobeBrushesPresetParser::ReadBrushes() {
		if ( Open() ) {
			m_dwVersion = ReadHeader();
			if ( LOWORD(m_dwVersion) == 0x0600 ) {
				return ReadBrushesV6();
			} else if ( ( LOWORD(m_dwVersion) == 0x0200 ) || ( LOWORD(m_dwVersion) == 0x0100 ) ) {
				if ( m_bUseFile ) {
					SetFilePointer ( m_oFile, 2, 0, FILE_BEGIN );
				} else {
					if ( m_pStream ) {
						LARGE_INTEGER temp = { 0 };
						temp.LowPart = 2;
						m_pStream->Seek( temp, STREAM_SEEK_SET, NULL );
					};
				};
				return ReadBrushesV1();
			};
		};

		// Report job is finished
		ShouldTerminate ( 0, 0, TRUE, 0 );
		
		return 0;
	};


	IAdobeBrushes* CAdobeBrushesPresetParser::ReadBrushesV1 () {
		// Array of adobe brushes
		CAdobeBrushes* pBrushes = new CAdobeBrushes();
		bool bSuccess = true;

		// Local helpers
		AdobeRawBlock			RawBlock;
		CParser				Reveler;

		// Storage for brush name
		CAdobeBrush*			pBrush;

		WORD					wItemsInFile = 0;

		// Begin recognition
		// Read brushes counter
		if ( ReadRawBlock ( &RawBlock, sizeof(WORD) ) ) {
			// Set section
			Reveler.Set ( RawBlock.Data, RawBlock.dwSize );

			// Detect how many brushes in file
			wItemsInFile = Reveler.Get<WORD>();

			// If brushes exists
			if ( wItemsInFile ) {
				// For each item in the space
				for ( WORD nItems = 0; nItems < wItemsInFile && !ShouldTerminate(wItemsInFile,nItems+1, FALSE, pBrushes ); nItems++ ) {
					// Create brush
					if ( pBrush = new CAdobeBrush ) {
						// Read brush raw data size
						if ( ReadRawBlock ( &RawBlock, 6 )) {
							// Set walker
							Reveler.Set ( RawBlock.Data, RawBlock.dwSize );
							pBrush->m_Type = Reveler.Get<WORD>() == 2 ? AdobeBrushTypeSampled : AdobeBrushTypeComputed;

							DWORD dwBlockSize = Reveler.Get<DWORD>();

							if ( pBrush->m_Type == 2 ) { // Sampled brush
								if ( ReadRawBlock ( &RawBlock, dwBlockSize ) ) {
									// Here we're
									Reveler.Set ( RawBlock.Data, RawBlock.dwSize );

									if ( LOWORD(m_dwVersion) == 0x0200 ) {
										Reveler += 6;
										pBrush->m_pName.reset ( Reveler.Get<CUCS2String>() );
										Reveler.Get<BYTE>();	// Antialiace
									} else {
										Reveler.Get<DWORD>();	// Misc
										Reveler.Get<WORD>();	// Spacing
										Reveler.Get<BYTE>();	// Antialiace
									};

									pBrush->m_dwTop		= Reveler.Get<WORD>();
									pBrush->m_dwLeft	= Reveler.Get<WORD>();
									pBrush->m_dwBottom	= Reveler.Get<WORD>();
									pBrush->m_dwRight	= Reveler.Get<WORD>();

									if ( pBrush->Width() && pBrush->Height() ) {
										pBrush->m_dwTop		= Reveler.Get<DWORD>();
										pBrush->m_dwLeft	= Reveler.Get<DWORD>();
										pBrush->m_dwBottom	= Reveler.Get<DWORD>();
										pBrush->m_dwRight	= Reveler.Get<DWORD>();
									} else {
										Reveler += ( 4 * sizeof(DWORD) );
									};

									pBrush->m_wDepth = Reveler.Get<WORD>();
									pBrush->m_cCompression = Reveler.Get<BYTE>();

									if ( Reveler.IsValid() && pBrush->Width() && pBrush->Height() ) {

										pBrush->m_nDataType = m_nDataTypeRequested;
										switch ( pBrush->m_nDataType ) {
										case AdobeBrushDataTypeRAW:
											Reveler.GetImage ( (BYTE*)&pBrush->m_oRawData, pBrush->m_nDataType, pBrush->Width(), pBrush->Height(), pBrush->m_cCompression );
											break;
										case AdobeBrushDataTypeDIB:
											Reveler.GetImage ( (BYTE*)&pBrush->m_oDibData, pBrush->m_nDataType, pBrush->Width(), pBrush->Height(), pBrush->m_cCompression );
											break;
										default:
											break;
										};

										if ( pBrush->m_pName == 0 ) {
											pBrush->m_pName.reset ( new WCHAR[50] );
											WCHAR Name[] = L"Sampled brush";
											memcpy ( pBrush->m_pName, Name, sizeof(WCHAR) * sizeof(Name) );
										};

										pBrushes->m_pBrushes.Add ( pBrush );
										pBrushes->m_dwBrushes++;

										pBrush->AddRef();
									};
								} else {
									bSuccess = false;
									break;
								};
							} else {
								bSuccess = false;
								break;
							};
						} else if ( pBrush->m_Type == 1 ) {
							// Skip 14 bytes of computed brush definition
							if ( !ReadRawBlock ( &RawBlock, 14 ) ) {
								bSuccess = false;
								break;
							};
						} else {
							bSuccess = false;
							break;
						};

						pBrush->Release();
					};
				};
			};
		};
		// Clean-up
		if ( !bSuccess && pBrushes ) {
			pBrushes->Release();
			pBrushes = 0;
		};

		// Pass data to callback
		ShouldTerminate ( wItemsInFile, wItemsInFile, TRUE, pBrushes );
		return pBrushes;
	}

	IAdobeBrushes* CAdobeBrushesPresetParser::ReadBrushesV6 () {
		// Array of adobe brushes
		CAdobeBrushes* pBrushes = new CAdobeBrushes();
		bool bSuccess			= true;
		CAdobeBrush* pBrush		= 0;

		// Local variables
		DWORD	dwBrushCount	= 1;

		// Storage for block
		CAutoPtr<AdobeBlock> Block ( new AdobeBlock );
		while ( ReadBlock( Block ) ) {
			// Begin block recognition
			CParser SectionWalker ( Block->Data, Block->Header.Size );

			if ( Block->Header.Type == AdobeBlockSignatureDesc ) {
				SectionWalker += 26;
				CProperiesParser parser ( SectionWalker );
				parser.Parse ();

				// for each
				for ( DWORD dwItem = 0; dwItem < parser.m_oProperties.Count(); dwItem++ ) {
					if ( CAdobeProperty* pItem = parser.m_oProperties.Value(dwItem) ) {
						if ( pItem->m_Name && CompareString ( LOCALE_NEUTRAL, 0, pItem->m_Name, -1, L"Brsh", 4 ) ) {
							for ( DWORD dwVIL = 0; dwVIL < pItem->Count(); dwVIL++ ) {
								WCHAR wcsName[] = L"sampledBrush/sampledData";
								IAdobeProperty* pBrushID = SearchForProperty(pItem->m_pProperties.Value(dwVIL), wcsName, sizeof(wcsName) / sizeof(WCHAR), PropertyTypeText );

								if ( pBrushID ) {
									// Search for brush
									for ( DWORD dwBrush = 0; dwBrush < pBrushes->Count(); dwBrush++ ) {
										if ( CompareString ( LOCALE_NEUTRAL, 0, pBrushes->m_pBrushes.Value(dwBrush)->Name(), -1, pBrushID->Value().wValue, -1 ) == CSTR_EQUAL )  {
											pBrushes->m_pBrushes.Value(dwBrush)->m_pProperties.Add ( pItem->m_pProperties.Value(dwVIL) );
											pBrushes->m_pBrushes.Value(dwBrush)->m_dwProperties = pBrushes->m_pBrushes.Value(dwBrush)->m_pProperties.Count();
											pItem->AddRef();
											break;
										};
									};

									pBrushID->Release();
								} else {
									WCHAR wcsComputedName[] = L"Nm  ";
									pBrushID = SearchForProperty(pItem->m_pProperties.Value(dwVIL), wcsComputedName, sizeof(wcsComputedName) / sizeof(WCHAR), PropertyTypeText );

									if ( pBrushID && pBrushID->Value().wValue ) {

										DWORD dwName = (DWORD)wcslen ( pBrushID->Value().wValue );
										if ( dwName ) {
											// Insert computed or lost brush
											CAdobeBrush* pBrush	= new CAdobeBrush;
											pBrush->m_Type = AdobeBrushTypeComputed;
											pBrushes->m_pBrushes.Add(pBrush);
											pBrushes->m_dwBrushes++;

											pBrush->m_pName.reset ( new WCHAR[dwName+1] );
											memcpy ( pBrush->m_pName.get(), pBrushID->Value().wValue, sizeof(WCHAR) * (dwName+1) );


											pBrush->m_pProperties.Add ( pItem->m_pProperties.Value(dwVIL) );
											pBrush->m_dwProperties = pBrush->m_pProperties.Count();
											pItem->AddRef();
										};

										pBrushID->Release();
									};
								};
							};
						};
						pItem->Release();
					};
				};
			} else if ( Block->Header.Type == AdobeBlockSignatureSamp ) { // This is brush samples
				// Brushes walker
				while ( CParser BrushesWalker = SectionWalker.Get<CParser>() ) {
					if ( ShouldTerminate ( dwBrushCount, dwBrushCount++, FALSE, pBrushes ) ) {
						return pBrushes;
					};
					// Create brush and append to storages
					if ( pBrush = new CAdobeBrush ) {
						// Read brush ID
						pBrush->m_pName.reset ( BrushesWalker.Get<CPascalString>() );

						CParser LayersWalker;

						if ( HIWORD(m_dwVersion) > 0x0100 ) {
							WORD wLayersCount = BrushesWalker.Get<WORD>();
							// Skip unknown three bytes
							BrushesWalker += ( 3 * sizeof (WORD));
							if ( LayersWalker = BrushesWalker.Get<CParser>() ) {
								LayersWalker += 252;
							};
						} else {
							LayersWalker = BrushesWalker;
							// Skip unknown three bytes
							LayersWalker += 10;
						};

						pBrush->m_dwTop			= LayersWalker.Get<DWORD>();
						pBrush->m_dwLeft		= LayersWalker.Get<DWORD>();
						pBrush->m_dwBottom		= LayersWalker.Get<DWORD>();
						pBrush->m_dwRight		= LayersWalker.Get<DWORD>();
						pBrush->m_wDepth		= LayersWalker.Get<WORD>();
						pBrush->m_cCompression	= LayersWalker.Get<BYTE>();

						if ( LayersWalker.IsValid() && pBrush->Width() && pBrush->Height() ) {
							pBrush->m_nDataType = m_nDataTypeRequested;
							switch ( pBrush->m_nDataType ) {
							case AdobeBrushDataTypeRAW:
								LayersWalker.GetImage ( (BYTE*)&pBrush->m_oRawData, pBrush->m_nDataType, pBrush->Width(), pBrush->Height(), pBrush->m_cCompression );
								break;
							case AdobeBrushDataTypeDIB:
								LayersWalker.GetImage ( (BYTE*)&pBrush->m_oDibData, pBrush->m_nDataType, pBrush->Width(), pBrush->Height(), pBrush->m_cCompression );
								break;
							default:
								break;
							};

							pBrush->m_Type = AdobeBrushTypeSampled;
							pBrushes->m_pBrushes.Add ( pBrush );
							pBrushes->m_dwBrushes++;

							pBrush->AddRef();
						};

						pBrush->Release();
					};
				};
			};
		};

		// Pass data to callback
		ShouldTerminate ( dwBrushCount, dwBrushCount, TRUE, pBrushes );

		return pBrushes;
	}

	BOOL CAdobeBrushesPresetParser::ShouldTerminate ( DWORD dwTotal, DWORD dwCurrent, BOOL bJobFinished, IAdobeBrushes* pBrushes ) {
		if ( m_pCallback ) {
			m_oJobInfo.dwBrushesTotal	= dwTotal;
			m_oJobInfo.dwCurrentBrush	= dwCurrent;
			m_oJobInfo.bJobFinished		= bJobFinished;
			m_oJobInfo.pBrushes			= pBrushes;
			m_oJobInfo.pParam			= m_pParam;
			return !(m_pCallback)(&m_oJobInfo);
		} else {
			return FALSE;
		};
	}
}
