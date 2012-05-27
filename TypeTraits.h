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

#ifndef LUABRIDGE_TYPERAITS_HEADER
#define LUABRIDGE_TYPERAITS_HEADER

//==============================================================================
/**
  Templates for extracting type information.

  These templates are used for extracting information about types used in
  various ways
*/

/**
  Since the throw specification is part of a function signature, the FuncTraits
  family of templates needs to be specialized for both types. The THROWSPEC
  macro controls whether we use the 'throw ()' form, or 'noexcept' (if C++11
  is available) to distinguish the functions.
*/
//#define THROWSPEC throw ()

//==============================================================================
//
// TypeList
//
//==============================================================================

/**
  nil type means void parameters or return value.
*/
typedef void nil;

template <typename Head, typename Tail = nil>
struct TypeList
{
};

/**
  A TypeList with actual values.
*/
template <typename List>
struct TypeListValues
{
  static std::string const tostring (bool)
  {
    return "";
  }
};

/**
  TypeListValues recursive template definition.
*/
template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head hd_, TypeListValues <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name ();

    return s + TypeListValues <Tail>::tostring (true);
  }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head&, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head& hd_, TypeListValues <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + "&";

    return s + TypeListValues <Tail>::tostring (true);
  }
};

template <typename Head, typename Tail>
struct TypeListValues <TypeList <Head const&, Tail> >
{
  Head hd;
  TypeListValues <Tail> tl;

  TypeListValues (Head const& hd_, const TypeListValues <Tail>& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + " const&";

    return s + TypeListValues <Tail>::tostring (true);
  }
};

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
template <typename MemFn>
struct FuncTraits
{
};

/* Ordinary function pointers. */

template <typename R>
struct FuncTraits <R (*) ()>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef nil Params;
  static R call (R (*fp) (), const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return fp ();
  }
};

template <typename R, typename P1>
struct FuncTraits <R (*) (P1)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (R (*fp) (P1), const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2>
struct FuncTraits <R (*) (P1, P2)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (R (*fp) (P1, P2), const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (*) (P1, P2, P3)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (R (*fp) (P1, P2, P3), const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits <R (*) (P1, P2, P3, P4)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4),
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5),
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, 
    TypeList <P6> > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6),
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6, P7),
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R>
struct FuncTraits <R (T::*) ()>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef nil Params;
  static R call (T *obj, R (T::*fp) (), const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1>
struct FuncTraits <R (T::*) (P1)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T *obj, R (T::*fp) (P1),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2>
struct FuncTraits <R (T::*) (P1, P2)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T *obj, R (T::*fp) (P1, P2),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (T::*) (P1, P2, P3)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4>
struct FuncTraits <R (T::*) (P1, P2, P3, P4)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6> > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6, P7),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R>
struct FuncTraits <R (T::*) () const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef nil Params;
  static R call (T const* const obj, R (T::*fp) () const,
    const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1>
struct FuncTraits <R (T::*) (P1) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, R (T::*fp) (P1) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2>
struct FuncTraits <R (T::*) (P1, P2) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (T::*) (P1, P2, P3) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3, P4) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3, P4, P5) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6> > > > > > Params;
  static R call (T const* const obj,
    R (T::*fp) (P1, P2, P3, P4, P5, P6) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj,
    R (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj,
    R (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) const,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#ifdef THROWSPEC

/* Ordinary function pointers. */

template <typename R>
struct FuncTraits <R (*) () THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef nil Params;
  static R call (R (*fp) () THROWSPEC, const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return fp ();
  }
};

template <typename R, typename P1>
struct FuncTraits <R (*) (P1) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (R (*fp) (P1) THROWSPEC, const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd);
  }
};

template <typename R, typename P1, typename P2>
struct FuncTraits <R (*) (P1, P2) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (R (*fp) (P1, P2) THROWSPEC, const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (*) (P1, P2, P3) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (R (*fp) (P1, P2, P3) THROWSPEC, const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
struct FuncTraits <R (*) (P1, P2, P3, P4) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5, 
    TypeList <P6> > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6, P7) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename R, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (*) (P1, P2, P3, P4, P5, P6, P7, P8) THROWSPEC>
{
  static bool const isMemberFunction = false;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (R (*fp) (P1, P2, P3, P4, P5, P6, P7, P8) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return fp (tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <class T, typename R>
struct FuncTraits <R (T::*) () THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef nil Params;
  static R call (T *obj, R (T::*fp) () THROWSPEC, const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1>
struct FuncTraits <R (T::*) (P1) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T *obj, R (T::*fp) (P1) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2>
struct FuncTraits <R (T::*) (P1, P2) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T *obj, R (T::*fp) (P1, P2) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (T::*) (P1, P2, P3) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6> > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6, P7) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = false;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T *obj, R (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <class T, typename R>
struct FuncTraits <R (T::*) () const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef nil Params;
  static R call (T const* const obj, R (T::*fp) () const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename R, typename P1>
struct FuncTraits <R (T::*) (P1) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1> Params;
  static R call (T const* const obj, R (T::*fp) (P1) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename R, typename P1, typename P2>
struct FuncTraits <R (T::*) (P1, P2) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2> > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3>
struct FuncTraits <R (T::*) (P1, P2, P3) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3> > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4>
struct FuncTraits <R (T::*) (P1, P2, P3, P4) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4> > > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3, P4) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4,
    TypeList <P5> > > > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3, P4, P5) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6> > > > > > Params;
  static R call (T const* const obj, R (T::*fp) (P1, P2, P3, P4, P5, P6) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7> > > > > > > Params;
  static R call (T const* const obj,
    R (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename R, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FuncTraits <R (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const THROWSPEC>
{
  static bool const isMemberFunction = true;
  static bool const isConstMemberFunction = true;
  typedef T ClassType;
  typedef R ReturnType;
  typedef TypeList <P1, TypeList <P2, TypeList <P3, TypeList <P4, TypeList <P5,
    TypeList <P6, TypeList <P7, TypeList <P8> > > > > > > > Params;
  static R call (T const* const obj,
    R (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) const THROWSPEC,
    const TypeListValues<Params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

#endif

/*
* Constructor generators.  These templates allow you to call operator new and
* pass the contents of a type/value list to the Constructor.  Like the
* function pointer containers, these are only defined up to 8 parameters.
*/

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the Constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, typename List>
struct Constructor {};

template <class T>
struct Constructor <T, nil>
{
  static T* call (TypeListValues <nil> const&)
  {
    return new T;
  }
  static T* call (void* mem, TypeListValues <nil> const&)
  {
    return new (mem) T;
  }
};

template <class T, class P1>
struct Constructor <T, TypeList <P1> >
{
  static T* call (const TypeListValues<TypeList <P1> > &tvl)
  {
    return new T(tvl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1> > &tvl)
  {
    return new (mem) T(tvl.hd);
  }
};

template <class T, class P1, class P2>
struct Constructor <T, TypeList <P1, TypeList <P2> > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2> > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2> > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3> > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3> > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3> > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4> > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4> > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4> > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5> > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5> > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5> > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6> > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6> > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7> > > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7> > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7> > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7, class P8>
struct Constructor <T, TypeList <P1, TypeList <P2, TypeList <P3,
  TypeList <P4, TypeList <P5, TypeList <P6, TypeList <P7, 
  TypeList <P8> > > > > > > > >
{
  static T* call (const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7, TypeList <P8> > > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const TypeListValues<TypeList <P1, TypeList <P2,
    TypeList <P3, TypeList <P4, TypeList <P5, TypeList <P6,
    TypeList <P7, TypeList <P8> > > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

//==============================================================================

#endif
