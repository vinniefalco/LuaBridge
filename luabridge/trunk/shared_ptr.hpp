// shared_ptr.hpp
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.
// Smart pointer with reference counting

#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

template <typename T>
class shared_ptr
{
	T* ptr;
public:
	shared_ptr (T* ptr_);

	// Copy constructors: the first one is necessary to write out,
	// since the compiler doesn't recognize the second as a copy ctor
	shared_ptr (const shared_ptr<T>& rhs);
	template <typename U> shared_ptr (const shared_ptr<U>& rhs);

	template <typename U> shared_ptr& operator =
		(const shared_ptr<U> & rhs);

	T* get () const;
	T* operator * () const;
	T* operator -> () const;

	void release ();
	~shared_ptr ();
};

#include "shared_ptr_impl.hpp"

#endif
