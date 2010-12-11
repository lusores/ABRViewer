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
#include "DragDrop.h"

namespace BigBrotherAndy {

	CLIPFORMAT CF_FILECONTENTS = (CLIPFORMAT)RegisterClipboardFormat(TEXT("FileContents") );
	CLIPFORMAT CF_FILEDESCRIPTOR = (CLIPFORMAT)RegisterClipboardFormat(TEXT("FileGroupDescriptor") );
	CLIPFORMAT CF_PREFERREDDROPEFFECT = (CLIPFORMAT)RegisterClipboardFormat(TEXT("Preferred DropEffect"));

	//	Constructor for the CDropTarget class
	CDropTarget::CDropTarget(HWND hwnd) {
		m_lRefCount  = 1;
		m_hWnd       = hwnd;
		m_fAllowDrop = false;

		if(FAILED(CoCreateInstance(CLSID_DragDropHelper,NULL,CLSCTX_INPROC_SERVER, IID_IDropTargetHelper,(LPVOID*)&m_pDropTargetHelper))) {
			m_pDropTargetHelper = 0;
		}
	}

	//	Destructor for the CDropTarget class
	CDropTarget::~CDropTarget() {
		if ( m_pDropTargetHelper ) {
			m_pDropTargetHelper->Release();
			m_pDropTargetHelper = NULL;
		}
	}

	//	IUnknown::QueryInterface
	HRESULT __stdcall CDropTarget::QueryInterface (REFIID iid, void ** ppvObject) {
		if(iid == IID_IDropTarget || iid == IID_IUnknown) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	//	IUnknown::AddRef
	ULONG __stdcall CDropTarget::AddRef() {
		return InterlockedIncrement(&m_lRefCount);
	}	

	//	IUnknown::Release
	ULONG __stdcall CDropTarget::Release() {
		LONG count = InterlockedDecrement(&m_lRefCount);

		if( count ) {
			return count;
		};

		delete this;
		return 0;
	}

	bool CDropTarget::QueryDrop ( DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect ) {  
		if(!m_fAllowDrop) {
			*pdwEffect = DROPEFFECT_NONE;
			return false;
		};

		DWORD dwOKEffects = *pdwEffect; 

		//CTRL+SHIFT  -- DROPEFFECT_LINK
		//CTRL        -- DROPEFFECT_COPY
		//SHIFT       -- DROPEFFECT_MOVE
		//no modifier -- DROPEFFECT_MOVE or whatever is allowed by src
		*pdwEffect = (grfKeyState & MK_CONTROL) ? ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_LINK : DROPEFFECT_COPY ): ( (grfKeyState & MK_SHIFT) ? DROPEFFECT_MOVE : 0 );
		if ( *pdwEffect == 0 )  {
			// No modifier keys used by user while dragging. 
			if (DROPEFFECT_COPY & dwOKEffects) {
				*pdwEffect = DROPEFFECT_COPY;
			} else if (DROPEFFECT_MOVE & dwOKEffects) {
				*pdwEffect = DROPEFFECT_MOVE; 
			} else if (DROPEFFECT_LINK & dwOKEffects) {
				*pdwEffect = DROPEFFECT_LINK; 
			} else {
				*pdwEffect = DROPEFFECT_NONE;
			};
		}  else {
			// Check if the drag source application allows the drop effect desired by user.
			// The drag source specifies this in DoDragDrop
			if(!(*pdwEffect & dwOKEffects)) {
				*pdwEffect = DROPEFFECT_NONE;
			};
		}  

		return (DROPEFFECT_NONE == *pdwEffect) ? false:true;
	}  

	//	IDropTarget::DragEnter
	HRESULT __stdcall CDropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
		m_fAllowDrop = false;

		if (!pDataObject) {
			return E_INVALIDARG; 
		};

		if ( m_pDropTargetHelper ) {
			m_pDropTargetHelper->DragEnter(m_hWnd, pDataObject, (LPPOINT)&pt, *pdwEffect);
		};

