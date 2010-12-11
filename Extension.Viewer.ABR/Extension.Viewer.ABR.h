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
#ifndef _EXTENSION_VIEWER_ABR_H_INCLUDED_
#define _EXTENSION_VIEWER_ABR_H_INCLUDED_

#pragma once

#define WINDOW_CLASS_NAME L"bba_ABRViewer_wndClass"

namespace BigBrotherAndy {
	///////////////////////////////////////////////////////////////////////////
	//
	enum ShowTimeState {
		ShowLoadingBrushes,
		ShowMultipleBrushes,
		ShowSingleBrush,
		ShowBrokenHeart,
	};

	///////////////////////////////////////////////////////////////////////////
	//
	class CABRViewer : public Window<CABRViewer> {
	public:
		CABRViewer();
		~CABRViewer();

	public:
		BOOL Create(HINSTANCE hInstance);
		BOOL BeginUnpackAdobeBrushesPresetFile(WCHAR* wcsFileName);


		LRESULT OnABRJobInProgress ( DWORD dwTotalBrushes, DWORD dwCurrentBrush );
		LRESULT OnABRJobComplete ( DWORD dwTotalBrushes, IAdobeBrushes* pBrushes );
		LRESULT OnMouseWheel ( WORD wKeys, short zDelta, WORD xPos, WORD yPos );
		LRESULT OnKeyUp ( UINT vkKeyCode,  UINT vkParams );
		LRESULT	OnNcHitTest(WPARAM wParam, LPARAM lParam);
		LRESULT OnDestroy();
		LRESULT	OnRButtonUp(UINT uVirtualKey, int x, int y);
		LRESULT	OnLButtonUp(UINT uVirtualKey, int x, int y);
		LRESULT	OnLButtonDown(UINT uVirtualKey, int x, int y);
		LRESULT	OnMouseMove(UINT uVirtualKey, int x, int y);
		LRESULT	OnEraseBkgnd(HDC hDC);
		LRESULT OnPaint();
		LRESULT OnDropFiles ( HDROP hDrop );
		LRESULT OnDrop ( STGMEDIUM* pstgmed, FORMATETC* pFormatEtc );

		void	BufferedDrawWindow ( HDC hDC, RECT* prc );
		void	DrawWindow ( HDC hDC, RECT* prc );

		void	Recolor ();

		void	Zoom ( BOOL bOut, WORD xPos, WORD yPos );
		void	Move ( BOOL bForward );

	public:
		HRESULT		DwmExtendFrameIntoClientArea();
		static BOOL UnpackingJobCallback (  ABRPARSER_JOB* pJobStatus );

	private:
		CDwmApiImpl			m_oDWMAPI;
		CUxThemeAeroImpl	m_oThemes;
	

		BOOL			m_fBrushDrag;
		HCURSOR			m_hArrowCursor, m_hDragCursor;
		POINT			m_ptDragPoint, m_ptOffset;

		RECT			m_rcClip;
		HFONT			m_hFont, m_hSmallFont, m_hSmileFont, m_hDetailsFont;

		double			m_fZoomFactor;


		DWORD			m_dwMagicNumber;
		DWORD			m_dwCurrentBrush;

		IAdobeBrushes*  m_pBrushes;

		HBRUSH			m_hFrameBrush, m_hBackgroundBrush;


		DWORD			m_clrBackground, m_clrHalfColor, m_clrQuaterColor;

		RGBQUAD			m_WhitePallette[256];
		RGBQUAD			m_BlackPallette[256];

		BOOL			m_bIsDecodingIsActive;
		BOOL			m_bTerminateApplication;

	public:
		ShowTimeState	m_uItsShowTimeBaby;
		CDropTarget*	m_pDropTarget;
	};
};

#endif