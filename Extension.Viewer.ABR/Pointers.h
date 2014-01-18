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
#ifndef __POINTERS_H_INCLUDED_
#define __POINTERS_H_INCLUDED_

#pragma once
#pragma warning(disable: 4290)

namespace BigBrotherAndy {

	/////////////////////////////////////////////////////////////////////////////
	// Class template CPointer<T>
	// Provides common functionality for both the CAutoPtr<T> and the 
	// CAutoArrayPtr<T> class templates.
	template <typename T> class CPointer {
	public:

		// Pointer access methods
		T* get() const throw () {
			return m_pObject;
		}

		T* release() throw () {
			T* pObj = m_pObject;
			m_pObject = NULL;
			return pObj;
		}

		// Typecasting operators
		operator T*() const throw () {
			return get();
		}

		// Conditional operators
		bool operator !() const throw () {
			return m_pObject == NULL;
		}

		// Dereferencing operators
		T* operator->() const throw () {
			return get();
		}

		T& operator *() const throw () {
			return *get();
		}

	protected:
		CPointer(T* pObject) throw () : m_pObject(pObject) {
			m_pObject = pObject;
		}

		T* m_pObject;
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class template CAutoPtr<T>
	// Provides functionality of the automatic (safe) pointer to object of
	// type T similarly as the ANSI STL's auto_ptr<T> does, but with some 
	// changes pertained to operator bool() and assignment rules.
	template <typename T> class CAutoPtr: public CPointer<T> {
	public:
		explicit CAutoPtr(T* pObject = NULL) throw () : CPointer<T>(pObject) {
			m_pObject = pObject;
		}

		CAutoPtr(CAutoPtr<T>& rPointer) throw () : CPointer<T>(rPointer.release()) {
		}

		template <typename X> CAutoPtr(CAutoPtr<X>& rPointer) throw () : CPointer<T>(rPointer.release()) {
		}

		~CAutoPtr() throw () {
			reset();
		}

		// Pointer access methods
		T* reset(T* pObject = NULL) throw () {
			if (m_pObject) {
				delete m_pObject;
			};
			return m_pObject = pObject;
		}

		// Assignment operators
		CAutoPtr<T>& operator =(CAutoPtr<T>& roPointer) throw () {
			reset(roPointer.release());
			return *this;
		}

		template <typename X> CAutoPtr<T>& operator =(CAutoPtr<X>& roPointer) throw () {
			reset(roPointer.release());
			return *this;
		}
	};

	/////////////////////////////////////////////////////////////////////////////
	// Class template CAutoArrayPtr<T>
	// Provides functionality of the automatic (safe) pointer to array of
	// objects of type T that is not provided by the ANSI STL.
	template <typename T> class CAutoArrayPtr: public CPointer<T> {
	public:
		explicit CAutoArrayPtr(T* pArray = NULL) throw () : CPointer<T>(pArray) {
			m_pObject = pArray;
		}

		CAutoArrayPtr(CAutoArrayPtr<T>& rPointer) throw () : CPointer<T>(rPointer.release()) {
		}

		template <typename X> CAutoArrayPtr(CAutoArrayPtr<X>& rPointer) throw () : CPointer<T>(rPointer.release()) {
			// Typically this will fail. This constructor is used to prevent
			// unwanted conversion to T* followed by CAutoArrayPtr(T*) usage.
		}

		~CAutoArrayPtr() throw () {
			reset();
		}

		// Pointer access methods
		virtual void reset(T* pArray = NULL) throw () {
			if (m_pObject) {
				delete [] m_pObject;
			};
			m_pObject = pArray;
		}

		// Assignment operators
		CAutoArrayPtr<T>& operator =(CAutoArrayPtr<T>& roPointer) throw () {
			reset(roPointer.release());
			return *this;
		}

		template <typename X> CAutoArrayPtr<T>& operator =(CAutoArrayPtr<X>& roPointer) throw () {
			// Typically this will fail. This operator is used to prevent
			// unwanted conversion to T* followed by CAutoArrayPtr(T*) usage.
			reset(roPointer.release());
			return *this;
		}
	};

}; // namespace BigBrotherAndy

#pragma warning(default: 4290)

#endif  // ! __POINTERS_H_INCLUDED_
