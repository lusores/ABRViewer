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
#include "Resource.h"
#include "DragDrop.h"
#include "WalkingDead.h"
#include "Extension.Viewer.ABR.h"

namespace BigBrotherAndy {

	#define DEF_HELP L"navigate between brushes\tleft, right arrows, mouse wheel\r\nzoom in/out\t\t\tup, down arrows, mouse wheel with ctrl\r\ninverse color\t\t\tspace\r\nsend to photoshop\t\tenter (return)\r\nclose window\t\t\tright mouse click, escape\r\nnext file:\t\t\tpgdn \r\nprev file:\t\t\tpgup"

	CABRViewer::CABRViewer() : Window<CABRViewer>(0,0) {
		m_uItsShowTimeBaby = ShowBrokenHeart;
		m_dwMagicNumber = 0;
		m_pBrushes = 0;

		m_hDragCursor = m_hArrowCursor = 0;
		m_hDetailsFont = m_hSmileFont = m_hFont = m_hSmallFont = 0;
		
		m_hFrameBrush = m_hBackgroundBrush = 0;
		
		m_fZoomFactor = 1.0;

		m_clrBackground = 0x000000;
		m_clrHalfColor = 0x0F0F0F;
		m_clrQuaterColor = 0x9F9F9F;


		for ( unsigned int i = 0; i < 256; i++ ) {
			m_BlackPallette[i].rgbBlue		= 
			m_BlackPallette[i].rgbRed		= 
			m_BlackPallette[i].rgbGreen		= 0xFF - i;
			m_BlackPallette[i].rgbReserved  = 0;

			m_WhitePallette[i].rgbBlue		= 
			m_WhitePallette[i].rgbRed		= 
			m_WhitePallette[i].rgbGreen		= i;
			m_WhitePallette[i].rgbReserved  = 0;
		};

		m_bIsDecodingIsActive	= FALSE;
		m_bTerminateApplication	= FALSE;


		m_pDropTarget = 0;
	}

	CABRViewer::~CABRViewer() {
		if ( m_pBrushes ) {
			m_pBrushes->Release();
		};

		if ( m_hDragCursor ) {
			DeleteObject ( m_hDragCursor );
		};

		if ( m_hArrowCursor ) {
			DeleteObject ( m_hArrowCursor );
		};

		if ( m_hSmallFont ) {
			DeleteObject ( m_hSmallFont );
		};

		if ( m_hFont ) {
			DeleteObject ( m_hFont );
		};

		if ( m_hSmileFont ) {
			DeleteObject ( m_hSmileFont );
		};

		if ( m_hDetailsFont ) {
			DeleteObject ( m_hDetailsFont );
		};

		if ( m_hFrameBrush ) {
			DeleteObject(m_hFrameBrush);
		};

		if ( m_hBackgroundBrush ) {
			DeleteObject ( m_hBackgroundBrush );
		};
	}

