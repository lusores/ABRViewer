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
#include "Helpers.h"

namespace BigBrotherAndy {

	CBufferDC::CBufferDC (HWND hWnd, HDC hDC, RECT* prcUpdate, BOOL bAutoFlush /*= TRUE*/ ) : m_hWnd (hWnd), m_hDC(hDC), m_bAutoFlush(bAutoFlush) {

		if ( m_hDC ) {
			m_bReleaseDC = 0;
		} else {
			m_hDC = GetDC(m_hWnd);
			m_bReleaseDC = 1;
		};


		GetClientRect ( m_hWnd, &m_rcSource);

		if ( prcUpdate ) {
			CopyMemory ( &m_rcUpdate, prcUpdate, sizeof(RECT) );
		} else {
			CopyMemory ( &m_rcUpdate, &m_rcSource, sizeof(RECT) );
		};

		m_hBufferDC = CreateCompatibleDC ( m_hDC );
		m_hBitmap = CreateCompatibleBitmap ( m_hDC, m_rcSource.right - m_rcSource.left, m_rcSource.bottom - m_rcSource.top );
		m_hOldBitmap = (HBITMAP) SelectObject ( m_hBufferDC, m_hBitmap );

		// Copy the mapping settings

		// Retrieve mapping mode of the original device context and set to the buffer device context
		int nMapMode = GetMapMode(m_hDC);	
		SetMapMode ( m_hBufferDC, nMapMode );

		// Retrieve the x-coordinates and y-coordinates of the window origin 
		// for the original device context and set to the buffer device context
		POINT pt;
		GetWindowOrgEx(m_hDC, &pt);
		SetWindowOrgEx(m_hBufferDC, pt.x, pt.y, &m_ptOrigin );

		// Retrieve the x-coordinates and y-coordinates of the view port origin 
		// for the original device context and set to the buffer device context
		GetViewportOrgEx(m_hDC, &pt);
		SetViewportOrgEx(m_hBufferDC, pt.x, pt.y, &m_ptViewport );

		// These are only relevant to MM_ISOTROPIC and MM_ANISOTROPIC
		if ( nMapMode > MM_MAX_FIXEDSCALE) {
			// Retrieve the x-extent and y-extent of the window for the original device context
			// and set to the buffer device context
			SIZE size;
			GetWindowExtEx ( m_hDC, &size );
			SetWindowExtEx ( m_hBufferDC, size.cx, size.cy, &m_szExtent );

			// Retrieve the x-extent and y-extent of the current view port for the original device context
			// and set to the buffer device context
			GetViewportExtEx ( m_hDC, &size );
			SetViewportExtEx ( m_hBufferDC, size.cx, size.cy, &m_szViewport );
		};

		BitBlt ( m_hBufferDC, m_rcUpdate.left, m_rcUpdate.top, m_rcUpdate.right - m_rcUpdate.left, m_rcUpdate.bottom - m_rcUpdate.top, m_hDC, m_rcUpdate.left, m_rcUpdate.top, SRCCOPY );
	}

	void CBufferDC::FlushRect ( RECT* rc ) {
		if ( rc ) {
			if ( RectVisible(m_hDC, rc) ) {
				BitBlt ( m_hDC, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, m_hBufferDC, rc->left, rc->top, SRCCOPY );
			};
		} else {
			BitBlt ( m_hDC, m_rcUpdate.left, m_rcUpdate.top, m_rcUpdate.right - m_rcUpdate.left, m_rcUpdate.bottom - m_rcUpdate.top, m_hBufferDC, m_rcUpdate.left, m_rcUpdate.top, SRCCOPY );
		};
	}


	CBufferDC::~CBufferDC () {
		if ( m_bAutoFlush ) {
			BitBlt ( m_hDC, m_rcUpdate.left, m_rcUpdate.top, m_rcUpdate.right - m_rcUpdate.left, m_rcUpdate.bottom - m_rcUpdate.top, m_hBufferDC, m_rcUpdate.left, m_rcUpdate.top, SRCCOPY );
		}

		SelectObject ( m_hBufferDC, m_hOldBitmap );

		DeleteObject ( m_hBitmap );
		DeleteDC ( m_hBufferDC );

		if ( m_bReleaseDC ) {
			ReleaseDC ( m_hWnd, m_hDC );
		};
	}



