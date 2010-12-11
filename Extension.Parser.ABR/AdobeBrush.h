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
#ifndef _ADOBEBRUSH_H_INCLUDED_
#define _ADOBEBRUSH_H_INCLUDED_

#pragma once

#include "Extension.Parser.ABR.h"
#include "Helpers.h"

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	typedef struct tagAdobeBrushRawDataExt { 
		DWORD					dwSize;
		BYTE*					pBits;  
		DWORD					dwSizeOfImage;
		CAutoArrayPtr<BYTE>		pData;  
	} AdobeBrushRawDataExt;

	// To make it smarter ;)
	typedef struct tagAdobeBrushDibDataExt { 
		DWORD				dwSize;
		BYTE*				pBits;  
		DWORD				dwSizeOfImage;
		HBITMAP				hBitmap;
		BITMAPINFO*			oHeader; 
		CAutoArrayPtr<BYTE> pHeader;
		CAutoArrayPtr<BYTE>	pData;  
	} AdobeBrushDibDataExt;

	//////////////////////////////////////////////////////////////////////////
	// adobe brush properties class
	class CAdobeProperty : public IAdobeProperty {
	public:
		CAdobeProperty();

	public:
		// Counter
		virtual unsigned long AddRef();
		virtual void Release();

	public:
		// Returns property name
		virtual WCHAR* Name();
		// Returns property type
		virtual AdobePropertyType Type();
		// Returns property value
		virtual AdobeBrushProperty& Value();

		// Returns count of the items
		virtual DWORD Count();			

		// Determines whether the container has child items
		virtual bool HasChild();
		// Retrieve child item
		virtual IAdobeProperty* GetProperty (DWORD dwIndex);

		CAutoArrayPtr<WCHAR>		m_Name;
		AdobePropertyType			m_Type;
		AdobeBrushProperty			m_Container;
		DWORD						m_dwProperties;	// Pointer to 
		CStorage<CAdobeProperty>	m_pProperties;	// Pointer to properties
		unsigned long				m_ulRefCount; // Reference counter
	};

	class CAdobeBrush : public IAdobeBrush {
	public:
		CAdobeBrush();

	public:
		// Counter
		virtual unsigned long AddRef();
		virtual void Release();

	public:
		// Dimensions
		virtual DWORD Top();			
		virtual DWORD Left();
		virtual DWORD Bottom();
		virtual DWORD Right();

		// Returns brush width
		virtual DWORD Width();
		// Returns brush height
		virtual DWORD Height();

		// Returns brush name
		virtual WCHAR* Name();

		// Returns brush type
		virtual AdobeBrushType Type();

		// Returns pointer to data
		virtual void* Data();

		// Returns count of the child items
		virtual DWORD Count();			

		// Determines whether the container has child items
		virtual bool HasProperties();

		// Retrieve child item
		virtual IAdobeProperty* GetProperty (DWORD dwIndex);
		virtual IAdobeProperty* GetProperty (WCHAR* wcsPath, DWORD dwPath, AdobePropertyType enPropType = PropertyTypeIgnr );


		DWORD						m_dwProperties;	// Pointer to 
		CStorage<CAdobeProperty>	m_pProperties;	// Pointer to properties
		AdobeBrushRawDataExt		m_oRawData;		// Raw data
		AdobeBrushDibDataExt		m_oDibData;		// DIB images
		CAutoArrayPtr<WCHAR>		m_pName;		// Brush name

		// Dimensions
		DWORD				m_dwTop;	// Padding top
		DWORD				m_dwBottom; // Padding bottom
		DWORD				m_dwLeft;	// Padding left
		DWORD				m_dwRight;	// Padding right

		BYTE				m_cCompression;	// Data compression	
		WORD				m_wDepth;		// Data width

		AdobeBrushType		m_Type;			// Brush type
		AdobeBrushDataType	m_nDataType;	// DIB or raw?

		unsigned long	m_ulRefCount; // Reference counter
	};

	//////////////////////////////////////////////////////////////////////////
	//
	class CAdobeBrushes : public IAdobeBrushes {
	public:		
		CAdobeBrushes();

	public:
		// Counter
		virtual unsigned long AddRef();
		virtual void Release();

	public:
		// Returns count of the brushes
		virtual DWORD Count();			

		// Retrieve child items
		virtual IAdobeBrush* Get (DWORD dwIndex);

		// Remove specified brush from collection
		virtual BOOL Remove (DWORD dwIndex);
		virtual BOOL Remove (IAdobeBrush* pBrush);

		// Insert new brush to collection
		virtual DWORD Add ( IAdobeBrush* pBrush );

		DWORD					m_dwBrushes;
		CStorage<CAdobeBrush>	m_pBrushes;
		unsigned long			m_ulRefCount;
	};
}

#endif