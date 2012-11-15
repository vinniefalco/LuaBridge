//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>

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
*/
//==============================================================================

#ifndef LUABRIDGE_FUNCTRAITS_HEADER
#define LUABRIDGE_FUNCTRAITS_HEADER

/**
  Since the throw specification is part of a function signature, the FuncTraits
  family of templates needs to be specialized for both types. The
  LUABRIDGE_THROWSPEC macro controls whether we use the 'throw ()' form, or
  'noexcept' (if C++11 is available) to distinguish the functions.
*/
#if defined (__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__clang__) || defined(__GNUC__) || \
    (defined (_MSC_VER) && (_MSC_VER >= 1700))
// Do not define LUABRIDGE_THROWSPEC since the Xcode and gcc  compilers do not
// distinguish the throw specification in the function signature.
#else
#define LUABRIDGE_THROWSPEC throw()
#endif

//==============================================================================
/**
  Traits for function pointers.

  There are three types of functions: global, non-const member, and const member.
  These templates determine the type of function, which class type it belongs to
  if it is a class member, the const-ness if it is a member function, and the
  type information for the return value and argument list.

  Expansions are provided for functions with up to 8 parameters. This can be
  manually extended, or expanded to an arbitrary amount using C++11 features.
*/
template <typename MemFn, typename D = MemFn>
struct FuncTraits
{
};

/* Ordinary function pointers. */

template <typename R, typename D>
struct FuncTraits <R (*) (), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef None Params;
  static R call (DeclType fp, TypeListValues <Params>)
  {
    return fp ();
  }
};

template <typename R, typename P1, typename D>
struct FuncTraits <R (*) (P1), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (*) (P1, P2), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (*) (P1, P2, P3), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,  TypeList <P6> > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8), D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) (), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8), D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2) const,
    TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#if defined (LUABRIDGE_THROWSPEC)

/* Ordinary function pointers. */

template <typename R, typename D>
struct FuncTraits <R (*) () LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef None Params;
  static R call (DeclType fp, TypeListValues <Params> const&)
  {
    return fp ();
  }
};

template <typename R, typename P1, typename D>
struct FuncTraits <R (*) (P1) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (*) (P1, P2) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (*) (P1, P2, P3) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,  TypeList <P6> > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = false;
  typedef D DeclType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (DeclType fp, TypeListValues <Params> tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R, typename D>
struct FuncTraits <R (T::*) () const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef None Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> const&)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1, typename D>
struct FuncTraits <R (T::*) (P1) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename D>
struct FuncTraits <R (T::*) (P1, P2) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5> > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8, typename D>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const LUABRIDGE_THROWSPEC, D>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef D DeclType;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj, DeclType fp, TypeListValues <Params> tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#endif

#endif
