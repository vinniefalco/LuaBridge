//==============================================================================
/*
  https://github.com/vinniefalco/LuaBridge
  https://github.com/vinniefalco/LuaBridgeDemo
  
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright (C) 2007, Nathan Reed

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

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

  This file incorporates work covered by the following copyright and
  permission notice:  

    The Loki Library
    Copyright (c) 2001 by Andrei Alexandrescu
    This code accompanies the book:
    Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
        Patterns Applied". Copyright (c) 2001. Addison-Wesley.
    Permission to use, copy, modify, distribute and sell this software for any 
        purpose is hereby granted without fee, provided that the above copyright 
        notice appear in all copies and that both that copyright notice and this 
        permission notice appear in supporting documentation.
    The author or Addison-Welsey Longman make no representations about the 
        suitability of this software for any purpose. It is provided "as is" 
        without express or implied warranty.
*/
//==============================================================================

#ifndef LUABRIDGE_SHARED_PTR_HEADER
#define LUABRIDGE_SHARED_PTR_HEADER

#ifdef _MSC_VER
# include <hash_map>
#else
# include <ext/hash_map>
#endif

namespace luabridge
{

//==============================================================================
/**
  Support for our shared_ptr.
*/
struct shared_ptr_base
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

protected:
  inline refcounts_t& refcounts_ ()
  {
    static refcounts_t refcounts;
    return refcounts ;
  }
};

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
template <class T>
class shared_ptr : private shared_ptr_base
{
public:
  template <typename Other>
  struct rebind
  {
    typedef shared_ptr <Other> other;
  };

  /** Construct as nullptr or from existing pointer to T.

      @param p The optional, existing pointer to assign from.
  */
  shared_ptr (T* p = 0) : m_p (p)
  {
    ++refcounts_ () [m_p];
  }

  /** Construct from another shared_ptr.

      @param rhs The shared_ptr to assign from.
  */
  shared_ptr (shared_ptr <T> const& rhs) : m_p (rhs.get())
  {
    ++refcounts_ () [m_p];
  }

  /** Construct from a shared_ptr of a different type.

      @invariant A pointer to U must be convertible to a pointer to T.

      @param  rhs The shared_ptr to assign from.
      @tparam U   The other object type.
  */
  template <typename U>
  shared_ptr (shared_ptr <U> const& rhs) : m_p (static_cast <T*> (rhs.get()))
  {
    ++refcounts_ () [m_p];
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
      ++refcounts_ () [m_p];
    }
    return *this;
  }

  /** Assign from another shared_ptr of a different type.

      @note A pointer to U must be convertible to a pointer to T.

      @tparam U   The other object type.
      @param  rhs The other shared_ptr to assign from.
      @return     A reference to the shared_ptr.
  */
  template <typename U>
  shared_ptr <T>& operator= (shared_ptr <U> const& rhs)
  {
    reset ();
    m_p = static_cast <T*> (rhs.get());
    ++refcounts_ () [m_p];
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
    return refcounts_ () [m_p];
  }

  /** Release the pointer.

      The reference count is decremented. If the reference count reaches
      zero, the object is deleted.
  */
  void reset ()
  {
    if (m_p != 0)
    {
      if (--refcounts_ () [m_p] <= 0)
        delete m_p;

      m_p = 0;
    }
  }

private:
  T* m_p;
};

//==============================================================================

template <class T>
struct ContainerInfo <luabridge::shared_ptr <T> >
{
  typedef typename T Type;

  template <class U>
  static inline void* get (luabridge::shared_ptr <U>& u)
  {
    T* p = u.get ();
    return p;
  }
};

//------------------------------------------------------------------------------

template <class T>
struct ContainerInfo <luabridge::shared_ptr <T const> >
{
  typedef typename T Type;

  template <class U>
  static inline void* get (luabridge::shared_ptr <U const>& u)
  {
    T const* p = u.get ();
    return const_cast <T*> (p);
  }
};

//------------------------------------------------------------------------------

template <class T>
struct Stack <luabridge::shared_ptr <T> > : Detail
{
  static inline void push (lua_State* L, luabridge::shared_ptr <T> const& p)
  {
    new (UserdataType <luabridge::shared_ptr <T> >::push (L, false))
      UserdataType <luabridge::shared_ptr <T> > (p);
  }

  template <class U>
  static inline void push (lua_State* L, luabridge::shared_ptr <U> const& p)
  {
    new (UserdataType <luabridge::shared_ptr <T> >::push (L, false))
      UserdataType <luabridge::shared_ptr <T> > (p);
  }

  static inline luabridge::shared_ptr <T> get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, false);
  }
};

//------------------------------------------------------------------------------

template <class T>
struct Stack <luabridge::shared_ptr <T const> > : Detail
{
  static inline void push (lua_State* L, luabridge::shared_ptr <T const> const& p)
  {
    new (UserdataType <luabridge::shared_ptr <T const> >::push (L, true))
      UserdataType <luabridge::shared_ptr <T const> > (p);
  }

  template <class U>
  static inline void push (lua_State* L, luabridge::shared_ptr <U const> const& p)
  {
    new (UserdataType <luabridge::shared_ptr <T const> >::push (L, true))
      UserdataType <luabridge::shared_ptr <T const> > (p);
  }

  static inline luabridge::shared_ptr <T const> get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index, true);
  }
};

}

#endif
