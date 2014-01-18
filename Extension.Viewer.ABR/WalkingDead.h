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
#ifndef _WALKINGDEAD_H_INCLUDED_
#define _WALKINGDEAD_H_INCLUDED_

#pragma once

namespace BigBrotherAndy {
	///////////////////////////////////////////////////////////////////////////
	// spike-nails on

	class _WalkingDead {
	public:
		_WalkingDead ();
		~_WalkingDead ();

	public:
		void   Enum ( WCHAR* wcsFileName );
		WCHAR* Next ( );
		WCHAR* Prev ( );
		int	   IsValid () { return m_bIsValid; };

	private:
		class _Item {
		public:
			_Item*			m_Prev;
			CAutoPtr<_Item> m_Next;
			CAutoPtr<WCHAR> m_pFileName;
		};

		CAutoPtr<_Item>	m_pRoot;
		int				m_bIsValid;
		_Item*			m_pCurrent;
	};

	class _FileEnumerator {
	public:
		class _Item {
		public:
			_Item ( WCHAR* lpwszFileName, unsigned long ulFileAttributes ) {
				if ( lpwszFileName ) {
					size_t dwLenght;
					if ( SUCCEEDED ( StringCchLength ( lpwszFileName, MAX_PATH, &dwLenght ) ) ) {
						m_pFileName.reset ( new WCHAR[dwLenght+1] );
						if ( FAILED ( StringCchCopy ( m_pFileName, dwLenght+1, lpwszFileName ) ) ) {
							m_pFileName.reset();
						};
					};
				};
				m_ulFileAttributes	= ulFileAttributes;
			};

			operator WCHAR* () {
				return m_pFileName.get();
			};

			operator ULONG () {
				return m_ulFileAttributes;
			};

			CAutoPtr<WCHAR>	m_pFileName;
			unsigned long	m_ulFileAttributes;
		};

	public:
		_FileEnumerator  ( WCHAR* lpwszRoot, WCHAR* lpwszMask  = L"*" ) {
			m_hFind = INVALID_HANDLE_VALUE;
			if ( lpwszRoot ) {
				m_pRoot.reset(new WCHAR[MAX_PATH]);
				wsprintf ( m_pRoot, L"%s\\%s", (WCHAR*)lpwszRoot, lpwszMask ? lpwszMask : L"*.*" );
			};
		};

		~_FileEnumerator () {
			if ( m_hFind != INVALID_HANDLE_VALUE ) {
				FindClose(m_hFind);
			};
		};

		_Item* Next () {
			if ( m_pRoot ) {
				if ( m_hFind == INVALID_HANDLE_VALUE ) {
					return ( m_hFind = FindFirstFileW((WCHAR*)(m_pRoot), &m_hFindFileData) ) == INVALID_HANDLE_VALUE ? 0 : m_pItem.reset ( new _Item ( (WCHAR*)&m_hFindFileData.cFileName, m_hFindFileData.dwFileAttributes ) );
				};
				return FindNextFileW(m_hFind, &m_hFindFileData) ? 
					m_pItem.reset ( new _Item ( (WCHAR*)&m_hFindFileData.cFileName, m_hFindFileData.dwFileAttributes ) ) : 0;
			};
			return 0;
		}

	private:
		WIN32_FIND_DATAW	m_hFindFileData;
		HANDLE				m_hFind;
		CAutoPtr<_Item>		m_pItem;
		CAutoPtr<WCHAR>		m_pRoot;
	};

	// spike-nails off
	///////////////////////////////////////////////////////////////////////////
}

#endif