	//////////////////////////////////////////////////////////////////////////
	// Based on Stefan Kueng's original code
	// I don't remember where it was originally used ;(
	// anyway: copyright (C) 2009 - Stefan Kueng
	//
	typedef HRESULT (__stdcall *DWM_EXTEND_FRAME_INTO_CLIENT_AREA)(HWND ,const MARGINS* );
	typedef HRESULT (__stdcall *DWM_IS_COMPOSITION_ENABLED)(BOOL *pfEnabled);
	typedef HRESULT (__stdcall *DWM_ENABLE_COMPOSITION)(UINT uCompositionAction);

	CDwmApiImpl::CDwmApiImpl(void):m_hDwmApiLib(NULL) {
	}

	BOOL CDwmApiImpl::Initialize(void) {
		if(m_hDwmApiLib) {
			SetLastError(ERROR_ALREADY_INITIALIZED);
			return FALSE;
		}

		m_hDwmApiLib = LoadLibraryW(L"dwmapi.dll");
		return IsInitialized();
	}

	BOOL CDwmApiImpl::IsInitialized(void) {
		return (NULL!=m_hDwmApiLib);
	}

	CDwmApiImpl::~CDwmApiImpl(void) {
		if(IsInitialized()) {
			FreeLibrary(m_hDwmApiLib);
			m_hDwmApiLib = NULL;
		}
	}

