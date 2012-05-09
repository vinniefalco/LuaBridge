//==============================================================================
/*
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright (C) 2007, Nathan Reed

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

#ifndef LUABRIDGE_SHARED_PTR_HEADER
#define LUABRIDGE_SHARED_PTR_HEADER

#include <stdint.h>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702) // unreachable code
#include <hash_map>
#pragma warning (pop)
#else
#include <ext/hash_map>
#endif

namespace luabridge
{

// Declaration of container for the refcounts
#ifdef _MSC_VER
typedef stdext::hash_map <const void *, int> refcounts_t;
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

/** @todo Trick the definition into existing in the header only.
*/
extern refcounts_t refcounts_;

//==============================================================================
/**
  A reference counted smart pointer.

  The api is compatible with boost::shared_ptr and std::shared_ptr, in the
  sense that it implements a strict subset of the functionality.

  This implementation uses a hash table to look up the reference count
  associated with a particular pointer.

  @tparam T The class type.

  @todo Decompose shared_ptr using a policy. At a minimum, the underlying
        reference count should be policy based (to support atomic operations)
        and the delete behavior should be policy based (to support custom
        disposal methods).

  @todo Provide an intrusive version of shared_ptr.
*/
template <typename T>
class shared_ptr
{
public:
  /** Construct as nullptr or from existing pointer to T.

      @param p The optional, existing pointer to assign from.
  */
  shared_ptr (T* p = 0) : m_p (p)
  {
    ++refcounts_ [m_p];
  }

  /** Construct from another shared_ptr.

      @param rhs The shared_ptr to assign from.
  */
  shared_ptr (shared_ptr <T> const& rhs) : m_p (rhs.get())
  {
    ++refcounts_ [m_p];
  }

  /** Construct from a shared_ptr of a different type.

      @invariant A pointer to U must be convertible to a pointer to T.

      @param  rhs The shared_ptr to assign from.
      @tparam U   The other object type.
  */
  template <typename U>
  shared_ptr (shared_ptr <U> const& rhs) : m_p (static_cast <T*> (rhs.get()))
  {
    ++refcounts_ [m_p];
  }

  /** Release the object.

      If there are no more references then the object is deleted.
  */
  ~shared_ptr ()
  {
    reset();
  }

  /** Assign from another shared_ptr.

      @param  rhs The shared_ptr to assign from.
      @return     A reference to the shared_ptr.
  */
  shared_ptr <T>& operator= (shared_ptr <T> const& rhs)
  {
    if (m_p != rhs.m_p)
    {
      reset ();
      m_p = rhs.m_p;
      ++refcounts_ [m_p];
    }
    return *this;
  }

  /** Assign from another shared_ptr of a different type.

      @invariant A pointer to U must be convertible to a pointer to T.

      @tparam U   The other object type.
      @param  rhs The other shared_ptr to assign from.
      @return     A reference to the shared_ptr.
  */
  template <typename U>
  shared_ptr <T>& operator= (shared_ptr <U> const& rhs)
  {
    reset ();
    m_p = static_cast <T*> (rhs.get());
    ++refcounts_ [m_p];
    return *this;
  }

  /** Retrieve the raw pointer.

      @return A pointer to the object.
  */
  T* get () const
  {
    return m_p;
  }

  /** Retrieve the raw pointer.

      @return A pointer to the object.
  */
  T* operator* () const
  {
    return m_p;
  }

  /** Retrieve the raw pointer.

      @return A pointer to the object.
  */
  T* operator-> () const
  {
    return m_p;
  }

  /** Determine the number of references.

      @note This is not thread-safe.

      @return The number of active references.
  */
  long use_count () const
  {
    return refcounts_ [m_p];
  }

  /** Release the pointer.

      The reference count is decremented. If the reference count reaches
      zero, the object is deleted.
  */
  void reset ()
  {
    if (m_p != 0)
    {
      if (--refcounts_ [m_p] <= 0)
        delete m_p;

      m_p = 0;
    }
  }

private:
  T* m_p;
};

}

#endif
