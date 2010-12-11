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

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	CAdobeProperty::CAdobeProperty() {
		m_dwProperties		= 0;
		m_Container.wValue	= 0;
		m_ulRefCount = 1; 
	}

	unsigned long CAdobeProperty::AddRef() {
		InterlockedIncrement(&m_ulRefCount);
		return m_ulRefCount;
	}

	void CAdobeProperty::Release() {
		InterlockedDecrement(&m_ulRefCount);
		// Clean-up
		if ( m_ulRefCount == 0 ) {
			// Release all stored brushes
			for ( DWORD i = 0; i < m_dwProperties; i++ ) {
				if ( m_pProperties.Value(i) ) {
					m_pProperties.Value(i)->Release();
				};
			};

			if ( m_Container.wValue ) {
				delete [] m_Container.wValue;
			};

			// Delete this object
			delete this;
		};
	}

	// Returns property name
	WCHAR* CAdobeProperty::Name() {
		return m_Name;
	}
	
	// Returns property type
	AdobePropertyType CAdobeProperty::Type() {
		return m_Type;
	}
	
	// Returns property value
	AdobeBrushProperty& CAdobeProperty::Value() {
		return m_Container;
	}

	// Returns count of the properties
	DWORD CAdobeProperty::Count() {
		return m_dwProperties;
	}		

	// Determines whether the container has child items
	bool CAdobeProperty::HasChild() {
		return m_dwProperties != 0;
	}
	
	// Retrieve child items
	IAdobeProperty* CAdobeProperty::GetProperty (DWORD dwIndex) {
		if ( dwIndex < Count() ) {
			m_pProperties.Value(dwIndex)->AddRef();
			return m_pProperties.Value(dwIndex);
		} else {
			return 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	CAdobeBrush::CAdobeBrush() {
		m_dwTop = m_dwBottom = m_dwLeft = m_dwRight = 0;
		m_cCompression			= 0;
		m_wDepth				= 0;
		m_dwProperties			= 0;	
		m_ulRefCount			= 1; 
		m_nDataType				= AdobeBrushDataTypeUnk;
	}

	unsigned long CAdobeBrush::AddRef() {
		InterlockedIncrement(&m_ulRefCount);
		return m_ulRefCount;
	}

	void CAdobeBrush::Release() {
		InterlockedDecrement(&m_ulRefCount);

		// Clean-up
		if ( m_ulRefCount == 0 ) {
			// Release all stored brushes
			for ( DWORD i = 0; i < m_dwProperties; i++ ) {
				if ( m_pProperties.Value(i) ) {
					m_pProperties.Value(i)->Release();
				};
			};

			if ( m_nDataType == AdobeBrushDataTypeDIB ) {
				DeleteObject ( m_oDibData.hBitmap );
			};

			// Delete this object
			delete this;
		};
	}

	// Dimensions
	DWORD CAdobeBrush::Top() {
		return m_dwTop;
	}

	DWORD CAdobeBrush::Left() {
		return m_dwLeft;
	}

	DWORD CAdobeBrush::Bottom() {
		return m_dwBottom;
	}

	DWORD CAdobeBrush::Right() {
		return m_dwRight;
	}

	// Returns brush width
	DWORD CAdobeBrush::Width() {
		if ( Right() > Left() ) {
			return Right() - Left();
		} else {
			return 0;
		};
	}

	// Returns brush height
	DWORD CAdobeBrush::Height() {
		if ( Bottom() > Top() ) {
			return Bottom() - Top();
		} else {
			return 0;
		};
	}

		// Returns brush name
	WCHAR* CAdobeBrush::Name() {
		return m_pName;
	}

	// Returns brush type
	AdobeBrushType CAdobeBrush::Type() {
		return m_Type;
	}

	// Returns pointer to raw data
	void* CAdobeBrush::Data() {
		switch ( m_nDataType ) {
		case AdobeBrushDataTypeRAW:
			return &m_oRawData;
		case AdobeBrushDataTypeDIB:
			return &m_oDibData;
		default:
			return 0;
		}
	}

	// Determines whether the container has child items
	bool CAdobeBrush::HasProperties() {
		return m_dwProperties != 0;
	}

	// Returns count of the properties
	DWORD CAdobeBrush::Count() {
		return m_dwProperties;
	}		

	// Retrieve child items
	IAdobeProperty* CAdobeBrush::GetProperty (DWORD dwIndex){
		if ( dwIndex < Count() ) {
			m_pProperties.Value(dwIndex)->AddRef();
			return m_pProperties.Value(dwIndex);
		} else {
			return 0;
		}
	}

	IAdobeProperty* CAdobeBrush::GetProperty(WCHAR* wcsPath, DWORD dwPath, AdobePropertyType enPropType /*= PropertyTypeIgnr*/ ) {
		if ( HasProperties() ) {
			for ( DWORD i = 0; i < Count(); i++ ) {
				if ( IAdobeProperty* pRoot = GetProperty(i) ) {
					if ( IAdobeProperty* pProperty = SearchForProperty ( pRoot, wcsPath, dwPath, enPropType ) ) {
						return pProperty;
					};
					pRoot->Release();
				};
			};
		};
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// 
	CAdobeBrushes::CAdobeBrushes() {
		// Reference counter is already 1
		m_ulRefCount	= 1;
		m_dwBrushes		= 0;
	}

	unsigned long CAdobeBrushes::AddRef() {
		InterlockedIncrement(&m_ulRefCount);
		return m_ulRefCount;
	}

	void CAdobeBrushes::Release() {
		InterlockedDecrement(&m_ulRefCount);

		if ( m_ulRefCount == 0 ) {
			// Release all stored brushes
			for ( DWORD i = 0; i < m_dwBrushes; i++ ) {
				if ( m_pBrushes.Value(i) ) {
					m_pBrushes.Value(i)->Release();
				};
			};
			// Delete this object
			delete this;
		};
	}

	// Returns count of the brushes
	DWORD CAdobeBrushes::Count() {
		return m_dwBrushes;
	}		

	// Retrieve child items
	IAdobeBrush* CAdobeBrushes::Get (DWORD dwIndex) {
		if ( dwIndex < Count() ) {
			m_pBrushes.Value(dwIndex)->AddRef();
			return m_pBrushes.Value(dwIndex);
		} else {
			return 0;
		}
	}

	// Remove specified brush from collection
	BOOL CAdobeBrushes::Remove (DWORD dwIndex) {
		return FALSE;
	}

	BOOL CAdobeBrushes::Remove (IAdobeBrush* pBrush) {
		if ( pBrush ) {
			for ( DWORD i = 0; i < m_dwBrushes; i++ ) {
				if ( m_pBrushes.Value ( i ) == pBrush ) {
					return Remove ( i );
				};
			};
		};
		return FALSE;
	}

	// Insert new brush to collection
	DWORD CAdobeBrushes::Add ( IAdobeBrush* pBrush ) {
		if ( pBrush ) {
			DWORD dwIndex = 0;
			for (; dwIndex < m_dwBrushes; dwIndex++ ) {
				if ( !m_pBrushes.Value (dwIndex) ) {
					break;
				};
			};
/*
			if ( dwIndex < m_dwBrushes ) {
				m_pBrushes.Values[dwIndex] = pBrush;
				pBrush->AddRef();
				return dwIndex;
			} else {
				m_pBrushes.Add ( pBrush );
				return m_dwBrushes++;
			}
*/
		};
		return 0x8FFFFFFF;
	}
}