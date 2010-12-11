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
#include "EnumFormat.h"

namespace BigBrotherAndy {

	//	"Drop-in" replacement for SHCreateStdEnumFmtEtc. Called by CDataObject::EnumFormatEtc
	HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc) {
		if(nNumFormats == 0 || pFormatEtc == 0 || ppEnumFormatEtc == 0)
			return E_INVALIDARG;

		*ppEnumFormatEtc = new CEnumFormatEtc(pFormatEtc, nNumFormats);

		return (*ppEnumFormatEtc) ? S_OK : E_OUTOFMEMORY;
	}

	//	Helper function to perform a "deep" copy of a FORMATETC
	static void DeepCopyFormatEtc(FORMATETC *dest, FORMATETC *source) {
		// copy the source FORMATETC into dest
		*dest = *source;

		if(source->ptd) {
			// allocate memory for the DVTARGETDEVICE if necessary
			dest->ptd = (DVTARGETDEVICE*)CoTaskMemAlloc(sizeof(DVTARGETDEVICE));
			// copy the contents of the source DVTARGETDEVICE into dest->ptd
			*(dest->ptd) = *(source->ptd);
		}
	}

	//	Constructor 
	CEnumFormatEtc::CEnumFormatEtc(FORMATETC *pFormatEtc, int nNumFormats) {
		m_lRefCount   = 1;
		m_nIndex      = 0;
		m_nNumFormats = nNumFormats;
		m_pFormatEtc  = new FORMATETC[nNumFormats];

		// copy the FORMATETC structures
		for(int i = 0; i < nNumFormats; i++) {	
			DeepCopyFormatEtc(&m_pFormatEtc[i], &pFormatEtc[i]);
		}
	}

	//	Destructor
	CEnumFormatEtc::~CEnumFormatEtc() {
		if(m_pFormatEtc) {
			for(ULONG i = 0; i < m_nNumFormats; i++) {
				if(m_pFormatEtc[i].ptd) {
					CoTaskMemFree(m_pFormatEtc[i].ptd);
				};
			};
			delete[] m_pFormatEtc;
		};
	}

	//	IUnknown::AddRef
	ULONG __stdcall CEnumFormatEtc::AddRef(void) {
		// increment object reference count
		return InterlockedIncrement(&m_lRefCount);
	}

	//	IUnknown::Release
	ULONG __stdcall CEnumFormatEtc::Release(void) {
		// decrement object reference count
		LONG count = InterlockedDecrement(&m_lRefCount);

		if ( count ) {
			return count;
		};

		delete this;
		return 0;
	}

	//	IUnknown::QueryInterface
	HRESULT __stdcall CEnumFormatEtc::QueryInterface(REFIID iid, void **ppvObject) {
		// check to see what interface has been requested
		if(iid == IID_IEnumFORMATETC || iid == IID_IUnknown) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	//	IEnumFORMATETC::Next
	//
	//	If the returned FORMATETC structure contains a non-null "ptd" member, then
	//  the caller must free this using CoTaskMemFree (stated in the COM documentation)
	HRESULT __stdcall CEnumFormatEtc::Next(ULONG celt, FORMATETC *pFormatEtc, ULONG * pceltFetched) {
		ULONG copied  = 0;

		// validate arguments
		if(celt == 0 || pFormatEtc == 0) {
			return E_INVALIDARG;
		};

		// copy FORMATETC structures into caller's buffer
		while(m_nIndex < m_nNumFormats && copied < celt) {
			DeepCopyFormatEtc(&pFormatEtc[copied], &m_pFormatEtc[m_nIndex]);
			copied++;
			m_nIndex++;
		}

		// store result
		if(pceltFetched != 0) {
			*pceltFetched = copied;
		};

		// did we copy all that was requested?
		return (copied == celt) ? S_OK : S_FALSE;
	}

	//	IEnumFORMATETC::Skip
	HRESULT __stdcall CEnumFormatEtc::Skip(ULONG celt) {
		m_nIndex += celt;
		return (m_nIndex <= m_nNumFormats) ? S_OK : S_FALSE;
	}

	//	IEnumFORMATETC::Reset
	HRESULT __stdcall CEnumFormatEtc::Reset(void) {
		m_nIndex = 0;
		return S_OK;
	}

	//	IEnumFORMATETC::Clone
	HRESULT __stdcall CEnumFormatEtc::Clone(IEnumFORMATETC ** ppEnumFormatEtc) {
		HRESULT hResult;

		// make a duplicate enumerator
		if ( hResult = CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc) == S_OK ) {
			// manually set the index state
			((CEnumFormatEtc *) *ppEnumFormatEtc)->m_nIndex = m_nIndex;
		};

		return hResult;
	}
}