	BOOL CABRViewer::Create(HINSTANCE hInstance) {
		// Take it
		m_hInstance = hInstance;

		//If the window class has not been registered, then do so.
		WNDCLASS wc;
		if ( !GetClassInfo ( m_hInstance, WINDOW_CLASS_NAME, &wc ) ) {
			wc.style          = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc    = (WNDPROC)this->WndProc;
			wc.cbClsExtra     = 0;
			wc.cbWndExtra     = 0;
			wc.hInstance      = m_hInstance;
			wc.hIcon          = 0;
			wc.hCursor        = LoadCursor ( 0, IDC_ARROW );
			wc.hbrBackground  = 0;
			wc.lpszMenuName   = 0;
			wc.lpszClassName  = WINDOW_CLASS_NAME;
			if ( !RegisterClass ( &wc ) ) {
				return FALSE;
			};
		};

		// Create the window. The WndProc will set m_hWnd
		if ( ! ( m_hWnd = CreateWindowEx ( WS_EX_ACCEPTFILES, WINDOW_CLASS_NAME, NULL, WS_OVERLAPPED | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, 0, 0, m_hInstance, (LPVOID)this )) ) {
				return FALSE;
		};

		// Create arrow cursor
		m_hArrowCursor = LoadCursor ( NULL,IDC_ARROW );
		m_hDragCursor = LoadCursor(NULL,IDC_CROSS);

		// Create font
		// Create font (lovely Tahoma family)
		m_hFont = CreateFont ( -112, 0, 0, 0, FW_DEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma" );

		m_hSmileFont = CreateFont ( -248, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Wingdings" );

		m_hSmallFont = CreateFont ( -14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma" );

		m_hDetailsFont = CreateFont ( -11, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma" );

		//
		Recolor();

		// Show window
		ShowWindow ( m_hWnd, SW_SHOW );

		// Update window
		UpdateWindow ( m_hWnd );

		return TRUE;
	}

	void CABRViewer::Recolor() {
		m_clrBackground = ( ~m_clrBackground ) & 0x00FFFFFF;

		if ( m_clrBackground ) {
			m_clrHalfColor = 0x9F9F9F;
			m_clrQuaterColor = 0x0F0F0F;
		} else {
			m_clrHalfColor = 0x0F0F0F;
			m_clrQuaterColor = 0x9F9F9F;
		}

		// Clean-up
		if ( m_hFrameBrush ) {
			DeleteObject(m_hFrameBrush);
		};

		if ( m_hBackgroundBrush ) {
			DeleteObject ( m_hBackgroundBrush );
		};

		// Yeah, baby!
		m_hFrameBrush = CreateSolidBrush ( m_clrHalfColor );
		m_hBackgroundBrush = CreateSolidBrush ( m_clrBackground );

		if ( m_pBrushes ) {
			for ( DWORD i = 0; i < m_pBrushes->Count(); i++ ) {
				if ( IAdobeBrush* pBrush = m_pBrushes->Get(i) ) {
					if ( pBrush->Type() == AdobeBrushTypeSampled ) {
						memcpy ( ((AdobeBrushDibData*)pBrush->Data())->oHeader->bmiColors + 1, m_clrBackground ? m_BlackPallette : m_WhitePallette, 256 * sizeof(RGBQUAD) );
					};
					pBrush->Release();
				};
			};
		};
	}

	LRESULT CABRViewer::OnDrop ( STGMEDIUM* pstgmed, FORMATETC* pFormatEtc ) {
		//
		if ( !pFormatEtc && !pstgmed ) {
			return 0;
		};

		//
		if ( pFormatEtc->cfFormat == CF_HDROP && pFormatEtc->tymed == TYMED_HGLOBAL ) {
			// we asked for the data as a HGLOBAL, so access it appropriately
			HDROP hDrop = (HDROP)GlobalLock(pstgmed->hGlobal);
			OnDropFiles ( hDrop );
			GlobalUnlock ( pstgmed->hGlobal );
		};

		return 0x00;
	};

	LRESULT CABRViewer::OnDropFiles ( HDROP hDrop ) {
		if ( !hDrop ) {
			return 0;
		};

		WCHAR wsFileName[MAX_PATH];
		if ( DragQueryFile ( hDrop, 0, wsFileName, MAX_PATH ) ) {
			YetAnotherBeginUnpack ( wsFileName );
			m_WalkingDead.reset();
		};

		DragFinish(hDrop);

		return 0;
	}

	void CABRViewer::YetAnotherBeginUnpack ( WCHAR* wcsFileName ) {
		if ( m_uItsShowTimeBaby != ShowLoadingBrushes ) {
			m_uItsShowTimeBaby = ShowLoadingBrushes;
			m_dwMagicNumber = 0;

			if ( HDC hDC = GetDC ( m_hWnd ) ) {
				RECT rc;
				GetClientRect ( m_hWnd, &rc );
				BufferedDrawWindow ( CBufferDC ( m_hWnd, hDC ), &rc );
				ReleaseDC ( m_hWnd, hDC );
			};

			if ( m_pBrushes ) {
				m_pBrushes->Release();
				m_pBrushes = 0;
			};

			BeginUnpackAdobeBrushesPresetFile ( wcsFileName );
			SetWindowText ( m_hWnd, wcsFileName );
		};
	}

	void CABRViewer::BufferedDrawWindow ( HDC hDC, RECT* prc ) {
		DrawWindow ( CBufferDC ( m_hWnd, hDC ), prc );
	}

	void CABRViewer::DrawWindow ( HDC hDC, RECT* prc ) {
		// Hey, bro!
		FillRect ( hDC, prc, m_hBackgroundBrush );

		// What do you wand to do today?
		switch ( m_uItsShowTimeBaby ) {
		case ShowBrokenHeart:
			{
				HFONT hOldFont = (HFONT)SelectObject ( hDC, m_hSmileFont );

				SetBkMode ( hDC, TRANSPARENT );

				SetTextColor ( hDC, m_clrHalfColor );
				DrawText ( hDC, L"J", -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );

				SetTextColor ( hDC, m_clrQuaterColor );
				hOldFont = (HFONT)SelectObject ( hDC, m_hSmallFont );
				DrawText ( hDC, L"Drop point ;)", -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );
				SelectObject ( hDC, m_hDetailsFont );
				prc->left += 10;
				prc->top += 10;
				DrawText ( hDC, DEF_HELP, -1, prc, DT_WORDBREAK | DT_EXPANDTABS );
				SelectObject ( hDC, hOldFont );
			};
			break;
		case ShowLoadingBrushes: 
			{
				HFONT hOldFont = (HFONT)SelectObject ( hDC, m_hFont );

				WCHAR wsProgress[10];
				wsprintf ( wsProgress, L"%d", m_dwMagicNumber );

				SetBkMode ( hDC, TRANSPARENT );

				SetTextColor ( hDC, m_clrHalfColor );
				DrawText ( hDC, wsProgress, -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );

				SetTextColor ( hDC, m_clrQuaterColor );
				hOldFont = (HFONT)SelectObject ( hDC, m_hSmallFont );

				DrawText ( hDC, L"One moment please...", -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );

				SelectObject ( hDC, m_hDetailsFont );
				prc->left += 10;
				prc->top += 10;
				DrawText ( hDC, DEF_HELP, -1, prc, DT_WORDBREAK | DT_EXPANDTABS );
				SelectObject ( hDC, hOldFont );
			};
			break;
		case ShowSingleBrush:
			if ( m_pBrushes ) {
				if ( IAdobeBrush* pBrush = m_pBrushes->Get ( m_dwCurrentBrush ) ) {
					RECT rc;
					memcpy ( &rc, prc, sizeof(RECT) );

					OffsetRect ( &rc, -rc.left + 10, -rc.top + 10);

					double width = rc.right - rc.left - 20;
					double height = rc.bottom - rc.top - 20;

					double w = pBrush->Width();
					double h = pBrush->Height();

					double raten = min(1, min ((width/w), (height/h)));

					h *= ( raten * m_fZoomFactor );
					w *= ( raten * m_fZoomFactor );

					double pointx = ( width  - w  ) / 2 + 10 + m_ptOffset.x;
					double pointy = ( height - h ) / 2 + 10 + m_ptOffset.y;

					if ( pBrush->Type() == 1 ) {
						w = rc.right - rc.left;
						h = rc.bottom - rc.top;
						pointx = rc.left;
						pointy = rc.top;
					};


					SetBkColor ( hDC,m_clrHalfColor );
					SetBkMode ( hDC, TRANSPARENT );

					if ( w < pBrush->Width() || h < pBrush->Height() ) {
						SetStretchBltMode(hDC, HALFTONE );
					} else {
						SetStretchBltMode(hDC, BLACKONWHITE );
					};

					//////////////////////////////////////////////////////////////////////////
					if ( pBrush->Type() == AdobeBrushTypeSampled ) {
						// Draw pixels to window    
						StretchDIBits ( hDC, (int)pointx, (int)pointy, (int)w, (int)h, 0, 0, pBrush->Width(), pBrush->Height(), ((AdobeBrushDibData*)pBrush->Data())->pBits, ((AdobeBrushDibData*)pBrush->Data())->oHeader, DIB_RGB_COLORS, SRCCOPY );
					} else {
						HFONT hOldFont = (HFONT)SelectObject ( hDC, m_hSmileFont );

						SetTextColor ( hDC, m_clrHalfColor );
						DrawText ( hDC, L"A", -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );

						SetTextColor ( hDC, m_clrQuaterColor );
						SelectObject ( hDC, m_hSmallFont );

						DrawText ( hDC, L"Sampled brush", -1, prc, DT_CENTER | DT_VCENTER | DT_SINGLELINE  );

						SelectObject ( hDC, hOldFont );
					}

					RECT rcText;
					rcText.top = prc->bottom - 80;
					rcText.bottom = rcText.top + 50;
					rcText.left = 0;
					rcText.right = 180;

					FillRect ( hDC, &rcText, m_hFrameBrush  );

					SetTextColor ( hDC,0xFFFFFF );

					rcText.left += 10;
					rcText.top += 10;

					HFONT hOldFont = (HFONT)SelectObject ( hDC, m_hDetailsFont );

					if ( IAdobeProperty* pProperty = pBrush->GetProperty ( L"Nm  ", 4, PropertyTypeText ) ) {
						WCHAR* Name = wcsstr ( pProperty->Value().wValue, L"=" );
						DrawText ( hDC, Name ? Name + 1 : pProperty->Value().wValue, -1, &rcText, DT_LEFT | DT_TOP );
						pProperty->Release();
					} else {
						DrawText ( hDC, L"Unnamed brush", -1, &rcText, DT_LEFT | DT_TOP );
					}

					rcText.top += 12;

					WCHAR wsSize[60];
					wsprintf ( wsSize, L"%d of %d. Sample size: %dpx", m_dwCurrentBrush + 1, m_pBrushes->Count(), max(pBrush->Width(), pBrush->Height()) );

					DrawText ( hDC, wsSize, -1, &rcText, DT_LEFT | DT_TOP );

					SelectObject ( hDC, hOldFont );

					//////////////////////////////////////////////////////////////////////////

					HRGN hrgn = CreateRectRgn(min(m_rcClip.left,(long)pointx), min(m_rcClip.top,(long)pointy), max(m_rcClip.right,(long)(pointx+w)), max(m_rcClip.bottom,(long)(pointy+h)) );
					SelectClipRgn ( hDC, hrgn ); 

					m_rcClip.left = (long)pointx;
					m_rcClip.top = (long)pointy;

					m_rcClip.right = m_rcClip.left + (long)w;
					m_rcClip.bottom = m_rcClip.top + (long)h;

					pBrush->Release();
				};
			};
			break;
		case ShowMultipleBrushes:
			// Always halftone stretch mode
			SetStretchBltMode(hDC, HALFTONE );

			if ( m_pBrushes ) {
				RECT rc;
				memcpy ( &rc, prc, sizeof(RECT) );

				OffsetRect ( &rc, -rc.left + 10, -rc.top + 10 );
				rc.bottom -= 20;
				rc.right -= 20;

				int count = (int)(1 / m_fZoomFactor );
				int offset = 10;
				int width = ( offset + rc.right - rc.left ) / (long)(1 / m_fZoomFactor) - offset; 
				int height = ( offset + rc.bottom - rc.top ) / (long)(1 / m_fZoomFactor) - offset; 

				for ( int y = 0; y < count ; y++ ) {
					for ( int x = 0; x < count; x++ ) {
						if ( IAdobeBrush* pBrush = m_pBrushes->Get ( m_dwCurrentBrush + y * count + x  ) ) {
							RECT rcBrush;
							
							rcBrush.left  = rc.left + ( width + offset ) * x;
							rcBrush.right = rcBrush.left + width;

							rcBrush.top  = rc.top + ( height + offset ) * y;
							rcBrush.bottom = rcBrush.top + height;

							FrameRect ( hDC, &rcBrush, m_hFrameBrush ); 

							double raten = min(1, min (((width-4.0)/pBrush->Width()), ((height-4.0)/pBrush->Height())));

							int w = (int)(raten * pBrush->Width());
							int h = (int)(raten * pBrush->Height());

							if ( pBrush->Type() == AdobeBrushTypeSampled ) {
								// Draw pixels to window    
								StretchDIBits ( hDC, rcBrush.left + ( width - w ) / 2, rcBrush.top + ( height - h ) / 2, (int)w, (int)h, 0, 0, pBrush->Width(), pBrush->Height(), ((AdobeBrushDibData*)pBrush->Data())->pBits, ((AdobeBrushDibData*)pBrush->Data())->oHeader, DIB_RGB_COLORS, SRCCOPY );
							};

							pBrush->Release();
						};
					};
				};
			};
			break;
		};
	}

	LRESULT CABRViewer::OnPaint() {
		// Small and simple
		PAINTSTRUCT		ps;
		HDC	 hDC = BeginPaint ( m_hWnd, &ps );
		RECT rc;
		GetClientRect ( m_hWnd, &rc );
		BufferedDrawWindow( hDC, &rc );
		EndPaint ( m_hWnd, &ps );
		return 0;
	}

	LRESULT	CABRViewer::OnEraseBkgnd(HDC hDC) {
		return 0x0;
	};

	LRESULT	CABRViewer::OnDestroy() {
		if ( m_bIsDecodingIsActive ) {
			m_bTerminateApplication = TRUE;
			ShowWindow ( m_hWnd, SW_HIDE );
		} else {
			if ( m_pDropTarget ) {
				CDropTarget::UnregisterDropWindow ( m_hWnd, m_pDropTarget );
			};
			PostQuitMessage(0);
		};
		return 0;
	}

	LRESULT	CABRViewer::OnMouseMove(UINT uVirtualKey, int x, int y) {
		if ((uVirtualKey && MK_LBUTTON) && m_fBrushDrag ) {
			m_ptOffset.x += ( x - m_ptDragPoint.x );
			m_ptOffset.y += ( y - m_ptDragPoint.y );

			InvalidateRect ( m_hWnd, 0, FALSE );

			// Save the coordinates of the mouse cursor.  
			m_ptDragPoint.x = x;
			m_ptDragPoint.y = y;
		} 
		return 0; 
	}

	LRESULT CABRViewer::OnLButtonDown(UINT uVirtualKey, int x, int y) {
		RECT rc;
		GetWindowRect ( m_hWnd, &rc );

		// Restrict the mouse cursor to the client area. This  
		// ensures that the window receives a matching  
		// WM_LBUTTONUP message.  
		ClipCursor(&rc); 

		// Save the coordinates of the mouse cursor.  
		m_ptDragPoint.x = x; 
		m_ptDragPoint.y = y; 

		SetCursor ( m_hDragCursor );
		SetClassLong ( m_hWnd, GCL_HCURSOR, (__int3264)(LONG_PTR)m_hDragCursor ); 

		m_fBrushDrag = TRUE; 

		return 0; 
	}

	LRESULT	CABRViewer::OnLButtonUp(UINT uVirtualKey, int x, int y) {
		// Check
		if ( m_fBrushDrag ) {
			m_fBrushDrag = FALSE;
			SetCursor ( m_hArrowCursor );
			SetClassLong ( m_hWnd, GCL_HCURSOR, (__int3264)(LONG_PTR)m_hArrowCursor); 

			// Release the mouse cursor.  
			ClipCursor(0); 
		};
		return 0; 
	}

	LRESULT	CABRViewer::OnRButtonUp(UINT uVirtualKey, int x, int y) {
		return OnDestroy();
	}

	LRESULT CABRViewer::OnLButtonDblClk ( UINT vkCode, int x, int y ) {
		if ( m_bIsDecodingIsActive || !m_wBrushesFile ) {
			return 0;
		};

		// Lookup for photoshop executable
		if ( !m_wPhotoshopExecutable.get() ) {
			IEnumAssocHandlers* pEnumAssocHandlers = 0;
			if ( SUCCEEDED ( SHAssocEnumHandlers ( L".abr", ASSOC_FILTER_RECOMMENDED, &pEnumAssocHandlers ) ) && pEnumAssocHandlers ) {
				ULONG ulFetched = 0;
				IAssocHandler* pAssocHandler = 0;
				while ( SUCCEEDED ( pEnumAssocHandlers->Next(1, &pAssocHandler, &ulFetched ) ) && pAssocHandler ) {
					LPWSTR lpwExecutableName = 0;
					if ( SUCCEEDED ( pAssocHandler->GetName(&lpwExecutableName) ) && lpwExecutableName ) {
						if ( wcsstr ( lpwExecutableName, L"otoshop.exe" ) ) {
							size_t dwLenght;
							StringCchLength ( lpwExecutableName, MAX_PATH, &dwLenght );
							m_wPhotoshopExecutable.reset ( new WCHAR[dwLenght+1] );
							StringCchCopy ( m_wPhotoshopExecutable, dwLenght + 1, lpwExecutableName );
							pAssocHandler->Release();
							break;
						};
					};
					pAssocHandler->Release();
				};
				pEnumAssocHandlers->Release();
			};
		};

		if ( m_wPhotoshopExecutable.get() ) {
			ShellExecute ( m_hWnd, 0, m_wPhotoshopExecutable, m_wBrushesFile, 0, SW_SHOW );
		};

		return 0;
	};


	LRESULT	CABRViewer::OnNcHitTest(WPARAM wParam, LPARAM lParam) {
		if ( ( GetKeyState (VK_CONTROL) & 0x8000 ) || ( GetKeyState ( GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON ) & 0x8000 ) ) {
			return HTCLIENT;
		} else {
			return HTCAPTION;
		}
	}

	void CABRViewer::Zoom ( BOOL bOut, WORD xPos, WORD yPos ) {
		if ( bOut ) {
			if ( m_fZoomFactor == 1 ) {
				m_fZoomFactor = 0.5;
				m_uItsShowTimeBaby = ShowMultipleBrushes;
				InvalidateRect ( m_hWnd, 0, FALSE );
			} else if ( m_fZoomFactor < 1 ) {
				if ( m_fZoomFactor != 0.125 ) {
					m_fZoomFactor /= 2;
				};
				InvalidateRect ( m_hWnd, 0, FALSE );
			} else {
				if ( m_fZoomFactor != 1 ) {
					m_fZoomFactor--;
					if ( m_fZoomFactor < 1 ) {
						m_fZoomFactor = 1;
					};
					m_ptOffset.x /= 2;
					m_ptOffset.y /= 2;
					InvalidateRect ( m_hWnd, 0, FALSE );
				};
			}
		} else {
			if ( m_fZoomFactor < 1 ) {

				int count = (int)(1 / m_fZoomFactor );

				RECT rcRect;
				GetWindowRect(m_hWnd, &rcRect);

				int x  = ( xPos - rcRect.left ) / (( rcRect.right - rcRect.left ) / count );
				int y  = ( yPos - rcRect.top ) / (( rcRect.bottom - rcRect.top ) / count );

				m_dwCurrentBrush += ( count * y + x );

				if ( m_dwCurrentBrush >= m_pBrushes->Count() ) {
					m_dwCurrentBrush = m_pBrushes->Count() - 1;
				}

				m_fZoomFactor *= 2;

				if ( m_fZoomFactor == 1 ) {
					m_uItsShowTimeBaby = ShowSingleBrush;
				};

				InvalidateRect ( m_hWnd, 0, FALSE );
			} else if ( m_fZoomFactor != 10 ) {
				m_fZoomFactor++;
				if ( m_fZoomFactor > 10 ) {
					m_fZoomFactor = 10;
				};
				InvalidateRect( m_hWnd, 0, FALSE );
			};
		};
	}

	void CABRViewer::Move ( BOOL bForward ) {
		if ( m_pBrushes ) {
			switch ( m_uItsShowTimeBaby ) {
			case ShowSingleBrush:
				if ( bForward ) {
					if ( m_dwCurrentBrush != ( m_pBrushes->Count() - 1 ) ) {
						m_dwCurrentBrush++;
						m_fZoomFactor = 1;
						ZeroMemory ( &m_ptOffset, sizeof(POINT) );
						InvalidateRect ( m_hWnd, 0, FALSE);
					};
				} else {
					if ( m_dwCurrentBrush != 0 ) {
						m_dwCurrentBrush--;
						m_fZoomFactor = 1;
						ZeroMemory ( &m_ptOffset, sizeof(POINT) );
						InvalidateRect ( m_hWnd, 0, FALSE);
					};
				};
				break;
			case ShowMultipleBrushes:
				{
					unsigned int count = (int)(1 / m_fZoomFactor );
					count *= count;

					if ( bForward ) {
						if ( ( m_dwCurrentBrush + count ) < m_pBrushes->Count() ) {
							m_dwCurrentBrush += count;
							InvalidateRect ( m_hWnd, 0, FALSE);
						};
					} else {
						if ( m_dwCurrentBrush && ( m_dwCurrentBrush < count ) ) {
							m_dwCurrentBrush = 0;
							InvalidateRect ( m_hWnd, 0, FALSE);
						} if ( m_dwCurrentBrush >= count ) {
							m_dwCurrentBrush -= count;
							InvalidateRect ( m_hWnd, 0, FALSE);
						};
					};
				}
				break;
			}
		};
	}

	LRESULT CABRViewer::OnMouseWheel ( WORD wKeys, short zDelta, WORD xPos, WORD yPos ) {
		wKeys & MK_CONTROL ? Zoom ( ((short)zDelta) < 0, xPos, yPos ) : Move ( ((short)zDelta) < 0 );
		return 0L;
	}

	LRESULT CABRViewer::OnKeyUp ( UINT vkKeyCode,  UINT vkParams ) {
		BOOL bUpdate = FALSE;

		switch ( vkKeyCode ) {
		case VK_SPACE:
			Recolor();
			bUpdate = TRUE;
			break;
		case VK_LEFT:
			Move ( FALSE );
			bUpdate = TRUE;
			break;
		case VK_RIGHT:
			Move ( TRUE );
			bUpdate = TRUE;
			break;
		case VK_UP:
			Zoom ( FALSE, 0, 0 );
			bUpdate = TRUE;
			break;
		case VK_DOWN:
			Zoom ( TRUE, 0, 0 );
			bUpdate = TRUE;
			break;
		case VK_ESCAPE:
			OnRButtonUp(0,0,0);
			break;
		case VK_RETURN:
			OnLButtonDblClk (0,0,0);
			break;
		case VK_NEXT:
			if ( !m_WalkingDead ) {
				m_WalkingDead.reset ( new _WalkingDead() );
				m_WalkingDead->Enum ( m_wBrushesFile );
			};

			if ( m_WalkingDead->IsValid() ) {
				if ( WCHAR* wcsNewFile = m_WalkingDead->Next() ) {
					if ( m_uItsShowTimeBaby != ShowLoadingBrushes ) {
						YetAnotherBeginUnpack(wcsNewFile);
					};
				};
			}

			break;
		case VK_PRIOR:
			if ( !m_WalkingDead ) {
				m_WalkingDead.reset ( new _WalkingDead() );
				m_WalkingDead->Enum ( m_wBrushesFile );
			};

			if ( m_WalkingDead->IsValid() ) {
				if ( WCHAR* wcsNewFile = m_WalkingDead->Prev() ) {
					if ( m_uItsShowTimeBaby != ShowLoadingBrushes ) {
						YetAnotherBeginUnpack(wcsNewFile);
					};
				};
			};

			break;
		};

		if ( bUpdate ) {
			if ( HDC hDC = GetDC ( m_hWnd ) ) {
				RECT rc;
				GetClientRect ( m_hWnd, &rc );
				BufferedDrawWindow ( CBufferDC ( m_hWnd, hDC ), &rc );
				ReleaseDC ( m_hWnd, hDC );
			};
		};

		return 0xFF01;
	};


	LRESULT CABRViewer::OnABRJobInProgress ( DWORD dwTotalBrushes, DWORD dwCurrentBrush ) {
		m_uItsShowTimeBaby = ShowLoadingBrushes;
		m_dwMagicNumber = dwCurrentBrush;
		if ( HDC hOriginalDC = GetDC ( m_hWnd ) ) {
			RECT rc;
			GetClientRect ( m_hWnd, &rc );
			BufferedDrawWindow ( CBufferDC ( m_hWnd, hOriginalDC ), &rc );
			ReleaseDC ( m_hWnd, hOriginalDC );
		};
		return 0L;
	}

	LRESULT CABRViewer::OnABRJobComplete ( DWORD dwTotalBrushes, IAdobeBrushes* pBrushes ) {
		m_uItsShowTimeBaby = ( dwTotalBrushes && pBrushes ) ? ShowSingleBrush : ShowBrokenHeart;
		m_dwCurrentBrush = 0;

		m_fZoomFactor = 1.0;

		if ( m_pBrushes ) {
			m_pBrushes->Release();
		};

		//
		// m_clrBackground = 0x000000;
		// Recolor();

		m_pBrushes = pBrushes;
		m_dwMagicNumber = dwTotalBrushes;

		m_uItsShowTimeBaby = ShowMultipleBrushes;
		m_fZoomFactor = 0.5;

		if ( HDC hOriginalDC = GetDC ( m_hWnd ) ) {
			RECT rc;
			GetClientRect ( m_hWnd, &rc );
			BufferedDrawWindow ( CBufferDC ( m_hWnd, hOriginalDC ), &rc );
			ReleaseDC ( m_hWnd, hOriginalDC );
		};

		m_bIsDecodingIsActive = FALSE;

		if ( m_bTerminateApplication ) {
			if ( m_pDropTarget ) {
				CDropTarget::UnregisterDropWindow ( m_hWnd, m_pDropTarget );
			};
			PostQuitMessage(0);
		};

		return 0L;
	}

	BOOL CABRViewer::BeginUnpackAdobeBrushesPresetFile(WCHAR* wcsFileName) {
		// 
		// m_clrBackground = 0xFFFFFF;
		// Recolor();

		m_bIsDecodingIsActive	= TRUE;

		size_t dwLenght;
		StringCchLength ( wcsFileName, MAX_PATH, &dwLenght );
		m_wBrushesFile.reset ( new WCHAR[dwLenght+1] );
		StringCchCopy ( m_wBrushesFile, dwLenght+1, wcsFileName );

		ReadAdobePhotoshopBrushFile ( wcsFileName, AdobeBrushDataTypeFile, AdobeBrushDataTypeDIB, UnpackingJobCallback, this );
		return TRUE;
	}

	BOOL CABRViewer::UnpackingJobCallback (  ABRPARSER_JOB* pJobStatus ) {
		if ( pJobStatus ) {
			if ( pJobStatus->bJobFinished ) {
				SendMessage ( ((CABRViewer*)(pJobStatus->pParam))->m_hWnd, WM_ABRJOBCOMPLETE, (WPARAM)pJobStatus->dwBrushesTotal, (LPARAM)pJobStatus->pBrushes );
			} else {
				SendMessage ( ((CABRViewer*)(pJobStatus->pParam))->m_hWnd, WM_ABRJOBINPROGRESS, (WPARAM)pJobStatus->dwBrushesTotal, (LPARAM)pJobStatus->dwCurrentBrush );
			}
		};
		return TRUE;
	}
}


CABRViewer g_AdobeBrushViewerApp;

typedef BOOL (__stdcall *CHANGEWINDOWMESSAGEFILTER)( UINT message, DWORD dwFlag);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	//
	MSG		msg;
	HACCEL	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EXTENSIONVIEWERABR));
	WCHAR** szArgList;  
	int		argCount;  

	g_AdobeBrushViewerApp.m_uItsShowTimeBaby = ShowLoadingBrushes;

	// Create application instance
	g_AdobeBrushViewerApp.Create ( hInstance );

	// Parse command line
	szArgList = CommandLineToArgvW ( GetCommandLine(), &argCount );
	if ( szArgList && argCount >= 2 ) {
		g_AdobeBrushViewerApp.BeginUnpackAdobeBrushesPresetFile ( szArgList[1] );
		SetWindowText ( g_AdobeBrushViewerApp.m_hWnd, szArgList[1] );
	} else {
		g_AdobeBrushViewerApp.m_uItsShowTimeBaby = ShowBrokenHeart;
		SetWindowText ( g_AdobeBrushViewerApp.m_hWnd, L"Lightweight .ABR Viewer" );

		InvalidateRect ( g_AdobeBrushViewerApp.m_hWnd, 0, FALSE );
	};

	if ( OleInitialize(0) != S_OK || !CDropTarget::RegisterDropWindow ( g_AdobeBrushViewerApp.m_hWnd, &g_AdobeBrushViewerApp.m_pDropTarget ) ) {
		if ( HMODULE hUser32 = LoadLibrary(L"user32.dll" ) ) {
			if ( CHANGEWINDOWMESSAGEFILTER fnChangeWindowMessageFilter = (CHANGEWINDOWMESSAGEFILTER)GetProcAddress ( hUser32, "ChangeWindowMessageFilter") ) {
				fnChangeWindowMessageFilter ( 73 , MSGFLT_ADD );
				fnChangeWindowMessageFilter ( WM_DROPFILES, MSGFLT_ADD);
			}
			FreeLibrary(hUser32);
		};
		DragAcceptFiles ( g_AdobeBrushViewerApp.m_hWnd, TRUE );
	}

	// Enter main message loop
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		};
	};

	// anyway we're going out of there
	// so one and only one failed call is not a bug
	// ...
	// i think
	OleUninitialize();
	
	return (int) msg.wParam;
}
