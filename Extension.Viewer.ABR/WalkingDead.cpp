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
#include "WalkingDead.h"

namespace BigBrotherAndy {
	_WalkingDead::_WalkingDead() {
		m_bIsValid = FALSE;
		m_pCurrent = 0;
	}

	_WalkingDead::~_WalkingDead () {
	}

	void _WalkingDead::Enum ( WCHAR* wcsFileName ) {
		m_bIsValid = FALSE;
		m_pRoot.reset();
		m_pCurrent = 0;

		if ( wcsFileName ) {
			size_t dwLenght = 0;
			// shit :/
			CAutoPtr<WCHAR> wcsPath; wcsPath.reset ( new WCHAR[MAX_PATH]);
			StringCchLength ( wcsFileName, MAX_PATH, &dwLenght );
			WCHAR* ptr = wcsFileName + dwLenght - 1; while ( *ptr != '\\' && *ptr ) { ptr--; dwLenght--;};
			StringCchCopy ( wcsPath, dwLenght, wcsFileName );

			m_pRoot.reset ( new _Item () );
			_Item* pFile = m_pRoot;
			pFile->m_Prev = 0;
			//
			_FileEnumerator Enumerator ( wcsPath, L"*.abr" );
			while ( _FileEnumerator::_Item* pItem = Enumerator.Next () ) {
				if ( ((ULONG)*pItem) & FILE_ATTRIBUTE_DIRECTORY ) {
					// TODO, tuduuu, tadaaaaa... :/
				} else {
					pFile->m_pFileName.reset ( new WCHAR[MAX_PATH] );
					wsprintf ( pFile->m_pFileName, L"%s\\%s", wcsPath, pItem->m_pFileName.get() );
					if ( !m_pCurrent && CompareString ( LOCALE_NEUTRAL, 0, pFile->m_pFileName, -1, wcsFileName, -1 ) == CSTR_EQUAL ) {
						m_pCurrent = pFile;
					};

					pFile->m_Next.reset ( new _Item );
					pFile->m_Next->m_Prev = pFile;
					pFile = pFile->m_Next;
				};
			};

			if ( pFile->m_Prev ) {
				pFile->m_Prev->m_Next.release();
			};

			if ( !m_pCurrent ) {
				m_pCurrent = m_pRoot;
			};

			m_bIsValid = TRUE;
		};
	}

	WCHAR* _WalkingDead::Next () {
		if ( m_pCurrent && m_pCurrent->m_Next ) {
			m_pCurrent = m_pCurrent->m_Next;
			return m_pCurrent->m_pFileName;
		};
		return 0;
	}

	WCHAR* _WalkingDead::Prev () {
		if ( m_pCurrent && m_pCurrent->m_Prev ) {
			m_pCurrent = m_pCurrent->m_Prev;
			return m_pCurrent->m_pFileName;
		};
		return 0;
	}

}
