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
#ifndef _REVELER_H_INCLUDED_
#define _REVELER_H_INCLUDED_

#pragma once

#include "AdobeBrush.h"

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	class CPascalString {
	public:
		CPascalString (WCHAR* pString) : m_pString(pString) {};
		CPascalString (CPascalString& rOperand) : m_pString(rOperand.m_pString) {};
		operator WCHAR*() { return m_pString; };
		WCHAR* m_pString;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	class CUCS2String {
	public:
		CUCS2String (WCHAR* pString) : m_pString(pString) {};
		CUCS2String (CUCS2String& rOperand) : m_pString(rOperand.m_pString) {};
		operator WCHAR*() { return m_pString; };
		WCHAR* m_pString;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	class CParser {
	public:
		CParser();
		CParser( BYTE* pBuffer, DWORD dwBuffer );
		CParser ( CParser& rOperand );
		~CParser();

	public:
		void Set ( BYTE* pBuffer, DWORD dwBuffer );
		bool UpdateImage ( BYTE* pBuffer, DWORD dwBuffer );
		bool IsValid () const; 

		DWORD GetSize() const;

		BYTE* GetBuffer(DWORD dwBuffer, bool bStepForward = true );

		bool MoveBackward ( DWORD nSize );
		bool MoveForward ( DWORD nSize );

		operator bool () const;

		CParser& operator += (DWORD dwSize );
		CParser& operator -= (DWORD dwSize );

		//
		template<class T> T Get ( bool bStepForward = true );

		//
		static DWORD ToBigEndian (DWORD n);
		static WORD ToBigEndian (WORD n);

		bool GetImage ( void* pBuffer, AdobeBrushDataType DataType, DWORD dwWidth, DWORD dwHeight, BYTE cCompression );

	private:
		BYTE* m_pBuffer, *m_pOriginal;
		DWORD m_dwBuffer, m_dwOriginal;
	};


	//////////////////////////////////////////////////////////////////////////
	//
	class CProperiesParser : public CParser {
	public:
		CProperiesParser ( CParser& rOperand );
		CProperiesParser ( CProperiesParser& rOperand );

		// Assign section walker
		void Parse ();

		// Properties storage
		CStorage<CAdobeProperty> m_oProperties;

	private:
		// Parse section
		void ParseSection ( CStorage<CAdobeProperty>& roRoot, DWORD dwItemsCount );
		void GetSection(CAdobeProperty* pProperty, bool bHasHeader );

		// Retrieve name
		void GetName ( CAdobeProperty* pProperty );
		// Retrieve value
		void GetValue ( CAdobeProperty* pProperty );
	};
};

#endif