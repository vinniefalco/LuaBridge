/*
 * shared_ptr.hpp - Copyright (C) 2007 by Nathan Reed
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
			return H((unsigned int)v);
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
