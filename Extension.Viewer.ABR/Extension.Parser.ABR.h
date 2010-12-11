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
#ifndef _IADOBEBRUSH_H_INCLUDED_
#define _IADOBEBRUSH_H_INCLUDED_

#pragma once

#ifdef EXTENSION_PARSER_ABR_STATIC
#define EXT_EXPORTS
#else
#ifdef EXTENSIONPREVIEWHANDLERABR_EXPORTS
#define EXT_EXPORTS __declspec(dllexport)
#else
#define EXT_EXPORTS __declspec(dllimport)
#endif
#endif

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	enum AdobeBrushDataType {
		AdobeBrushDataTypeUnk = -1,
		AdobeBrushDataTypeDIB = 1,
		AdobeBrushDataTypeRAW = 2,
	};

	enum AdobeBrushDataStreamType {
		AdobeBrushDataTypeFile		= 1,
		AdobeBrushDataTypeIStream	= 2,
	};

	enum AdobeBrushType {
		AdobeBrushTypeComputed	= 1,
		AdobeBrushTypeSampled	= 2,
	};

	//////////////////////////////////////////////////////////////////////////
	//
	typedef struct tagAdobeBrushRawData { 
		DWORD		dwSize;
		BYTE*		pBits;  
		DWORD		dwSizeOfImage;
	} AdobeBrushRawData;

	typedef struct tagAdobeBrushDibData  { 
		DWORD		dwSize;
		BYTE*		pBits;  
		DWORD		dwSizeOfImage;
		HBITMAP		hBitmap;
		BITMAPINFO*	oHeader; 
	} AdobeBrushDibData;


	enum AdobePropertyType {
		PropertyTypeText	= 0,
		PropertyTypeLong	= 1,
		PropertyTypeBool	= 2,
		PropertyTypeObjc	= 3,
		PropertyTypeDoub	= 4,
		PropertyTypeUntF	= 5,
		PropertyTypeEnum	= 6,

		PropertyTypeIgnr	= 50,
	};

	typedef struct tagAdobeBrushProperty {
		WCHAR*	wValue;
		DWORD	dwValue;
		bool	bValue;
		double	fValue;
		struct {
			DWORD	dwType;
			double  fValue;
		} ufValue;
	} AdobeBrushProperty;

	//////////////////////////////////////////////////////////////////////////
	// adobe brush properties virtual class
	class IUnknown {
	public:
		// Counter
		virtual unsigned long AddRef() = 0;
		virtual void Release() = 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// adobe brush properties virtual class
	class IAdobeProperty : public IUnknown {
	public:
		// Returns property name
		virtual WCHAR* Name() = 0;
		// Returns property type
		virtual AdobePropertyType	Type() = 0;
		// Returns property value
		virtual AdobeBrushProperty& Value() = 0;

		// Returns count of the child properties
		virtual DWORD Count() = 0;			
		// Determines whether the container has child items
		virtual bool HasChild() = 0;
		// Retrieve child items
		virtual IAdobeProperty* GetProperty (DWORD dwIndex) = 0;
	};

	class IAdobeBrush : public IUnknown {
	public:
		// Dimensions
		virtual DWORD Top() = 0;			
		virtual DWORD Left() = 0;
		virtual DWORD Bottom() = 0;
		virtual DWORD Right() = 0;

		// Returns brush width
		virtual DWORD Width() = 0;
		// Returns brush height
		virtual DWORD Height() = 0;

		// Returns brush name
		virtual WCHAR* Name() = 0;

		// Returns brush type
		virtual AdobeBrushType Type() = 0;

		// Returns pointer to raw data
		virtual void* Data() = 0;

		// Returns count of the child properties
		virtual DWORD Count() = 0;			

		// Determines whether the container has child items
		virtual bool HasProperties() = 0;

		// Retrieve child items
		virtual IAdobeProperty* GetProperty (DWORD dwIndex) = 0;

		virtual IAdobeProperty* GetProperty (WCHAR* wcsPath, DWORD dwPath, AdobePropertyType enPropType = PropertyTypeIgnr ) = 0;
	};


	class IAdobeBrushes : public IUnknown {
	public:
		// Returns count of the brushes
		virtual DWORD Count() = 0;			

		// Retrieve child items
		virtual IAdobeBrush* Get (DWORD dwIndex) = 0;
	
		// Remove specified brush from collection
		virtual BOOL Remove (DWORD dwIndex) = 0;
		virtual BOOL Remove (IAdobeBrush* pBrush) = 0;

		// Insert new brush to collection
		virtual DWORD Add ( IAdobeBrush* pBrush ) = 0;
	};
	

	typedef struct tagABRPARSER_JOB {
		DWORD			dwBrushesTotal;
		DWORD			dwCurrentBrush;
		BOOL			bJobFinished;
		IAdobeBrushes*	pBrushes;
		void*			pParam;
	} ABRPARSER_JOB, *PABRPARSER_JOB;

	typedef BOOL (*PABRCALLBACK) ( ABRPARSER_JOB* pJobStatus );

	EXT_EXPORTS DWORD SaveAdobePhotoshopBrushFile ( void* pStream, IAdobeBrushes* pBrushes, AdobeBrushDataStreamType nStreamType = AdobeBrushDataTypeFile, PABRCALLBACK pCallback = 0, void* pParam = 0 );
	EXT_EXPORTS IAdobeBrushes* ReadAdobePhotoshopBrushFile ( void* pStream, AdobeBrushDataStreamType nStreamType = AdobeBrushDataTypeFile, AdobeBrushDataType nDataTypeRequested = AdobeBrushDataTypeDIB, PABRCALLBACK pCallback = 0, void* pParam = 0 );
	EXT_EXPORTS IAdobeProperty* SearchForProperty ( IAdobeProperty* pRoot, WCHAR* wcsName, DWORD dwName, AdobePropertyType PropType = PropertyTypeIgnr );
}

#endif