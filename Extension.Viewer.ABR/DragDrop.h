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
#ifndef _DROPTARGET_H_INCLUDED_
#define _DROPTARGET_H_INCLUDED_

#pragma once

#include "Helpers.h"

namespace BigBrotherAndy {

	HRESULT CreateDropSource(IDropSource **ppDropSource);
	HRESULT CreateEnumFormatEtc(UINT nNumFormats, FORMATETC *pFormatEtc, IEnumFORMATETC **ppEnumFormatEtc);

	class CDataObject : public IDataObject {
	public:
		// Constructor / Destructor
		CDataObject(FORMATETC *fmt, STGMEDIUM *stgmed, int count);
		~CDataObject();

		// IUnknown members
		HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
		ULONG __stdcall AddRef ();
		ULONG __stdcall Release ();

		// IDataObject members
		HRESULT __stdcall GetData ( FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
		HRESULT __stdcall GetDataHere ( FORMATETC *pFormatEtc, STGMEDIUM *pMedium);
		HRESULT __stdcall QueryGetData ( FORMATETC *pFormatEtc);
		HRESULT __stdcall GetCanonicalFormatEtc ( FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut);
		HRESULT __stdcall SetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease);
		HRESULT __stdcall EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc);
		HRESULT __stdcall DAdvise ( FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
		HRESULT __stdcall DUnadvise ( DWORD dwConnection);
		HRESULT __stdcall EnumDAdvise ( IEnumSTATDATA **ppEnumAdvise);

	private:
		int LookupFormatEtc(FORMATETC *pFormatEtc);

		// any private members and functions
		volatile LONG		m_lRefCount;
		FORMATETC*			m_pFormatEtc;
		STGMEDIUM*			m_pStgMedium;
		LONG				m_nNumFormats;
	};

	class CDropTarget : public IDropTarget {
	public:
		// Constructor
		CDropTarget(HWND hwnd);
		~CDropTarget();

		// IUnknown implementation
		HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
		ULONG	__stdcall AddRef (void);
		ULONG	__stdcall Release (void);

		// IDropTarget implementation
		HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
		HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
		HRESULT __stdcall DragLeave (void);
		HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

		static BOOL RegisterDropWindow(HWND hwnd, CDropTarget **ppDropTarget);
		static void UnregisterDropWindow(HWND hwnd, CDropTarget *pDropTarget);

	private:

		// internal helper function
		bool	QueryDrop(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

		// Private member variables
		LONG	m_lRefCount;
		HWND	m_hWnd;
		bool    m_fAllowDrop;

		struct IDropTargetHelper *m_pDropTargetHelper;
		IDataObject *m_pDataObject;

		CStorage<FORMATETC> m_oFormatEtc;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	class CDropSource : public IDropSource {
	public:
		CDropSource();
		~CDropSource();

		// IUnknown members
		HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
		ULONG __stdcall AddRef ();
		ULONG __stdcall Release ();

		//
		// IDropSource members
		HRESULT __stdcall QueryContinueDrag	(BOOL fEscapePressed, DWORD grfKeyState);
		HRESULT __stdcall GiveFeedback (DWORD dwEffect);

	private:
		volatile LONG m_lRefCount;
	};
};

#endif