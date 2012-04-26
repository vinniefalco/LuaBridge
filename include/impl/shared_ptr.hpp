//==============================================================================
/*
	LuaBridge is Copyright (C) 2007 by Nathan Reed.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
//==============================================================================

#include <stdint.h>

/*
 * Implementation of shared_ptr class template.
 */

// Declaration of container for the refcounts
#ifdef _MSC_VER
	typedef stdext::hash_map<const void *, int> refcounts_t;
#else
	struct ptr_hash
	{
		size_t operator () (const void * const v) const
		{
			static __gnu_cxx::hash<unsigned int> H;
			return H(uintptr_t(v));
		}
	};
	typedef __gnu_cxx::hash_map<const void *, int, ptr_hash> refcounts_t;
#endif
extern refcounts_t refcounts_;

/*
 * shared_ptr <T> implementation
 */

template <typename T>
shared_ptr<T>::shared_ptr (T* ptr_): ptr(ptr_)
{
	++refcounts_[ptr];
}

template <typename T>
shared_ptr<T>::shared_ptr (const shared_ptr<T>& rhs): ptr(rhs.get())
{
	++refcounts_[ptr];
}

template <typename T>
template <typename U>
shared_ptr<T>::shared_ptr (const shared_ptr<U>& rhs): ptr(rhs.get())
{
	++refcounts_[ptr];
}

template <typename T>
shared_ptr<T>& shared_ptr<T>::operator = (const shared_ptr<T>& rhs)
{
	reset();
	ptr = rhs.ptr;
	++refcounts_[ptr];
	return *this;
}

template <typename T>
template <typename U>
shared_ptr<T>& shared_ptr<T>::operator = (const shared_ptr<U>& rhs)
{
	reset();
	ptr = static_cast<T*>(rhs.get());
	++refcounts_[ptr];
	return *this;
}

template <typename T>
T* shared_ptr<T>::get () const
{
	return ptr;
}

template <typename T>
T* shared_ptr<T>::operator * () const
{
	return ptr;
}

template <typename T>
T* shared_ptr<T>::operator -> () const
{
	return ptr;
}

template <typename T>
long shared_ptr<T>::use_count () const
{
	return refcounts_[ptr];
}

template <typename T>
void shared_ptr<T>::reset ()
{
	if (!ptr) return;
	if (--refcounts_[ptr] <= 0)
		delete ptr;
	ptr = 0;
}

template <typename T>
shared_ptr<T>::~shared_ptr ()
{
	reset();
}
