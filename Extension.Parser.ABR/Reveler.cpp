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
#include "AdobeBrush.h"
#include "Helpers.h"
#include "Extension.Parser.ABR.h"

#include "Reveler.h"


namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	CParser::CParser() {
		Set ( 0, 0 );
	}

	CParser::CParser( BYTE* pBuffer, DWORD dwBuffer ) {
		Set ( pBuffer, dwBuffer );
	}

	CParser::CParser ( CParser& rOperand ) {
		m_pBuffer = m_pBuffer = rOperand.GetBuffer(0,false);
		m_dwBuffer = m_dwOriginal = rOperand.GetSize();
	}

	CParser::~CParser() {
	}

	//////////////////////////////////////////////////////////////////////////
	//
	void CParser::Set ( BYTE* pBuffer, DWORD dwBuffer ) {
		m_pBuffer = m_pOriginal = pBuffer;
		m_dwBuffer = m_dwOriginal = dwBuffer;
	}

	bool CParser::IsValid () const {
		return m_pBuffer && m_dwBuffer != 0;
	}

	DWORD CParser::GetSize() const {
		return m_dwBuffer;
	}

	BYTE* CParser::GetBuffer(DWORD dwBuffer, bool bStepForward /*= true*/  ) {
		BYTE* pBuffer = m_pBuffer;
		if ( bStepForward ) {
			MoveForward(dwBuffer);
		};
		return pBuffer;
	}

	bool CParser::MoveBackward ( DWORD nSize ) {
		if ( ( m_dwBuffer + nSize ) > m_dwOriginal ) {
			m_dwBuffer = m_dwOriginal;
			m_pBuffer = m_pOriginal;
			return false;
		} else {
			m_pBuffer -= nSize;
			m_dwBuffer += nSize;
			return true;
		};
	}

	bool CParser::MoveForward ( DWORD nSize ) {
		if ( nSize <= m_dwBuffer ) {
			m_dwBuffer -= nSize;
			m_pBuffer += nSize;
			return true;
		} else {
			m_pBuffer += m_dwBuffer;
			m_dwBuffer = 0;
			return false;
		};
	}

	CParser::operator bool () const {
		return IsValid();
	}

	CParser& CParser::operator += (DWORD dwSize ) {
		MoveForward ( dwSize );
		return *this;
	}

	CParser& CParser::operator -= (DWORD dwSize ) {
		MoveBackward ( dwSize );
		return *this;
	}

	template<class T> T CParser::Get ( bool bStepForward /*= true*/ ) {
		if ( IsValid() && m_dwBuffer < sizeof(T) ) {
			return 0;
		};

		T* pTemp = (T*) m_pBuffer;

		if ( bStepForward ) {
			MoveForward (sizeof(T));
		};

		return *pTemp;
	}
	
	DWORD CParser::ToBigEndian (DWORD n) {
		return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
	}

	WORD CParser::ToBigEndian (WORD n) {
		return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	template<> DWORD CParser::Get ( bool bStepForward ) {
		if ( IsValid() && m_dwBuffer >= sizeof(DWORD) ) {
			if ( bStepForward ) {
				MoveForward(sizeof(DWORD));
			};
			return ToBigEndian(*(DWORD*)(m_pBuffer-4));
		};
		return 0;
	}

	template<> WORD CParser::Get ( bool bStepForward ) {
		if ( IsValid() && m_dwBuffer >= sizeof(WORD) ) {
			if ( bStepForward ) {
				MoveForward(sizeof(WORD));
			};
			return ToBigEndian(*(WORD*)(m_pBuffer-2));
		};
		return 0;
	}

	typedef union tagIEEE8 {
		unsigned char b[8];
		unsigned long long l;
		double f;
	} IEEE8;

	inline void ieee8todouble(unsigned char bytes[8], double * IeeeDoubleFloat) {
		IEEE8 u;
		u.b[0] = bytes[7];
		u.b[1] = bytes[6];
		u.b[2] = bytes[5];
		u.b[3] = bytes[4];
		u.b[4] = bytes[3];
		u.b[5] = bytes[2];
		u.b[6] = bytes[1];
		u.b[7] = bytes[0];
		*IeeeDoubleFloat = u.f;
	}

	inline void doubletoieee8(double IeeeDoubleFloat, unsigned char* bytes) {
		IEEE8 u;
		u.f = IeeeDoubleFloat;
		bytes[7] = u.b[0];
		bytes[6] = u.b[1];
		bytes[5] = u.b[2];
		bytes[4] = u.b[3];
		bytes[3] = u.b[4];
		bytes[2] = u.b[5];
		bytes[1] = u.b[6];
		bytes[0] = u.b[7];
	}

	template<> double CParser::Get ( bool bStepForward ) {
		if ( IsValid() && m_dwBuffer >= 8 ) {
			if ( bStepForward ) {
				MoveForward(8);
			};
			double fValue;
			ieee8todouble ( (unsigned char*)(m_pBuffer-8), &fValue );
			return fValue;
		};
		return 0;
	}

	template<> CParser CParser::Get ( bool bStepForward ) {
		if ( IsValid() ) {
			if ( m_dwBuffer >= sizeof(DWORD) ) {
				DWORD dwBlock = ToBigEndian ( *(DWORD*) m_pBuffer );

				if ( dwBlock > ( m_dwBuffer - 4 )  ) {
					dwBlock = m_dwBuffer - 4;
				};

				BYTE* pBlock  = m_pBuffer + 4;

				if ( bStepForward ) {
					MoveForward ( 4 + dwBlock + (( 4 - ( dwBlock & 3 ) ) & 3) );
				};
				return CParser ( pBlock, dwBlock );
			};
		};
		return CParser ( 0, 0 );
	}

	template<> CUCS2String CParser::Get ( bool bStepForward ) {
		// Check for incoming parameters
		if ( this->IsValid() ) {
			// Retrieve pascal string length
			DWORD dwDataLenght = this->Get<DWORD>();

			if ( this->IsValid() ) {

				// Check if there is remaining data available
				if ( ( dwDataLenght * 2 ) > m_dwBuffer ) {
					dwDataLenght =  m_dwBuffer / 2;
				};

				// Allocate required space 
				WCHAR* wTempBuffer =  new WCHAR[dwDataLenght+1];

				if ( wTempBuffer ) {
					// Save pointer
					BYTE* pString = m_pBuffer;
					// Increment reveler
					MoveForward ( dwDataLenght * 2 );

					// Zero memory ;)
					ZeroMemory ( wTempBuffer, sizeof(WCHAR) * (dwDataLenght + 1) );

					for ( DWORD i = 0; i < dwDataLenght; i++ ) {
						wTempBuffer[i] = pString[i*2+1];
					};

					return wTempBuffer;
				};
			};
		};
		// Something gonna blow
		return 0;
	}

	template<> CPascalString CParser::Get ( bool bStepForward ) {
		// Check for incoming parameters
		if ( this->IsValid() ) {
			// Retrieve pascal string length
			BYTE cStringLenght = *m_pBuffer;

			// Check if there is remaining data available
			if ( cStringLenght > (m_dwBuffer - 1) ) {
				cStringLenght = (BYTE)( m_dwBuffer - 1 );
			};

			// Allocate required space 
			WCHAR* wTempBuffer =  new WCHAR[cStringLenght+1];

			if ( wTempBuffer ) {
				// Save pointer
				BYTE* pString = m_pBuffer + 1;
				// Increment reveler
				MoveForward ( cStringLenght + 1 );
				// Zero memory ;)
				ZeroMemory ( wTempBuffer, sizeof(WCHAR) * (cStringLenght + 1) );

				// Convert and return
				if ( MultiByteToWideChar ( CP_ACP, 0, (LPCSTR)pString, cStringLenght, wTempBuffer, cStringLenght + 1 ) == cStringLenght ) {
					return wTempBuffer;
				};

				delete [] wTempBuffer;
			};
		};

		// Something gonna blow
		return 0;
	}

	bool CParser::UpdateImage ( BYTE* pBuffer, DWORD dwBuffer ) {
		return true;
	}


	// Performs RLE8 compression
	DWORD CompressImage ( BYTE* pSrc, DWORD dwWidth, DWORD dwHeight, BYTE* pDst ) {
		// For calculation 
		DWORD dwCompressed = 0;

		// Perform per line encoding
		for ( DWORD y = dwHeight - 1; y != 0 ; y-- ) {
			BYTE* p = pSrc + y * ((((dwWidth * 8) + 31) & ~31) >> 3);
			for ( DWORD x = 0, nCount = 0; x < dwWidth; x += nCount ) {
				// Detect how many similar bytes followed

				for ( nCount=1; ( x + nCount ) < dwWidth; nCount++ )
					if ((nCount == 255) || (*(p+nCount) != *p))
						break;

				if ( nCount == 1 ) {
					// Detect if we can use absolute mode

					for (; ( x + nCount ) < dwWidth; nCount++ )
						if ( nCount == 255 || (*(p+nCount) == *(p+nCount-1) ) )
							break;

					if ( nCount < 4 ) {
						// Encoded mode

						// The first byte specifies the number of consecutive pixels to be drawn 
						// using the color index contained in the second byte. 
						
						nCount = 1;
						if ( pDst ) { *pDst++ = (unsigned char) nCount; *pDst++ = (*p); }; dwCompressed += 2;
						p+=nCount;

					} else {
						// Absolute mode 

						// Absolute mode is signaled by the first byte in the pair being set to zero 
						// and the second byte to a value between 0x03 and 0xFF represents the number 
						// of bytes that follow, each of which contains the color index of a single pixel. 

						nCount--;
						if ( pDst ) { *pDst++ = 0; *pDst++ = (unsigned char) nCount; }; dwCompressed += 2;
						
						for ( DWORD i = 0; i < nCount; i++ ) {
							if ( pDst ) { *pDst++ = *(p+i); }; dwCompressed += 1;
						};

						// Must be aligned on a word boundary
						if ( nCount %2 ) {
							if ( pDst ) { *pDst++ = 0; }; dwCompressed += 1;
						};
						p += nCount;
					};
				} else {
					// Encoded mode

					// The first byte specifies the number of consecutive pixels to be drawn 
					// using the color index contained in the second byte. 

					if ( pDst ) { *pDst++ = (unsigned char) nCount; *pDst++ = (*p); }; dwCompressed += 2;
					p+=nCount;
				}
			}
			// Encoded mode - end of line.
			if ( pDst ) { *pDst++ = 0; *pDst++ = 0; }; dwCompressed += 2;
		}
		// Encoded mode - end of bitmap
		if ( pDst ) { *pDst++ = 0; *pDst++ = 1; }; dwCompressed += 2;
		
		// Return something
		return dwCompressed;
	}


	bool CParser::GetImage ( void* pBuffer, AdobeBrushDataType DataType, DWORD dwWidth, DWORD dwHeight, BYTE cCompression ) {
		if ( pBuffer && dwWidth && dwHeight && ( DataType == AdobeBrushDataTypeDIB || DataType == AdobeBrushDataTypeRAW ) ) {
			// Calculate image size
			if ( DataType == AdobeBrushDataTypeDIB ) {
				AdobeBrushDibDataExt* pBitmap = (AdobeBrushDibDataExt*)pBuffer;

				// Align is (((dwWidth * 8) + 31) & ~31) >> 3:
				BYTE align = ( dwWidth & 3 ) ? 4 - ( dwWidth & 3 ) : 0;
	

				((AdobeBrushRawData*)pBuffer)->dwSizeOfImage = ( dwWidth + align ) * dwHeight; 

				pBitmap->pData.reset ( new BYTE[pBitmap->dwSizeOfImage] );
				pBitmap->pBits = pBitmap->pData;

			} else if ( DataType == AdobeBrushDataTypeRAW ) {
				AdobeBrushRawDataExt* pBitmap = (AdobeBrushRawDataExt*)pBuffer;
				((AdobeBrushRawData*)pBuffer)->dwSizeOfImage = dwWidth * dwHeight; 

				pBitmap->pData.reset ( new BYTE[pBitmap->dwSizeOfImage] );
				pBitmap->pBits = pBitmap->pData;

			};

			BYTE* pRawData = ((AdobeBrushRawData*)pBuffer)->pBits;
			DWORD dwRawData = ((AdobeBrushRawData*)pBuffer)->dwSizeOfImage;

			// Check allocation
			if ( !pRawData ) {
				return false;
			};

			// Copy and decompress
			// No compression, so simple copy
			if ( cCompression == 0 ) {
				// DWORD alignment
				if ( DataType == AdobeBrushDataTypeDIB ) {
					DWORD dwOffset = dwWidth + ( dwWidth & 3 ) ? 4 - ( dwWidth & 3 ) : 0;
					for ( int i = 0; i < dwHeight && IsValid(); i++ ) {
						CopyMemory ( pRawData, GetBuffer(dwWidth), dwWidth );
						pRawData += dwOffset;
					};
				} else {
					CopyMemory ( pRawData, GetBuffer(m_dwBuffer), min(m_dwBuffer,dwRawData) );
				};
			} else if ( cCompression == 1 ) {
				// Compressed bitmap
				BYTE nIncrement		= 1;
				BYTE *pUnpackedData = pRawData;
				long lUnpackedData  = dwRawData;

				CAutoArrayPtr<WORD> oScanLine;
				oScanLine.reset ( new WORD[dwHeight] );

				// Read compressed size for each scan line
				for ( DWORD i = 0; i < dwHeight; i++ ) {
					oScanLine[i] = Get<WORD>();
				};

				// Loop until we're get the number of unpacked bytes we're are expecting
				for (DWORD i = 0; ( i < dwHeight) && ( lUnpackedData > 0 ); i++ ) {
					for ( WORD j = 0; ( j < oScanLine[i] ) && ( lUnpackedData > 0 ); ) {
						// Read the next source byte into n.
						long n = Get<BYTE>();
						j++;

						// BYTE to signed char
						if ( n >= 128 ) {
							n -= 256;
						};

						if ( n < 0 ) { 
							if ( n == -128 ) {
								// if n is -128, noop.
								continue;
							};
							// else if n is between -127 and -1 inclusive, copy the next byte -n+1
							n = -n + 1;
							j++;
							char ch = Get<BYTE>();
							for ( int u = 0; ( u < n ) && ( lUnpackedData > 0 ); u++, pUnpackedData+=nIncrement, lUnpackedData-- ) {
								memset( pUnpackedData, ch, nIncrement );
							};
						} else { 
							// else if n is between 0 and 127 inclusive, copy the next n+1 bytes literally
							for ( int l = 0; ( l < n + 1 ) && ( lUnpackedData > 0 ); l++, j++, pUnpackedData+=nIncrement, lUnpackedData-- ) {
								memset( pUnpackedData, Get<BYTE>(), nIncrement );
							};
						};
					};
					// DWORD alignment
					if ( DataType == AdobeBrushDataTypeDIB ) {
						BYTE align = ( dwWidth & 3 ) ? 4 - ( dwWidth & 3 ) : 0;
						pUnpackedData += align;
						lUnpackedData -= align;
					};
				};

			} else {
				// Fuck off
				return false;
			};

			if ( DataType != AdobeBrushDataTypeDIB ) {
				return true;
			};

			AdobeBrushDibDataExt* pBitmap = (AdobeBrushDibDataExt*)pBuffer;

			pBitmap->pHeader.reset ( new BYTE [sizeof(BITMAPINFOHEADER) + (257 * sizeof(RGBQUAD))] );
			pBitmap->oHeader = (BITMAPINFO*)pBitmap->pHeader.get();

			pBitmap->oHeader->bmiHeader.biSize			= sizeof(BITMAPINFO);  
			pBitmap->oHeader->bmiHeader.biWidth			= dwWidth;  
			pBitmap->oHeader->bmiHeader.biHeight		= -(long)dwHeight; 
			pBitmap->oHeader->bmiHeader.biPlanes		= 1;  
			pBitmap->oHeader->bmiHeader.biBitCount		= 8; 
			pBitmap->oHeader->bmiHeader.biCompression	= BI_RGB;  
			pBitmap->oHeader->bmiHeader.biSizeImage		= pBitmap->dwSizeOfImage;
			pBitmap->oHeader->bmiHeader.biXPelsPerMeter	= 0;  
			pBitmap->oHeader->bmiHeader.biYPelsPerMeter	= 0;  
			pBitmap->oHeader->bmiHeader.biClrUsed		= 256;  
			pBitmap->oHeader->bmiHeader.biClrImportant	= 256;

			RGBQUAD* pPalette = (RGBQUAD*)(pBitmap->pHeader.get() + sizeof(BITMAPINFOHEADER));

			for ( unsigned int i = 1; i <= 256; i++ ) {
				pPalette[i].rgbBlue		= 
				pPalette[i].rgbRed		= 
				pPalette[i].rgbGreen	= 0xFF - (i - 1);
				pPalette[i].rgbReserved = 0;
			};

			DWORD dwSizeRequired = CompressImage ( pBitmap->pBits, dwWidth, dwHeight, 0 );

			if ( dwSizeRequired < pBitmap->dwSizeOfImage ) {
				if ( BYTE* pCompressed = new BYTE[dwSizeRequired] ) {
					CompressImage ( pBitmap->pBits, dwWidth, dwHeight, pCompressed );

					((AdobeBrushRawData*)pBuffer)->dwSizeOfImage	= dwSizeRequired;
					pBitmap->oHeader->bmiHeader.biHeight			= (long)dwHeight; 
					pBitmap->oHeader->bmiHeader.biCompression		= BI_RLE8;  
					pBitmap->oHeader->bmiHeader.biSizeImage			= ((AdobeBrushRawData*)pBuffer)->dwSizeOfImage;
					pBitmap->pBits									= pCompressed;

					pBitmap->pData.reset ( pCompressed );
				};
			};

			if ( pBitmap->hBitmap = CreateDIBitmap( 0, &pBitmap->oHeader->bmiHeader, CBM_INIT, pBitmap->pBits, pBitmap->oHeader, DIB_RGB_COLORS ) ) {
				return true;
			};

			// Reset everything
			pBitmap->pBits			= 0;
			pBitmap->oHeader		= 0;
			pBitmap->dwSizeOfImage	= 0;
			pBitmap->pData.reset();
			pBitmap->pHeader.reset();
		};
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	CProperiesParser::CProperiesParser ( CParser& rOperand ) : CParser ( rOperand ) {
	}

	CProperiesParser::CProperiesParser ( CProperiesParser& rOperand ) : CParser ( rOperand ) {
	}

	void CProperiesParser::Parse () {
		// Simple and pretty
		ParseSection ( m_oProperties, 1 );
	}

	void CProperiesParser::ParseSection ( CStorage<CAdobeProperty>& roRoot, DWORD dwItemsCount ) {
		// For each items in the space
		for ( DWORD i = 0; i < dwItemsCount && IsValid(); i++ ) {
			// Create property
			CAdobeProperty* pProperty = new CAdobeProperty;
			roRoot.Add ( pProperty );

			if ( Get<DWORD>() != 'Objc' ) {
				MoveBackward(4);
				// Retrieve name
				GetName(pProperty);
			} else {
				MoveBackward(4);
			};

			// Retrieve value
			GetValue(pProperty);
		};
	}

	void CProperiesParser::GetName ( CAdobeProperty* pProperty ) {
		if ( pProperty && IsValid() ) {
			DWORD dwName = 0; 

			// Skip trailing spaces
			while ( dwName == 0 && IsValid() ) {
				dwName = Get<DWORD>();
			};

			if ( ( dwName & 0xFFFF0000 ) == 0 ) {
				MoveBackward(1);
				pProperty->m_Name.reset ( Get<CPascalString>() );
			} else {
				pProperty->m_Name.reset ( new WCHAR[5] );

				// Fast conversion
				pProperty->m_Name[0] = HIBYTE(HIWORD(dwName));
				pProperty->m_Name[1] = LOBYTE(HIWORD(dwName));
				pProperty->m_Name[2] = HIBYTE(LOWORD(dwName));
				pProperty->m_Name[3] = LOBYTE(LOWORD(dwName));
				pProperty->m_Name[4] = 0;
			};
		}
	}

	void CProperiesParser::GetSection(CAdobeProperty* pProperty, bool bHasHeader ) {
		if ( pProperty ) {
			if ( bHasHeader ) {
				// Skip still unknown 6 bytes
				MoveForward (6);
			};

			if ( bHasHeader ) {
				// Retrieve section name
				GetName ( pProperty );
			};

			// Retrieve count of sub items
			pProperty->m_dwProperties = Get<DWORD>();

			// Set section ID
			pProperty->m_Type = PropertyTypeObjc;

			// Begin parsing of items
			ParseSection ( pProperty->m_pProperties, pProperty->m_dwProperties );
		}
	}

	void CProperiesParser::GetValue ( CAdobeProperty* pProperty ) {
		if ( pProperty && IsValid() ) {
			// Retrieve value type
			DWORD dwType = Get<DWORD>();
			switch ( dwType ) {
			case 'VlLs':
				GetSection(pProperty, false );
				break;
			case 'Objc':
				GetSection(pProperty, true );
				break;
			case 'TEXT': // 
				pProperty->m_Type = PropertyTypeText;
				pProperty->m_Container.wValue = Get<CUCS2String>();
				break;
			case 'doub':
				pProperty->m_Type = PropertyTypeDoub;
				pProperty->m_Container.fValue = Get<double>();
				break;
			case 'long':
				pProperty->m_Type = PropertyTypeLong;
				pProperty->m_Container.dwValue = Get<DWORD>();
				break;
			case 'UntF':
				pProperty->m_Type = PropertyTypeUntF;
				pProperty->m_Container.ufValue.dwType = Get<DWORD>();
				pProperty->m_Container.ufValue.fValue = Get<double>();
				break;
			case 'bool':
				pProperty->m_Type = PropertyTypeBool;
				pProperty->m_Container.bValue = (Get<BYTE>() == 1);
				break;
			case 'enum':
				{
					pProperty->m_Type = PropertyTypeEnum;
					GetName(pProperty);

					DWORD dwValue = Get<DWORD>();
					if ( ( dwValue & 0x000000FF ) != 0 ) {
						MoveBackward(1);
						pProperty->m_Container.wValue = Get<CPascalString>();
					} else {
						dwValue = Get<DWORD>();

						pProperty->m_Container.wValue = new WCHAR[5];
						// Fast conversion
						pProperty->m_Container.wValue[0] = HIBYTE(HIWORD(dwValue));
						pProperty->m_Container.wValue[1] = LOBYTE(HIWORD(dwValue));
						pProperty->m_Container.wValue[2] = HIBYTE(LOWORD(dwValue));
						pProperty->m_Container.wValue[3] = LOBYTE(LOWORD(dwValue));
						pProperty->m_Container.wValue[4] = 0;
					}
				}
				break;
			default:
				break;
			};
		};
	}
}
