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
#ifndef _WINDOW_H_INCLUDED_
#define _WINDOW_H_INCLUDED_

#pragma once

#define WM_ABRJOBCOMPLETE	WM_USER + 1
#define WM_ABRJOBINPROGRESS WM_ABRJOBCOMPLETE + 1

#define WM_ONDROP			WM_ABRJOBINPROGRESS + 1

#include "Extension.Parser.ABR.h"

namespace BigBrotherAndy {
	//////////////////////////////////////////////////////////////////////////
	//
	template <typename T, DWORD GrowSize = 10> class CStorage {
	public:
		CStorage() {
			m_dwRemaining	= 0;
			m_dwSize		= 0;
			Values			= 0;
			m_dwCount		= 0;
		};

		~CStorage() {
			if ( Values ) {
				delete [] Values;
			};
		}

		CStorage& operator=(const CStorage &rValue) {
			// Only do assignment if RHS is a different object from this.
			if (this != &rValue) {
				m_dwRemaining	= rValue.m_dwRemaining;
				m_dwSize		= rValue.m_dwSize;
				Values			= new T*[m_dwSize];
				memcpy ( Values, rValue.Values, sizeof(T*) * m_dwSize );
			}
			return *this;
		}

		DWORD Count() {
			return m_dwCount;
		};

		T* Value ( DWORD dwIndex ) {
			return Values[dwIndex];
		};

		void Add (T* pItem) {
			if ( !m_dwRemaining ) {
				T** pTemp = Values;
				Values = new T*[m_dwSize+GrowSize];
				if ( pTemp ) {
					memcpy ( Values, pTemp, sizeof(T*) * m_dwSize );
					delete [] pTemp;
				};
				m_dwSize += GrowSize;
				m_dwRemaining = GrowSize;
			};
			Values[m_dwSize-m_dwRemaining] = pItem;
			m_dwRemaining--;
			m_dwCount++;
		};

	private:
		T**		Values;
		DWORD	m_dwRemaining;
		DWORD	m_dwSize;
		DWORD   m_dwCount;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//
	class CBufferDC {
	public:
		CBufferDC (HWND hWnd, HDC hDC = 0, RECT* prcUpdate = 0, BOOL bAutoFlush = TRUE );
		~CBufferDC ();

		void FlushRect ( RECT* rc );

		operator HDC () {
			return m_hBufferDC;
		};

	private:
		HDC		m_hDC;
		HDC		m_hBufferDC;
		HBITMAP	m_hBitmap;
		HBITMAP m_hOldBitmap;
		HWND	m_hWnd;
		int		m_bReleaseDC;
		RECT	m_rcUpdate;
		RECT	m_rcSource;
		POINT	m_ptOrigin;
		POINT   m_ptViewport;
		SIZE	m_szViewport;
		SIZE	m_szExtent;
		BOOL	m_bAutoFlush;
	};

	//////////////////////////////////////////////////////////////////////////
	// class CFilesEnumerator
	//
	class CFilesEnumerator {
	public:
		class CItem {
		public:
			CItem ( WCHAR* lpwszFileName, unsigned long ulFileAttributes ) {
				m_lpwszFileName		= lpwszFileName;
				m_ulFileAttributes	= ulFileAttributes;
			};

			operator WCHAR* () {
				return m_lpwszFileName;
			};

			operator ULONG () {
				return m_ulFileAttributes;
			};

		private:
			WCHAR* m_lpwszFileName;
			unsigned long	m_ulFileAttributes;
		};

	public:
		CFilesEnumerator ( ) : m_hFind (INVALID_HANDLE_VALUE) {
		};

		~CFilesEnumerator () {
			if ( m_hFind != INVALID_HANDLE_VALUE ) {
				FindClose(m_hFind);
			};
		};

		CItem* Find (WCHAR* lpszRoot, WCHAR* lpwszMask  = L"*" ) {
			WCHAR wszCurrentDir[MAX_PATH];
			if ( SUCCEEDED ( StringCchPrintf ( wszCurrentDir, MAX_PATH, L"%s\\%s", lpszRoot, lpwszMask ) ) ) {
				m_hFind = FindFirstFileW((WCHAR*)(&wszCurrentDir), &m_hFindFileData);
				return m_hFind == INVALID_HANDLE_VALUE ? 0 : new CItem ( (WCHAR*)&m_hFindFileData.cFileName, m_hFindFileData.dwFileAttributes );
			} else {
				return 0;
			};
		}

		CItem* Next () {
			return m_hFind == INVALID_HANDLE_VALUE ? 0 : ( FindNextFileW(m_hFind, &m_hFindFileData) ? new CItem ( (WCHAR*)&m_hFindFileData.cFileName, m_hFindFileData.dwFileAttributes ) : 0 );
		}; 

	private:
		WIN32_FIND_DATAW	m_hFindFileData;
		HANDLE				m_hFind;
	};


	//////////////////////////////////////////////////////////////////////////
	// Simple window messaging API encapsulation
	//
	template<class T> class Window {
	public:
		Window(HINSTANCE hInstance, HWND hParent) : m_hInstance (hInstance), m_hWnd(0) {
		};

		virtual ~Window() {
		};

	public:
		virtual LRESULT OnNcDestroy(WPARAM wParam, LPARAM lParam) {
			return 0xFF00;
		};

		virtual LRESULT OnNcCreate(WPARAM wParam, LPARAM lParam) {
			return 0xFF00;
		};

		virtual LRESULT OnNcPaint(WPARAM wParam, LPARAM lParam) {
			return 0xFF00;
		};

		virtual LRESULT	OnNcHitTest(WPARAM wParam, LPARAM lParam) {
			return 0xFF00;
		};

		virtual LRESULT OnMove(int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT OnSize(int nType, int nWidth, int nHeight) {
			return 0xFF00;
		};

		virtual LRESULT OnPaint() {
			return 0xFF00;
		};

		virtual LRESULT OnExitSizeMove() {
			return 0xFF00;
		};

		virtual LRESULT OnShowWindow(BOOL bShow, int nStatus) {
			return 0xFF00;
		};

		virtual LRESULT	OnDestroy() {
			return 0xFF00;
		};

		virtual LRESULT OnCommand(BOOL bFromAccelerator, int nCtrlID, HWND hWndCtrl) {
			return 0xFF00;
		};

		virtual LRESULT	OnCreate(CREATESTRUCT* pCreate) {
			return 0xFF00;
		};

		virtual LRESULT	OnEraseBkgnd(HDC hDC) {
			return 0xFF00;
		};

		virtual LRESULT	OnTimer(unsigned int uID, TIMERPROC pTimerProc) {
			return 0xFF00;
		};

		virtual LRESULT	OnMessage(UINT uMessage, WPARAM wParam, LPARAM lParam) {
			return 0xFF00;
		};

		virtual LRESULT	OnDrawItem(UINT uControlID, LPDRAWITEMSTRUCT lpDrawItem) {
			return 0xFF00;
		};

		virtual LRESULT	OnMouseMove(UINT uVirtualKey, int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT OnMouseWheel ( WORD wKeys, short zDelta, WORD xPos, WORD yPos ) {
			return 0xFF00;
		};

		virtual LRESULT	OnLButtonDown(UINT uVirtualKey, int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT	OnLButtonUp(UINT uVirtualKey, int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT	OnRButtonDown(UINT uVirtualKey, int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT	OnRButtonUp(UINT uVirtualKey, int x, int y) {
			return 0xFF00;
		};

		virtual LRESULT OnInitDialog (HWND hFocus, LPARAM lInitParam ) {
			return 0xFF00;
		};

		virtual LRESULT OnNotify ( int idCtrl, LPNMHDR lpnm ) {
			return 0xFF00;
		};

		virtual LRESULT OnKeyUp ( UINT vkKeyCode,  UINT vkParams ) {
			return 0xFF00;
		};

		virtual LRESULT OnDropFiles ( HDROP hDrop ) {
			return 0xFF00;
		};

		virtual LRESULT OnABRJobInProgress ( DWORD dwTotalBrushes, DWORD dwCurrentBrush ) {
			return 0xFF00;
		};

		virtual LRESULT OnABRJobComplete ( DWORD dwTotalBrushes, IAdobeBrushes* pBrushes ) {
			return 0xFF00;
		};

		virtual LRESULT OnDrop ( STGMEDIUM* pstgmed, FORMATETC* pFormatEtc ) {
			return 0xFF00;
		};

		HINSTANCE	m_hInstance;	// Application instance
		HWND		m_hWnd;			// This window handle

		operator HWND() {
			return m_hWnd;
		};

		static LRESULT CALLBACK WndProc(HWND hWnd,  UINT uMessage,  WPARAM wParam,  LPARAM lParam) {
			LRESULT lResult = 0xFF00;

			#pragma warning(disable:4312)
			T *pWindow = reinterpret_cast<T*>(GetWindowLong(hWnd, GWL_USERDATA));
			#pragma warning(default:4312)

			if ( !pWindow ) {
				if ( uMessage == WM_NCCREATE ) {
					#pragma warning(disable:4312 4311)
					SetWindowLong(hWnd, GWL_USERDATA, (LONG)(((LPCREATESTRUCT)lParam)->lpCreateParams) );
					return reinterpret_cast<T*>(GetWindowLong(hWnd, GWL_USERDATA))->OnNcCreate(wParam,lParam);
					#pragma warning(default:4312 4311)
				} else {
					return DefWindowProc ( hWnd, uMessage, wParam, lParam );
				}; 
			}

			switch (uMessage) {
			case WM_MOUSEWHEEL:
				lResult = pWindow->OnMouseWheel ( GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam),  (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam) );
				break;

			case WM_ABRJOBCOMPLETE:
				lResult = pWindow->OnABRJobComplete ( (DWORD)wParam, (IAdobeBrushes*)lParam );
				break;

			case WM_ABRJOBINPROGRESS:
				lResult = pWindow->OnABRJobInProgress ( (DWORD)wParam, (DWORD)lParam );
				break;

			case WM_NCDESTROY:
				lResult = pWindow->OnNcDestroy(wParam,lParam);
				break;

			case WM_NCHITTEST:
				lResult = pWindow->OnNcHitTest(wParam,lParam);
				break;

			case WM_NCPAINT:
				lResult = pWindow->OnNcPaint(wParam,lParam);
				break;

			case WM_MOVE:
				lResult = pWindow->OnMove((int)(short) LOWORD(lParam),(int)(short) HIWORD(lParam));
				break;

			case WM_SIZE:
				lResult = pWindow->OnSize((int)wParam, (int)(short) LOWORD(lParam),(int)(short) HIWORD(lParam));
				break;

			case WM_ONDROP:
				lResult = pWindow->OnDrop ( (STGMEDIUM*)wParam, (FORMATETC*)lParam );
				break;

			case WM_DROPFILES:
				lResult = pWindow->OnDropFiles ( (HDROP) wParam );
				break;

			case WM_PAINT:
				lResult = pWindow->OnPaint();
				break;

			case WM_KEYUP:
				lResult = pWindow->OnKeyUp((UINT)wParam, (UINT)lParam);
				break;

			case WM_EXITSIZEMOVE:
				lResult = pWindow->OnExitSizeMove();
				break;

			case WM_SHOWWINDOW:
				lResult = pWindow->OnShowWindow((BOOL)wParam,(int)lParam);
				break;

			case WM_DESTROY:
				lResult = pWindow->OnDestroy();
				break;

			case WM_COMMAND:
				lResult = pWindow->OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
				break;

			case WM_CREATE:
				lResult = pWindow->OnCreate((CREATESTRUCT*)wParam);
				break;

			case WM_ERASEBKGND:
				lResult = pWindow->OnEraseBkgnd((HDC)wParam);
				break;

			case WM_TIMER:
				lResult = pWindow->OnTimer((unsigned int)wParam,(TIMERPROC)lParam);
				break;

			case WM_DRAWITEM:
				lResult = pWindow->OnDrawItem((UINT)wParam,(LPDRAWITEMSTRUCT)lParam);
				break;

			case WM_MOUSEMOVE:
				lResult = pWindow->OnMouseMove((UINT)wParam,((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
				break;

			case WM_LBUTTONDOWN:
				lResult = pWindow->OnLButtonDown((UINT)wParam,((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
				break;

			case WM_LBUTTONUP:
				lResult = pWindow->OnLButtonUp((UINT)wParam,((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
				break;

			case WM_RBUTTONDOWN:
				lResult = pWindow->OnRButtonDown((UINT)wParam,((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
				break;

			case WM_RBUTTONUP:
				lResult = pWindow->OnRButtonUp((UINT)wParam,((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)));
				break;

			case WM_INITDIALOG:
				lResult = pWindow->OnInitDialog((HWND)wParam,(LPARAM)lParam);
				break;

			case WM_NOTIFY:
				lResult = pWindow->OnNotify ( (int)wParam, (LPNMHDR) lParam );
				break;
			};

			if ( lResult & 0xFF00 ) {
				return DefWindowProc ( hWnd, uMessage, wParam, lParam );
			} else {
				return lResult;
			}
		}
	};
};

#endif
