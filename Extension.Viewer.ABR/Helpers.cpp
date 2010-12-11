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
}