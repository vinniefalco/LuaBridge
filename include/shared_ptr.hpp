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

#ifndef LUABRIDGE_SHARED_PTR_HEADER
#define LUABRIDGE_SHARED_PTR_HEADER

#include <stdint.h>

#ifdef _MSC_VER
#  pragma warning (push)
#  pragma warning (disable: 4702) // unreachable code
#  include <hash_map>
#  pragma warning (pop)
#else
#  include <ext/hash_map>
#endif

namespace luabridge
{

/** A reference counted smart pointer.

    The api is compatible with boost::shared_ptr and std::shared_ptr, in the
    sense that it implements a strict subset of the functionality.

    @tparam T The class type.
*/
template <typename T>
class shared_ptr
{
public:
  shared_ptr (T* ptr_ = 0);
  // Copy constructors: the first one is necessary to write out,
  // since the compiler doesn't recognize the second as a copy ctor
  shared_ptr (const shared_ptr<T>& rhs);
  template <typename U> shared_ptr (const shared_ptr<U>& rhs);
  ~shared_ptr ();

  // Assignment operators: same deal, compiler doesn't recognize the
  // second as an assign op and generates its own if we don't write it
  shared_ptr<T>& operator = (const shared_ptr<T> & rhs);
  template <typename U> shared_ptr& operator =
    (const shared_ptr<U> & rhs);

  T* get () const;
  T* operator * () const;
  T* operator -> () const;
  long use_count () const;

  void reset ();

private:
  T* ptr;
};

#	include "impl/shared_ptr.hpp"
}

#endif
