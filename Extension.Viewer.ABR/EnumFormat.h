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
#ifndef _ENUMFORMAT_H_INCLUDED_
#define _ENUMFORMAT_H_INCLUDED_

#pragma once

namespace BigBrotherAndy {

	//////////////////////////////////////////////////////////////////////////
	//
	class CEnumFormatEtc : public IEnumFORMATETC {
	public:
		// Ctor
		CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats);
		~CEnumFormatEtc();

		// IUnknown members
		HRESULT __stdcall  QueryInterface (REFIID iid, void ** ppvObject);
		ULONG	__stdcall  AddRef (void);
		ULONG	__stdcall  Release (void);

		// IEnumFormatEtc
		HRESULT __stdcall  Next  (ULONG celt, FORMATETC * rgelt, ULONG * pceltFetched);
		HRESULT __stdcall  Skip  (ULONG celt); 
		HRESULT __stdcall  Reset (void);
		HRESULT __stdcall  Clone (IEnumFORMATETC ** ppEnumFormatEtc);

	private:
		volatile LONG	m_lRefCount;		// Reference count for this COM interface
		ULONG			m_nIndex;			// Current enumerator index
		ULONG			m_nNumFormats;		// Number of FORMATETC members
		FORMATETC *		m_pFormatEtc;		// Array of FORMATETC objects
	};
}

#endif