		// does the data object contain data we want?
		for  ( int i = 0 ; i < m_oFormatEtc.Count(); i++ ) {
			if ( pDataObject->QueryGetData ( m_oFormatEtc.Value(i) ) == S_OK ) {
				m_fAllowDrop = true;
				break;
			};
		};

		if ( m_fAllowDrop ) {
			if ( QueryDrop ( grfKeyState, pt, pdwEffect ) ) {
				SetFocus(m_hWnd);
			};
		} else {
			*pdwEffect = DROPEFFECT_NONE;
		}
		
		return S_OK;
	}

	//	IDropTarget::DragOver
	HRESULT __stdcall CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
		if(m_pDropTargetHelper)
			m_pDropTargetHelper->DragOver((LPPOINT)&pt, *pdwEffect);

		QueryDrop ( grfKeyState, pt, pdwEffect );
		return S_OK;
	}

	//	IDropTarget::DragLeave
	HRESULT __stdcall CDropTarget::DragLeave(void) {
		if(m_pDropTargetHelper)
			m_pDropTargetHelper->DragLeave();

		return S_OK;
	}

	//	IDropTarget::Drop
	HRESULT __stdcall CDropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect) {
		if (!pDataObject)
			return E_INVALIDARG;
		
		if(m_pDropTargetHelper)
			m_pDropTargetHelper->Drop(pDataObject, (LPPOINT)&pt, *pdwEffect);

		if ( QueryDrop ( grfKeyState, pt, pdwEffect ) ) {
			STGMEDIUM stgmed;
			BOOL bContinueDropOp = false;

			int i = 0;

			// does the data object contain data we want?
			for  ( int i = 0 ; i < m_oFormatEtc.Count(); i++ ) {
				if ( pDataObject->QueryGetData ( m_oFormatEtc.Value(i) ) == S_OK ) {
					bContinueDropOp = ( pDataObject->GetData( m_oFormatEtc.Value(i), &stgmed ) == S_OK );
					break;
				};
			};

			if ( bContinueDropOp ) {
				SendMessage ( m_hWnd, WM_ONDROP, (WPARAM)&stgmed, (LPARAM)m_oFormatEtc.Value(i) );
				// release the data using the COM API
				ReleaseStgMedium(&stgmed);
			};
		};

		m_fAllowDrop = false;
		*pdwEffect = DROPEFFECT_NONE;
		return S_OK;
	}

	BOOL CDropTarget::RegisterDropWindow(HWND hwnd, CDropTarget **ppDropTarget) {
		if ( *ppDropTarget = new CDropTarget(hwnd) ) {
			// acquire a strong lock
			if ( CoLockObjectExternal ( *ppDropTarget, TRUE, TRUE ) == S_OK ) {
				// tell OLE that the window is a drop target
				if ( RegisterDragDrop ( hwnd, (IDropTarget*)*ppDropTarget ) == S_OK ) {
					return TRUE;
				}
				// remove the strong lock
				CoLockObjectExternal (*ppDropTarget, FALSE, TRUE);
			};
			delete *ppDropTarget;
			*ppDropTarget = 0;
		}
		return FALSE;
	}

	void CDropTarget::UnregisterDropWindow(HWND hwnd, CDropTarget *pDropTarget) {
		if ( pDropTarget ) {
			// remove drag+drop
			RevokeDragDrop(hwnd);

			// remove the strong lock
			CoLockObjectExternal(pDropTarget, FALSE, TRUE);

			// release our own reference
			pDropTarget->Release();
		}
	}

	//	Constructor
	CDataObject::CDataObject(FORMATETC *fmtetc, STGMEDIUM *stgmed, int count) {
		m_lRefCount = 1;
		m_nNumFormats = count;

		m_pFormatEtc = new FORMATETC[count];
		m_pStgMedium = new STGMEDIUM[count];

		for(int i = 0; i < count; i++) {
			m_pFormatEtc[i] = fmtetc[i];
			m_pStgMedium[i] = stgmed[i];
		}
	}

	//	Destructor
	CDataObject::~CDataObject() {
		// cleanup
		if(m_pFormatEtc) delete[] m_pFormatEtc;
		if(m_pStgMedium) delete[] m_pStgMedium;
	}

	//	IUnknown::AddRef
	ULONG __stdcall CDataObject::AddRef(void) {
		// increment object reference count
		return InterlockedIncrement(&m_lRefCount);
	}

	//	IUnknown::Release
	ULONG __stdcall CDataObject::Release(void) {
		// decrement object reference count
		LONG count = InterlockedDecrement(&m_lRefCount);

		if ( count ) {
			return count;
		}

		delete this;
		return 0;
	}

	//	IUnknown::QueryInterface
	HRESULT __stdcall CDataObject::QueryInterface(REFIID iid, void **ppvObject) {
		// check to see what interface has been requested
		if(iid == IID_IDataObject || iid == IID_IUnknown) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	HGLOBAL DupMem(HGLOBAL hMem) {
		// lock the source memory object
		DWORD len = (DWORD)GlobalSize(hMem);
		PVOID source = GlobalLock(hMem);

		// create a fixed "global" block - i.e. just
		// a regular lump of our process heap
		PVOID dest = GlobalAlloc(GMEM_FIXED, len);

		memcpy(dest, source, len);

		GlobalUnlock(hMem);

		return dest;
	}

	int CDataObject::LookupFormatEtc(FORMATETC *pFormatEtc) {
		if ( !pFormatEtc ) {
			return E_INVALIDARG;
		};

		for(int i = 0; i < m_nNumFormats; i++) {
			if ( ( pFormatEtc->tymed & m_pFormatEtc[i].tymed) && pFormatEtc->cfFormat == m_pFormatEtc[i].cfFormat && pFormatEtc->dwAspect == m_pFormatEtc[i].dwAspect) {
				return i;
			};
		};
		return -1;
	}

	//	IDataObject::GetData
	HRESULT __stdcall CDataObject::GetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium) {
		int idx;

		// try to match the requested FORMATETC with one of our supported formats
		if((idx = LookupFormatEtc(pFormatEtc)) == -1) {
			return DV_E_FORMATETC;
		}

		// found a match! transfer the data into the supplied storage-medium
		pMedium->tymed			 = m_pFormatEtc[idx].tymed;
		pMedium->pUnkForRelease = 0;

		switch(m_pFormatEtc[idx].tymed) {
		case TYMED_HGLOBAL:
			pMedium->hGlobal = DupMem(m_pStgMedium[idx].hGlobal);
			break;
		default:
			return DV_E_FORMATETC;
		}

		return S_OK;
	}

	//	IDataObject::GetDataHere
	HRESULT __stdcall CDataObject::GetDataHere (FORMATETC *pFormatEtc, STGMEDIUM *pMedium) {
		// GetDataHere is only required for IStream and IStorage mediums
		// It is an error to call GetDataHere for things like HGLOBAL and other clipboard formats
		//
		//	OleFlushClipboard 
		//
		return DATA_E_FORMATETC;
	}

	//	IDataObject::QueryGetData
	//
	//	Called to see if the IDataObject supports the specified format of data
	HRESULT __stdcall CDataObject::QueryGetData (FORMATETC *pFormatEtc) {
		return ( LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
	}

	//	IDataObject::GetCanonicalFormatEtc
	HRESULT __stdcall CDataObject::GetCanonicalFormatEtc (FORMATETC *pFormatEct, FORMATETC *pFormatEtcOut) {
		// Apparently we have to set this field to NULL even though we don't do anything else
		pFormatEtcOut->ptd = NULL;
		return E_NOTIMPL;
	}

	//	IDataObject::SetData
	HRESULT __stdcall CDataObject::SetData (FORMATETC *pFormatEtc, STGMEDIUM *pMedium, BOOL fRelease) {
		return E_NOTIMPL;
	}

	//	IDataObject::EnumFormatEtc
	HRESULT __stdcall CDataObject::EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC **ppEnumFormatEtc) {
		if(dwDirection == DATADIR_GET) {
			// for Win2k+ you can use the SHCreateStdEnumFmtEtc API call, however
			// to support all Windows platforms we need to implement IEnumFormatEtc ourselves.
			return CreateEnumFormatEtc(m_nNumFormats, m_pFormatEtc, ppEnumFormatEtc);
		} else {
			// the direction specified is not support for drag+drop
			return E_NOTIMPL;
		}
	}

	//	IDataObject::DAdvise
	HRESULT __stdcall CDataObject::DAdvise (FORMATETC *pFormatEtc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) {
		return OLE_E_ADVISENOTSUPPORTED;
	}

	//	IDataObject::DUnadvise
	HRESULT __stdcall CDataObject::DUnadvise (DWORD dwConnection) {
		return OLE_E_ADVISENOTSUPPORTED;
	}

	//	IDataObject::EnumDAdvise
	HRESULT __stdcall CDataObject::EnumDAdvise (IEnumSTATDATA **ppEnumAdvise) {
		return OLE_E_ADVISENOTSUPPORTED;
	}

	//	Helper function
	HRESULT CreateDataObject (FORMATETC *fmtetc, STGMEDIUM *stgmeds, UINT count, IDataObject **ppDataObject) {
		if(ppDataObject == 0) {
			return E_INVALIDARG;
		};

		*ppDataObject = new CDataObject(fmtetc, stgmeds, count);

		return (*ppDataObject) ? S_OK : E_OUTOFMEMORY;
	}


	//////////////////////////////////////////////////////////////////////////
	//

	//	Constructor
	CDropSource::CDropSource()  {
		m_lRefCount = 1;
	}

	//	Destructor
	CDropSource::~CDropSource() {
	}

	//	IUnknown::AddRef
	ULONG __stdcall CDropSource::AddRef() {
		// increment object reference count
		return InterlockedIncrement(&m_lRefCount);
	}

	//	IUnknown::Release
	ULONG __stdcall CDropSource::Release(void) {
		// decrement object reference count
		LONG count = InterlockedDecrement(&m_lRefCount);

		if(count) {
			return count;
		};

		delete this;
		return 0;
	}

	//	IUnknown::QueryInterface
	HRESULT __stdcall CDropSource::QueryInterface(REFIID iid, void **ppvObject) {
		// check to see what interface has been requested
		if(iid == IID_IDropSource || iid == IID_IUnknown) {
			AddRef();
			*ppvObject = this;
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	//	Called by OLE whenever Escape/Control/Shift/Mouse buttons have changed
	HRESULT __stdcall CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) {
		// if the <Escape> key has been pressed since the last call, cancel the drop
		if(fEscapePressed == TRUE)
			return DRAGDROP_S_CANCEL;	

		// if the <LeftMouse> button has been released, then do the drop!
		if((grfKeyState & MK_LBUTTON) == 0)
			return DRAGDROP_S_DROP;

		// continue with the drag-drop
		return S_OK;
	}

	//	Return either S_OK, or DRAGDROP_S_USEDEFAULTCURSORS to instruct OLE to use the
	//  default mouse cursor images
	HRESULT __stdcall CDropSource::GiveFeedback(DWORD dwEffect) {
		return DRAGDROP_S_USEDEFAULTCURSORS;
	}

	//	Helper routine to create an IDropSource object
	HRESULT CreateDropSource(IDropSource **ppDropSource) {
		if(ppDropSource == 0)
			return E_INVALIDARG;

		*ppDropSource = new CDropSource();

		return (*ppDropSource) ? S_OK : E_OUTOFMEMORY;
	}
}