	HRESULT CDwmApiImpl::DwmExtendFrameIntoClientArea(HWND hWnd,const MARGINS* pMarInset) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		DWM_EXTEND_FRAME_INTO_CLIENT_AREA pfnDwmExtendFrameIntoClientArea = (DWM_EXTEND_FRAME_INTO_CLIENT_AREA)GetProcAddress(m_hDwmApiLib, "DwmExtendFrameIntoClientArea");
		if(!pfnDwmExtendFrameIntoClientArea)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDwmExtendFrameIntoClientArea(hWnd, pMarInset);
	}

	BOOL CDwmApiImpl::IsDwmCompositionEnabled(void) {
		if(!IsInitialized()) {
			SetLastError((DWORD)OLE_E_BLANK);
			return FALSE;
		}
		DWM_IS_COMPOSITION_ENABLED pfnDwmIsCompositionEnabled = (DWM_IS_COMPOSITION_ENABLED)GetProcAddress(m_hDwmApiLib, "DwmIsCompositionEnabled");
		if(!pfnDwmIsCompositionEnabled)
			return FALSE;
		BOOL bEnabled = FALSE;
		HRESULT hRes = pfnDwmIsCompositionEnabled(&bEnabled);
		return SUCCEEDED(hRes) && bEnabled;
	}

	HRESULT CDwmApiImpl::DwmEnableComposition(UINT uCompositionAction) {
		if(!IsInitialized()) {
			SetLastError((DWORD)OLE_E_BLANK);
			return FALSE;
		}
		DWM_ENABLE_COMPOSITION pfnDwmEnableComposition = (DWM_ENABLE_COMPOSITION)GetProcAddress(m_hDwmApiLib, "DwmEnableComposition");
		if(!pfnDwmEnableComposition)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDwmEnableComposition(uCompositionAction);
	}

	CUxThemeAeroImpl::CUxThemeAeroImpl(void) : m_hUxThemeLib(NULL) {
	}

	BOOL CUxThemeAeroImpl::Initialize(void) {
		if(m_hUxThemeLib) {
			SetLastError(ERROR_ALREADY_INITIALIZED);
			return FALSE;
		}

		m_hUxThemeLib = LoadLibraryW(L"uxtheme.dll");
		return IsInitialized();
	}

	typedef HRESULT (__stdcall *BUFFERED_PAINT_INIT)(VOID);
	typedef HTHEME (__stdcall *OPEN_THEME_DATA)(HWND hwnd, LPCWSTR pszClassList);
	typedef HRESULT (__stdcall *CLOSE_THEME_DATA)(HTHEME hTheme);
	typedef HPAINTBUFFER (__stdcall *BEGIN_BUFFERED_PAINT)(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams,  HDC *phdc);
	typedef HRESULT (__stdcall *END_BUFFERED_PAINT)(HPAINTBUFFER hBufferedPaint, BOOL fUpdateTarget);
	typedef HRESULT (__stdcall *DRAW_THEME_TEXT_EX)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect,  const DTTOPTS *pOptions);
	typedef HRESULT (__stdcall *GET_THEME_INT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal);
	typedef HRESULT (__stdcall *GET_THEME_SYS_FONT)(HTHEME hTheme, int iFontId, LOGFONTW *plf);
	typedef HRESULT (__stdcall *BUFFERED_PAINT_SET_ALPHA)(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha);
	typedef HRESULT (__stdcall *DRAW_THEME_BACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect);
	typedef HRESULT (__stdcall *GET_THEME_BKG_CONTENT_RECT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect);
	typedef HRESULT (__stdcall *GET_THEME_BKG_CONTENT_EXTENT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect);
	typedef HRESULT (__stdcall *GET_THEME_BITMAP)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap);
	typedef HRESULT (__stdcall *DRAW_THEME_PARENT_BACKGROUND)(HWND hwnd, HDC hdc, const RECT *prc);
	typedef BOOL (__stdcall *IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT)(HTHEME hTheme,int iPartId, int iStateId);
	typedef HRESULT (__stdcall *DRAW_THEME_TEXT)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect);
	typedef HRESULT (__stdcall *GET_THEME_COLOR)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor);
	typedef HRESULT (__stdcall *GET_THEME_PART_SIZE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz);
	typedef HRESULT (__stdcall *GET_THEME_POSITION)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint);
	typedef HRESULT (__stdcall *GET_THEME_MARGINS)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins);
	typedef HRESULT (__stdcall *GET_THEME_METRIC)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal);
	typedef HRESULT (__stdcall *GET_THEME_RECT)(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect);

	BOOL CUxThemeAeroImpl::IsInitialized(void) {
		return (NULL!=m_hUxThemeLib);
	}

	CUxThemeAeroImpl::~CUxThemeAeroImpl(void) {
		if(IsInitialized()) {
			FreeLibrary(m_hUxThemeLib);
			m_hUxThemeLib = NULL;
		}
	}

	HRESULT CUxThemeAeroImpl::BufferedPaintInit(void) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		BUFFERED_PAINT_INIT pfnBufferedPaintInit = (BUFFERED_PAINT_INIT)GetProcAddress(m_hUxThemeLib, "BufferedPaintInit");
		if(!pfnBufferedPaintInit)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnBufferedPaintInit();
	}

	HRESULT CUxThemeAeroImpl::BufferedPaintUnInit(void) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		BUFFERED_PAINT_INIT pfnBufferedPaintUnInit = (BUFFERED_PAINT_INIT)GetProcAddress(m_hUxThemeLib, "BufferedPaintUnInit");
		if(!pfnBufferedPaintUnInit)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnBufferedPaintUnInit();
	}

	HTHEME CUxThemeAeroImpl::OpenThemeData(HWND hwnd, LPCWSTR pszClassList) {
		if(!IsInitialized()) {
			SetLastError((DWORD)OLE_E_BLANK);
			return NULL;
		}
		OPEN_THEME_DATA pfnOpenThemeData = (OPEN_THEME_DATA)GetProcAddress(m_hUxThemeLib, "OpenThemeData");
		if(!pfnOpenThemeData)
			return NULL;

		return pfnOpenThemeData(hwnd, pszClassList);
	}

	HRESULT CUxThemeAeroImpl::CloseThemeData(HTHEME hTheme) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		CLOSE_THEME_DATA pfnCloseThemeData = (CLOSE_THEME_DATA)GetProcAddress(m_hUxThemeLib, "CloseThemeData");
		if(!pfnCloseThemeData)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnCloseThemeData(hTheme);
	}

	HANDLE CUxThemeAeroImpl::BeginBufferedPaint(HDC hdcTarget, const RECT* prcTarget, BP_BUFFERFORMAT dwFormat, BP_PAINTPARAMS *pPaintParams, HDC *phdc) {
		if(!IsInitialized()) {
			SetLastError((DWORD)OLE_E_BLANK);
			return NULL;
		}
		BEGIN_BUFFERED_PAINT pfnBeginBufferedPaint = (BEGIN_BUFFERED_PAINT)GetProcAddress(m_hUxThemeLib, "BeginBufferedPaint");
		if(!pfnBeginBufferedPaint)
			return NULL;

		return pfnBeginBufferedPaint(hdcTarget, prcTarget, dwFormat, pPaintParams, phdc);
	}

	HRESULT CUxThemeAeroImpl::EndBufferedPaint(HANDLE hBufferedPaint, BOOL fUpdateTarget) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		END_BUFFERED_PAINT pfnEndBufferedPaint = (END_BUFFERED_PAINT)GetProcAddress(m_hUxThemeLib, "EndBufferedPaint");
		if(!pfnEndBufferedPaint)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnEndBufferedPaint(hBufferedPaint, fUpdateTarget);
	}

	HRESULT CUxThemeAeroImpl::DrawThemeTextEx(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int cchText, DWORD dwTextFlags, LPRECT pRect, const DTTOPTS *pOptions) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		DRAW_THEME_TEXT_EX pfnDrawThemeTextEx = (DRAW_THEME_TEXT_EX)GetProcAddress(m_hUxThemeLib, "DrawThemeTextEx");
		if(!pfnDrawThemeTextEx)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDrawThemeTextEx(hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, pRect, pOptions);
	}

	HRESULT CUxThemeAeroImpl::GetThemeInt(HTHEME hTheme, int iPartId, int iStateId, int iPropId, int *piVal) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		GET_THEME_INT pfnGetThemeInt = (GET_THEME_INT)GetProcAddress(m_hUxThemeLib, "GetThemeInt");
		if(!pfnGetThemeInt)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeInt(hTheme, iPartId, iStateId, iPropId, piVal);
	}

	HRESULT CUxThemeAeroImpl::GetThemeSysFont(HTHEME hTheme, int iFontId, LOGFONTW *plf) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}
		GET_THEME_SYS_FONT pfnGetThemeSysFont = (GET_THEME_SYS_FONT)GetProcAddress(m_hUxThemeLib, "GetThemeSysFont");
		if(!pfnGetThemeSysFont)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeSysFont(hTheme, iFontId, plf);
	}

	HRESULT CUxThemeAeroImpl::BufferedPaintSetAlpha(HPAINTBUFFER hBufferedPaint, const RECT *prc, BYTE alpha) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		BUFFERED_PAINT_SET_ALPHA pfnBufferedPaintSetAlpha = (BUFFERED_PAINT_SET_ALPHA)GetProcAddress(m_hUxThemeLib, "BufferedPaintSetAlpha");
		if(!pfnBufferedPaintSetAlpha)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnBufferedPaintSetAlpha(hBufferedPaint, prc, alpha);
	}

	HRESULT CUxThemeAeroImpl::BufferedPaintMakeOpaque_(HPAINTBUFFER hBufferedPaint, const RECT *prc) {
		return BufferedPaintSetAlpha(hBufferedPaint, prc, 255);
	}

	HRESULT CUxThemeAeroImpl::DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		DRAW_THEME_BACKGROUND pfnDrawThemeBackground = (DRAW_THEME_BACKGROUND)GetProcAddress(m_hUxThemeLib, "DrawThemeBackground");
		if(!pfnDrawThemeBackground)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDrawThemeBackground(hTheme, hdc, iPartId, iStateId, pRect, pClipRect);
	}

	HRESULT CUxThemeAeroImpl::GetThemeBackgroundContentRect(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pBoundingRect, LPRECT pContentRect) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_BKG_CONTENT_RECT pfnGetThemeBackgroundContentRect = (GET_THEME_BKG_CONTENT_RECT)GetProcAddress(m_hUxThemeLib, "GetThemeBackgroundContentRect");
		if(!pfnGetThemeBackgroundContentRect)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeBackgroundContentRect(hTheme, hdc, iPartId, iStateId, pBoundingRect, pContentRect);
	}

	HRESULT CUxThemeAeroImpl::GetThemeBackgroundExtent(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT pContentRect, LPRECT pExtentRect) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_BKG_CONTENT_EXTENT pfnGetThemeBackgroundExtent = (GET_THEME_BKG_CONTENT_EXTENT)GetProcAddress(m_hUxThemeLib, "GetThemeBackgroundExtent");
		if(!pfnGetThemeBackgroundExtent)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeBackgroundExtent(hTheme, hdc, iPartId, iStateId, pContentRect, pExtentRect);
	}

	HRESULT CUxThemeAeroImpl::GetThemeBitmap(HTHEME hTheme, int iPartId, int iStateId, int iPropId, ULONG dwFlags, HBITMAP *phBitmap) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_BITMAP pfnGetThemeBitmap = (GET_THEME_BITMAP)GetProcAddress(m_hUxThemeLib, "GetThemeBitmap");
		if(!pfnGetThemeBitmap)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeBitmap(hTheme, iPartId, iStateId, iPropId, dwFlags, phBitmap);
	}

	BOOL CUxThemeAeroImpl::DetermineGlowSize(int *piSize, LPCWSTR pszClassIdList /*= NULL*/) {
		if(!piSize) {
			SetLastError(ERROR_INVALID_PARAMETER);
			return FALSE;
		}

		if(!pszClassIdList)
			pszClassIdList = L"CompositedWindow::Window";

		HTHEME hThemeWindow = OpenThemeData(NULL, pszClassIdList);
		
		if (hThemeWindow != NULL) {
			GetThemeInt(hThemeWindow, 0, 0, TMT_TEXTGLOWSIZE, piSize);
			CloseThemeData(hThemeWindow);
			return TRUE;
		}

		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}

	HRESULT CUxThemeAeroImpl::DrawThemeParentBackground(HWND hwnd, HDC hdc, const RECT *prc) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		DRAW_THEME_PARENT_BACKGROUND pfnDrawThemeParentBackground = (DRAW_THEME_PARENT_BACKGROUND)GetProcAddress(m_hUxThemeLib, "DrawThemeParentBackground");
		if(!pfnDrawThemeParentBackground)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDrawThemeParentBackground(hwnd, hdc, prc);
	}

	BOOL CUxThemeAeroImpl::IsThemeBackgroundPartiallyTransparent(HTHEME hTheme,int iPartId, int iStateId) {
		if(!IsInitialized()) {
			return FALSE;
		}

		IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT pfnIsThemeBackgroundPartiallyTransparent = (IS_THEME_BACKGROUND_PARTIALLY_TRANSPARENT)GetProcAddress(m_hUxThemeLib, "IsThemeBackgroundPartiallyTransparent");
		if(!pfnIsThemeBackgroundPartiallyTransparent)
			return FALSE;

		return pfnIsThemeBackgroundPartiallyTransparent(hTheme,iPartId, iStateId);
	}

	HRESULT CUxThemeAeroImpl::DrawThemeText(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		DRAW_THEME_TEXT pfnDrawThemeText = (DRAW_THEME_TEXT)GetProcAddress(m_hUxThemeLib, "DrawThemeText");
		if(!pfnDrawThemeText)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnDrawThemeText(hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);
	}

	HRESULT CUxThemeAeroImpl::GetThemeColor(HTHEME hTheme, int iPartId, int iStateId, int iPropId, COLORREF *pColor) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_COLOR pfnGetThemeColor = (GET_THEME_COLOR)GetProcAddress(m_hUxThemeLib, "GetThemeColor");
		if(!pfnGetThemeColor)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeColor(hTheme, iPartId, iStateId, iPropId, pColor);
	}

	HRESULT CUxThemeAeroImpl::GetThemePartSize(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize,SIZE *psz) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_PART_SIZE pfnGetThemePartSize = (GET_THEME_PART_SIZE)GetProcAddress(m_hUxThemeLib, "GetThemePartSize");
		if(!pfnGetThemePartSize)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemePartSize(hTheme, hdc, iPartId, iStateId, prc, eSize, psz);
	}

	HRESULT CUxThemeAeroImpl::GetThemePosition(HTHEME hTheme, int iPartId, int iStateId, int iPropId, POINT *pPoint) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_POSITION pfnGetThemePosition = (GET_THEME_POSITION)GetProcAddress(m_hUxThemeLib, "GetThemePosition");
		if(!pfnGetThemePosition)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemePosition(hTheme, iPartId, iStateId, iPropId, pPoint);
	}

	HRESULT CUxThemeAeroImpl::GetThemeMargins(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS *pMargins) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_MARGINS pfnGetThemeMargins = (GET_THEME_MARGINS)GetProcAddress(m_hUxThemeLib, "GetThemeMargins");
		if(!pfnGetThemeMargins)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeMargins(hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);
	}

	HRESULT CUxThemeAeroImpl::GetThemeMetric(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, int iPropId, int *piVal) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_METRIC pfnGetThemeMetric = (GET_THEME_METRIC)GetProcAddress(m_hUxThemeLib, "GetThemeMetric");
		if(!pfnGetThemeMetric)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeMetric(hTheme, hdc, iPartId, iStateId, iPropId, piVal);
	}

	HRESULT CUxThemeAeroImpl::GetThemeRect(HTHEME hTheme, int iPartId, int iStateId, int iPropId, LPRECT pRect) {
		if(!IsInitialized()) {
			return OLE_E_BLANK;
		}

		GET_THEME_RECT pfnGetThemeRect = (GET_THEME_RECT)GetProcAddress(m_hUxThemeLib, "GetThemeRect");
		if(!pfnGetThemeRect)
			return HRESULT_FROM_WIN32(GetLastError());

		return pfnGetThemeRect(hTheme, iPartId, iStateId, iPropId, pRect);
	}
}