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

#ifndef LUABRIDGE_LUABRIDGE_HEADER
#define LUABRIDGE_LUABRIDGE_HEADER

#include <typeinfo>
#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
# include <hash_map>
#else
# include <ext/hash_map>
#endif

//==============================================================================
/**
  @mainpage LuaBridge: Simple C++ to Lua bindings.

  @details

  # LuaBridge

  LuaBridge is a lightweight, dependency-free library for binding Lua to C++ which
  works with Lua revisions starting from 5.1.2.

  ## Compiling

  LuaBridge compiles correctly in g++ 4, MSVC 7.1, and MSVC 8.0.  Because of the
  advanced template features it uses, I can't guarantee LuaBridge will compile
  correctly with anything else, but it is written in standard-compliant C++, so
  if you have a compliant compiler you *should* be fine.

  Lua headers need to be included before including luabridge.h.

  Compiling should be very simple.  Ensure that Lua is installed and its headers
  are in your include path.  If you are using MSVC, load the provided solution
  file and click build.  Otherwise, a Makefile is provided; just enter the
  directory where you have decompressed LuaBridge and type `make'.

  One option you may have to set manually is the name of the Lua library.  On
  some systems it is called 'lua' (liblua.a, liblua.so) and on others 'lua5.1'
  (liblua5.1.a, liblua5.1.so).  Setting the make variable LUA_NAME lets you
  specify this.  The default is 'lua'.

  The distributed LuaBridge source includes a test application, which serves to
  verify that everything works as expected, and also demonstrates the code
  necessary to use each of LuaBridge's features.  You can run it by clicking
  Execute (Ctrl+F5) in MSVC after building, or by typing `make test` at the
  command line.  (In either case the command should be executed from the
  directory in which LuaBridge was decompressed.)  The application will print
  'All tests succeeded' if everything worked as expected, or an error message
  otherwise.

  ## Usage

  LuaBridge is based on C++ template metaprogramming.  It contains template code
  to automatically generate at compile-time the various Lua API calls necessary
  to export your program's classes and functions to the Lua environment.

  You will need to ensure that LuaBridge's include directory is in your include
  path.  The only file that needs to be included is LuaBridge.hpp; it will
  include all the implementation files.  You will also need to link with
  libLuaBridge (release version) or libLuaBridged (debug version), which will be
  made in the lib directory.  These are static libraries containing the small
  amount of common binary code in LuaBridge.

  ### Registering functions

  If L is a pointer to an instance of lua_State, the following code creates a
  LuaBridge scope for registering C++ functions and classes to L:

     LuaBridge::scope s(L);

  Functions can then be registered as follows:

      s  .function("foo", &foo)
        .function("bar", &bar);

  The 'function' function returns a reference to s, so you can chain many
  function definitions together.  The first argument is the name by which the
  function will be available in Lua, and the second is the function's address.
  LuaBridge will automatically detect the number (up to 8, by default) and type
  of the parameters.  Functions registered this way will be available at the
  global scope to Lua scripts executed by L.  Overloading of function names is
  not supported, nor is it likely to be supported in the future.

  ### Registering data

  Variables can also be registered.  You can expose a 'bare' variable to Lua, or
  wrap it in getter and setter functions:

      s  .variable_rw("var1", &var1)
        .variable_rw("var2", &getter2, &setter2)
        .variable_ro("var3", &var3)
        .variable_ro("var4", &getter4)

  The first registration above gives Lua direct access to the 'var' variable.
  The second creates a variable which appears like any other variable to Lua
  code, but is retrieved and set through the 'getter2' and 'setter2' function.
  The getter must take no parameters and return a value, and the setter must take
  a value of the same type and return nothing.  The 'variable_rw' function
  creates a readable and writeable variable, while 'variable_ro' creates a
  read-only one.  Obviously, there is no setter for the read-only variable.

  Supported types for variables, and function parameters and returns, are:
   * Primitive types:
        int, unsigned int, unsigned char, short, unsigned short, long,
        unsigned long, float, double
        (all converted to Lua_number, since Lua does not distinguish
        different number types),
        char (converted to strings of length one)
        bool
   * Strings: const char * and std::string
   * Objects: pointers, references, and shared_ptrs to objects of registered
       classes (more about shared_ptrs later)

  ### Registering classes

  C++ classes can be registered with Lua as follows:

    s.class_<MyClass>("MyClass")
      .constructor<void (*) (// parameter types )>()
      .method("method1", &MyClass::method1)
      .method("method2", &MyClass::method2);

    s.subclass<MySubclass, MyBaseClass>("MySubclass")
      .constructor<...>
      ...

  The "class_" function registers a class; its constructor will be available as
  a global function with name given as argument to class_.  The object returned
  can then be used to register the constructor (no overloading is supported, so
  there can only be one constructor) and methods.

  LuaBridge cannot automatically determine the number and types of constructor
  parameters like it can for functions and methods, so you must provide them.
  This is done by letting the 'constructor' function take a template parameter,
  which must be a function pointer type.  The parameter types will be extracted
  from this (the return type is ignored).  For example, to register a
  constructor taking two parameters, one int and one const char *, you would
  write:

    s.class_...
      .constructor<void (*) (int, const char *)>()

  Method registration works just like function registration.  Virtual methods
  work normally; no special syntax is needed.  Const methods are detected and
  const-correctness is enforced, so if a function returns a const object (or
  rather, a shared_ptr to a const object) to Lua, that reference to the object
  will be considered const and only const methods will be called on it.
  Destructors are registered automatically for each class.

  Static methods may be registered using the 'static_method' function, which is
  simply an alias for the 'function' function:

    s.class_...
      .static_method("method3", &MyClass::method3)

  LuaBridge also supports properties, which allow class data members to be read
  and written from Lua as if they were variables.  Properties work much like
  variables do, and the syntax for registering them is as follows:

    s.class_...
      .property_rw("property1", &MyClass::property1)
      .property_rw("property2", &MyClass::getter2, &MyClass::setter2)
      .property_ro("property3", &MyClass::property3)
      .property_ro("property4", &MyClass::getter4)

  Static properties on classes are also supported, using 'static_property_rw'
  and 'static_property_ro', which are simply aliases for 'variable_rw' and
  'variable_ro'.

  To register a subclass of another class that has been registered with Lua, use
  the "subclass" function.  This takes two template arguments: the class to be
  registered, and its base class.  Inherited methods do not have to be
  re-declared and will function normally in Lua.  If a class has a base class
  that is *not* registered with Lua, there is no need to declare it as a
  subclass.

  ### Lifetime Management

  LuaBridge uses a built-in reference counted smart pointer implementation 
  called shared_ptr for memory management.  It is necessary that all objects
  that are created by Lua or exposed to Lua are referred to using shared_ptr
  in C++, since C++ code may not be able to predict how long a Lua reference
  to an object may last.  shared_ptr is declared in the LuaBridge namespace and
  implements a strict subset of boost::shared_ptr's functionality.  If desired,
  LuaBridge will use another shared_ptr implementation rather than its own;
  simply #define USE_OTHER_SHARED_PTR before including LuaBridge.hpp to enable
  this.  shared_ptr must be visible to the LuaBridge namespace, i.e. you will
  need to write

    using boost::shared_ptr;

  or

    namespace luabridge
          {
    using boost::shared_ptr;
    }

  to make the other shared_ptr visible to LuaBridge.

  ## Limitations 

  LuaBridge does not support:

  - More than 8 parameters on a function or method (although this can be
   increased by editing include/impl/typelist.hpp)

  - Returning objects from functions other than through a shared_ptr

  - Passing objects to functions by value

  - Overloaded functions, methods, or constructors

  - Global variables (variables must be wrapped in a named scope)

  - Automatic conversion between STL container types and Lua tables

  - Inheriting Lua classes from C++ classes

  ## Development

  Github is the new official home for LuaBridge. The old SVN repository is
  deprecated since it is no longer used, or maintained. The original author has
  graciously passed the reins to Vinnie Falco for maintaining and improving the
  project.

  We welcome contributions to LuaBridge. Feel free to fork the repository and
  issue a Pull Request. All questions, comments, suggestions, and/or proposed
  changes will be handled by the new maintainer.

  The 'master' branch contains only stable commits belonging to tagged releases,
  while the 'develop' branch contains proposed features for inclusion in the
  next release.

  ## License

  The current version of LuaBridge is distributed under the terms of the MIT
  License. Older versions up to and including 0.2 are distributed under the
  BSD 3-Clause License. See the corresponding license file in those versions
  for more details.
*/

#include <cassert>
#include <string>

//#ifndef USE_OTHER_SHARED_PTR
//#include "shared_ptr.h"
//#endif

namespace luabridge
{

// forward declaration
template <class T>
class class__;

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
template <typename T>
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
/**
  Holds the address of a unique string to identify unregistered classes.
*/
class classnamebase
{
protected:
  static inline char const* unregisteredClassName ()
  {
    static char const* name = "(unknown type)";
    return name;
  }
};

//------------------------------------------------------------------------------
/**
  Registered class attributes.

  This template provides introspection to retrieve the attributes associated
  with any class type. Attributes include whether or not the class is
  registered, a name string for registered classes, the const-ness, and
  the Policy object.

  @tparam T The class for obtaining attributes.
*/
template <class T>
class classname : private classnamebase
{
public:
  /** Register a class.
  */
  static void registerClass (char const* name)
  {
    assert (!isRegistered ());

    classname <T>::s_string = std::string ("const ") + std::string (name);
    classname <T>::s_constname = classname <T>::s_string.c_str ();
    classname <T>::s_name = classname <T>::s_constname + 6;
  }

  /** Determine if the class is registered to Lua.
  */
  static inline bool isRegistered ()
  {
    return classname <T>::s_name != unregisteredClassName ();
  }

  /** Retrieve the class name.

      @note The class must be registered.
  */
  static inline char const* name ()
  {
    assert (isRegistered ());
    return classname <T>::s_name;
  }

  /** Retrieve the class const name.

      @note The class must be registered.
  */
  static inline char const* const_name ()
  {
    assert (isRegistered ());
    return classname <T>::s_constname;
  }

  /** Determine if a registered class is const.

      @note Unregistered classes are not const.

      @todo Should we require that the class is registered?
  */
  static inline bool isConst ()
  {
    //assert (isRegistered ());
    return false;
  }

private:
  static char const* s_constname;
  static char const* s_name;
  static std::string s_string;
};

template <class T>
std::string classname <T>::s_string;

template <class T>
char const* classname <T>::s_constname = classnamebase::unregisteredClassName ();

template <class T>
char const* classname <T>::s_name = classnamebase::unregisteredClassName ();

//------------------------------------------------------------------------------
/**
  Container specialization for const types.

  The mapped name is the same.
*/
template <typename T>
struct classname <const T> : public classname <T>
{
  static inline bool isConst ()
  {
    return true;
  }
};

//==============================================================================
//
// typelist
//
//==============================================================================
/*
  Implementation of C++ template type lists and related tools.
  Type list and definition of nil type list, which is void.
*/

typedef void nil;

template <typename Head, typename Tail = nil>
struct typelist {};

/*
* Type/value list.
*/

template <typename Typelist>
struct typevallist {};

template <typename Head, typename Tail>
struct typevallist <typelist<Head, Tail> >
{
  Head hd;
  typevallist<Tail> tl;
  typevallist(Head hd_, const typevallist<Tail> &tl_):
  hd(hd_), tl(tl_)
  {}
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct typevallist <typelist<Head &, Tail> >
{
  Head hd;
  typevallist<Tail> tl;
  typevallist(Head &hd_, const typevallist<Tail> &tl_):
  hd(hd_), tl(tl_)
  {}
};

template <typename Head, typename Tail>
struct typevallist <typelist<const Head &, Tail> >
{
  Head hd;
  typevallist<Tail> tl;
  typevallist(const Head &hd_, const typevallist<Tail> &tl_):
  hd(hd_), tl(tl_)
  {}
};

/*
* Containers for function pointer types.  We have three kinds of containers:
* one for global functions, one for non-const member functions, and one for
* const member functions.  These containers allow the function pointer types
* to be broken down into their components.
*
* Of course, because of the limitations of C++ templates, we can only define
* this for up to a constant number of function parameters.  We give the
* definitions for up to 8 parameters here, though this can be easily
* increased if necessary.
*/

template <typename FnPtr>
struct fnptr {};

/* Ordinary function pointers. */

template <typename Ret>
struct fnptr <Ret (*) ()>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (Ret (*fp) (), const typevallist<params> &tvl)
  {
    (void)tvl;
    return fp();
  }
};

template <typename Ret, typename P1>
struct fnptr <Ret (*) (P1)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (Ret (*fp) (P1), const typevallist<params> &tvl)
  {
    return fp(tvl.hd);
  }
};

template <typename Ret, typename P1, typename P2>
struct fnptr <Ret (*) (P1, P2)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (Ret (*fp) (P1, P2), const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (*) (P1, P2, P3)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (Ret (*fp) (P1, P2, P3), const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4>
struct fnptr <Ret (*) (P1, P2, P3, P4)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5, 
    typelist<P6> > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename Ret, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static const bool mfp = false;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
  static Ret call (Ret (*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const typevallist<params> &tvl)
  {
    return fp(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Non-const member function pointers. */

template <typename T, typename Ret>
struct fnptr <Ret (T::*) ()>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (T *obj, Ret (T::*fp) (), const typevallist<params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (T *obj, Ret (T::*fp) (P1),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6> > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8)>
{
  static const bool mfp = true;
  static const bool const_mfp = false;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist <P8> > > > > > > > params;
  static Ret call (T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8),
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/* Const member function pointers. */

template <typename T, typename Ret>
struct fnptr <Ret (T::*) () const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (const T *obj, Ret (T::*fp) () const,
    const typevallist<params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <typename T, typename Ret, typename P1>
struct fnptr <Ret (T::*) (P1) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (const T *obj, Ret (T::*fp) (P1) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2>
struct fnptr <Ret (T::*) (P1, P2) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (const T *obj, Ret (T::*fp) (P1, P2) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3>
struct fnptr <Ret (T::*) (P1, P2, P3) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (const T *obj, Ret (T::*fp) (P1, P2, P3) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct fnptr <Ret (T::*) (P1, P2, P3, P4) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (const T *obj, Ret (T::*fp) (P1, P2, P3, P4) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (const T *obj, Ret (T::*fp) (P1, P2, P3, P4, P5) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6> > > > > > params;
  static Ret call (const T *obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (const T *obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct fnptr <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
  static Ret call (const T *obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7, P8) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

/*
* Constructor generators.  These templates allow you to call operator new and
* pass the contents of a type/value list to the constructor.  Like the
* function pointer containers, these are only defined up to 8 parameters.
*/

template <typename T, typename Typelist>
struct constructor {};

template <typename T>
struct constructor <T, nil>
{
  static T* call (const typevallist<nil> &tvl)
  {
    (void)tvl;
    return new T;
  }
  static T* call (void* mem, const typevallist<nil> &tvl)
  {
    (void)tvl;
    return new (mem) T;
  }
};

template <typename T, typename P1>
struct constructor <T, typelist<P1> >
{
  static T* call (const typevallist<typelist<P1> > &tvl)
  {
    return new T(tvl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1> > &tvl)
  {
    return new (mem) T(tvl.hd);
  }
};

template <typename T, typename P1, typename P2>
struct constructor <T, typelist<P1, typelist<P2> > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2> > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2> > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3> > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3> > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3> > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3, typename P4>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4> > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4> > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4> > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
  typename P5>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5> > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5> > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5> > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6> > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6> > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6> > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6, typelist<P7> > > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7> > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7> > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <typename T, typename P1, typename P2, typename P3, typename P4,
  typename P5, typename P6, typename P7, typename P8>
struct constructor <T, typelist<P1, typelist<P2, typelist<P3,
  typelist<P4, typelist<P5, typelist<P6, typelist<P7, 
  typelist<P8> > > > > > > > >
{
  static T* call (const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7, typelist<P8> > > > > > > > > &tvl)
  {
    return new T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
  static T* call (void* mem, const typevallist<typelist<P1, typelist<P2,
    typelist<P3, typelist<P4, typelist<P5, typelist<P6,
    typelist<P7, typelist<P8> > > > > > > > > &tvl)
  {
    return new (mem) T(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.tl.tl.hd);
  }
};

//==============================================================================
/*
  Utilities.

  Some are provided as static class members so the definitions may be placed
  in the header rather than a source file. Only one instance of the definition
  will be linked in even though the header is included in multiple translation
  units.
*/

//------------------------------------------------------------------------------
/**
  Get a value, bypassing metamethods.
*/  
inline void rawgetfield (lua_State* const L, int const index, char const* const key)
{
  lua_pushstring (L, key);
  if (index < 0)
    lua_rawget (L, index-1);
  else
    lua_rawget (L, index);
}

//------------------------------------------------------------------------------
/**
  Set a value, bypassing metamethods.
*/  
inline void rawsetfield (lua_State* const L, int const index, char const* const key)
{
  lua_pushstring (L, key);
  lua_insert (L, -2);
  if (index < 0)
    lua_rawset (L, index-1);
  else
    lua_rawset (L, index);
}

struct detail // namespace detail
{

//------------------------------------------------------------------------------
/**
  Produce an error message.

  This is our version of luaL_typerror, which was removed in Lua 5.2.
*/
static int typeError (lua_State* L, int narg, const char *tname)
{
  const char *msg = lua_pushfstring (L, "%s expected, got %s",
    tname, luaL_typename (L, narg));

  return luaL_argerror (L, narg, msg);
}

//------------------------------------------------------------------------------
/**
  Custom __index metamethod for C++ classes.

  If the given key is not found, the search will be delegated up the parent
  hierarchy.
*/
static int indexer (lua_State* L)
{
  int result = 0;

  lua_getmetatable (L, 1);

  for (;;)
  {
    // Check the metatable.
    lua_pushvalue (L, 2);
    lua_rawget (L, -2);
    if (!lua_isnil (L, -1))
    {
      // found
      result = 1;
      break;
    }
    lua_pop(L, 1);

    // Check the __propget metafield.
    rawgetfield (L, -1, "__propget");
    if (!lua_isnil (L, -1))
    {
      lua_pushvalue (L, 2);
      lua_rawget (L, -2);
      if (!lua_isnil (L, -1))
      {
        // found
        assert (lua_isfunction (L, -1));
        lua_pushvalue (L, 1);
        lua_call (L, 1, 1);
        result = 1;
        break;
      }
      lua_pop (L, 1);
    }
    lua_pop (L, 1);

    // Check the __const metafield.
    rawgetfield (L, -1, "__const");
    if (!lua_isnil (L, -1))
    {
      lua_pushvalue (L, 2);
      lua_rawget (L, -2);
      if (!lua_isnil (L, -1))
      {
        // found
        result = 1;
        break;
      }
      lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Repeat the lookup in the __parent metafield,
    // or return nil if the field doesn't exist.
    rawgetfield (L, -1, "__parent");
    if (lua_isnil(L, -1))
    {
      // no parent
      result = 1;
      break;
    }
    lua_remove(L, -2);
  }

  return result;
}

//------------------------------------------------------------------------------
/**
  Custom __newindex metamethod for static tables.

  This supports properties on scopes, and static properties of classes.
*/
static int newindexer (lua_State* L)
{
  int result = 0;

  lua_getmetatable (L, 1);

  for (;;)
  {
    // Check the __propset metafield.
    rawgetfield (L, -1, "__propset");
    if (!lua_isnil (L, -1))
    {
      lua_pushvalue (L, 2);
      lua_rawget (L, -2);
      if (!lua_isnil (L, -1))
      {
        // found
        assert (lua_isfunction (L, -1));
        lua_pushvalue (L, 3);
        lua_call (L, 1, 0);
        result = 0;
        break;
      }
      lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Repeat the lookup in the __parent metafield.
    rawgetfield (L, -1, "__parent");
    if (lua_isnil (L, -1))
    {
      // Either the property or __parent must exist.
      result = luaL_error (L,
        "attempt to set %s, which isn't a property", lua_tostring(L, 2));
      break;
    }
    lua_remove(L, -2);
  }

  return result;
}

//------------------------------------------------------------------------------
/**
  Custom __newindex metamethod for metatables.

  This supports properties on class objects. The corresponding object is
  passed in the first parameter to the setFunction.
*/
static int object_newindexer (lua_State* L)
{
  int result = 0;

  lua_getmetatable (L, 1);

  for (;;)
  {
    // Check __propset
    rawgetfield (L, -1, "__propset");
    if (!lua_isnil (L, -1))
    {
      lua_pushvalue (L, 2);
      lua_rawget (L, -2);
      if (!lua_isnil (L, -1))
      {
        // found it, call the setFunction.
        assert (lua_isfunction (L, -1));
        lua_pushvalue (L, 1);
        lua_pushvalue (L, 3);
        lua_call (L, 2, 0);
        result = 0;
        break;
      }
      lua_pop (L, 1);
    }
    lua_pop (L, 1);

    // Repeat the lookup in the __parent metafield.
    rawgetfield (L, -1, "__parent");
    if (lua_isnil (L, -1))
    {
      // Either the property or __parent must exist.
      result = luaL_error (L,
        "attempt to set %s, which isn't a property", lua_tostring (L, 2));
    }
    lua_remove (L, -2);
  }

  return result;
}

//------------------------------------------------------------------------------
/**
  Create a static table.

  The resulting table is placed on the stack.
*/
static void createStaticTable (lua_State* L)
{
  lua_newtable (L);                         // Create the table.
  lua_pushvalue (L, -1);
  lua_setmetatable (L, -2);                 // Set it as its own metatable.
  lua_pushcfunction (L, &indexer);
  rawsetfield (L, -2, "__index");           // Use our __index.
  lua_pushcfunction (L, &newindexer);
  rawsetfield (L, -2, "__newindex");        // Use our __newindex.
  lua_newtable (L);
  rawsetfield (L, -2, "__propget");         // Create empty __propget.
  lua_newtable (L);
  rawsetfield (L, -2, "__propset");         // Create empty __propset.
}

//------------------------------------------------------------------------------
/**
  Create static tables from a dot-separated identifier.

  "x.y.z" produces _G["x"] = x[], x["y"] = y[], and y["z"] = z[].
    
  The last table (z[] in the example) is left on the stack.
*/
static void createStaticTables (lua_State* L, std::string name)
{
  assert (name.length () > 0);

  lua_getglobal (L, "_G");

  // Process each dot-separated namespace identifier.
  size_t start = 0;
  size_t pos = 0;
  while ((pos = name.find ('.', start)) != std::string::npos)
  {
    std::string const id = name.substr (start, pos - start);
    lua_getfield (L, -1, id.c_str ()); //! @todo Do we need rawgetfield() here?
    if (lua_isnil (L, -1))
    {
      lua_pop (L, 1);
      createStaticTable (L);
      lua_pushvalue (L, -1);
      rawsetfield (L, -3, id.c_str ());
    }
    lua_remove(L, -2);
    start = pos + 1;
  }

  // Create a new table with the remaining portion of the name.
  createStaticTable (L);
  rawsetfield (L, -2, name.c_str() + start);
  lua_pop (L, 1);
}

//------------------------------------------------------------------------------
/**
  Look up a static table.

  The table is identified by its fully qualified dot-separated name. The
  resulting table is returned on the stack.

  @note The table must exist.
*/
static void findStaticTable (lua_State* const L, char const* const name)
{
  lua_getglobal (L, "_G");

  if (name && name [0] != '\0')
  {
    std::string namestr (name);
    size_t start = 0;
    size_t pos = 0;
    while ((pos = namestr.find ('.', start)) != std::string::npos)
    {
      lua_getfield (L, -1, namestr.substr(start, pos - start).c_str());
      assert (!lua_isnil(L, -1));
      lua_remove (L, -2);
      start = pos + 1;
    }
    lua_getfield (L, -1, namestr.substr(start).c_str());
    assert (!lua_isnil(L, -1));
    lua_remove (L, -2);
  }
}

//------------------------------------------------------------------------------
/*
* Class type checker.  Given the index of a userdata on the stack, makes
* sure that it's an object of the given classname or a subclass thereof.
* If yes, returns the address of the data; otherwise, throws an error.
* Works like the luaL_checkudata function.
*/

static void* checkClass (lua_State* L, int index, const char *tname, bool exact)
{
  void* p = 0;

  // If index is relative to the top of the stack, convert it into an index
  // relative to the bottom of the stack, so we can push our own stuff
  if (index < 0)
    index += lua_gettop(L) + 1;

  // Check that the thing on the stack is indeed a userdata
  if (!lua_isuserdata(L, index))
    typeError (L, index, tname);

  // Lookup the given name in the registry
  luaL_getmetatable(L, tname);

  // Lookup the metatable of the given userdata
  lua_getmetatable(L, index);

  // If exact match required, simply test for identity.
  if (exact)
  {
    // Ignore "const" for exact tests (which are used for destructors).
    if (!strncmp (tname, "const ", 6))
      tname += 6;

    if (lua_rawequal(L, -1, -2))
    {
      p = lua_touserdata(L, index);
    }
    else
    {
      // Generate an informative error message
      rawgetfield (L, -1, "__type");
      luaL_argerror (L, index, lua_pushfstring (L,
        "%s expected, got %s", tname , lua_typename (L, lua_type (L, index))));
      // doesn't get here
    }
  }

  if (!p)
  {
    // Navigate up the chain of parents if necessary
    while (!lua_rawequal (L, -1, -2))
    {
      // Check for equality to the const metatable
      rawgetfield(L, -1, "__const");
      if (!lua_isnil(L, -1))
      {
        if (lua_rawequal(L, -1, -3))
          break;
      }
      lua_pop(L, 1);

      // Look for the metatable's parent field
      rawgetfield(L, -1, "__parent");

      // No parent field?  We've failed; generate appropriate error
      if (lua_isnil(L, -1))
      {
        // Lookup the __type field of the original metatable, so we can
        // generate an informative error message
        lua_getmetatable(L, index);
        rawgetfield(L, -1, "__type");
        luaL_argerror (L, index, lua_pushfstring (L,
          "%s expected, got %s", tname , lua_tostring (L, -1)));
        break; // doesn't get here
      }

      // Remove the old metatable from the stack
      lua_remove(L, -2);
    }

    // Found a matching metatable; return the userdata
    p = lua_touserdata(L, index);
  }

  return p;
}

};

//==============================================================================
/**
  Class wrapped in a Lua userdata.
*/
class Userdata
{
public:
  //----------------------------------------------------------------------------
  /**
    lua_CFunction to garbage collect a class object.

    This is used for the __gc metamethod.

    @note The expected class name is passed as an upvalue so that we can
          ensure that we are destroying the right kind of object.
  */
  template <typename T>
  static int gcProxy (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), true);
    Userdata* const ud = static_cast <Userdata*> (p);
    ud->~Userdata ();
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a class member function with a return value.

    The argument list contains the 'this' pointer followed by the method
    arguments.

    @note The expected class name is in upvalue 1, and the member function
          pointer is in upvalue 2.
  */
  template <typename MemberFunction,
            typename ReturnType = typename fnptr <MemberFunction>::resulttype>
  struct methodProxy
  {
    typedef typename fnptr <MemberFunction>::classtype T;
    typedef typename fnptr <MemberFunction>::params params;
    static int f (lua_State* L)
    {
      //char const* n = typeid (MemberFunction).name ();
      void* const p = detail::checkClass (
        L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
      Userdata* const ud = static_cast <Userdata*> (p);
      T* const t = ud->get <T> (L);
      MemberFunction fp = *static_cast <MemberFunction*> (
        lua_touserdata (L, lua_upvalueindex (2)));
      arglist <params, 2> args(L);
      tdstack <ReturnType>::push (L, fnptr <MemberFunction>::call (t, fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a class member function with no return value.

    The argument list contains the 'this' pointer followed by the method
    arguments.

    @note The expected class name is in upvalue 1, and the member function
          pointer is in upvalue 2.
  */
  template <typename MemberFunction>
  struct methodProxy <MemberFunction, void>
  {
    typedef typename fnptr <MemberFunction>::classtype T;
    typedef typename fnptr <MemberFunction>::params params;
    static int f (lua_State* L)
    {
      void* const p = detail::checkClass (
        L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
      Userdata* const ud = static_cast <Userdata*> (p);
      T* const t = ud->get <T> (L);
      MemberFunction fp = *static_cast <MemberFunction*> (
        lua_touserdata (L, lua_upvalueindex (2)));
      arglist <params, 2> args (L);
      fnptr <MemberFunction>::call (t, fp, args);
      return 0;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to get a class data member.

    @note The expected class name is in upvalue 1, and the pointer to the
          data member is in upvalue 2.
  */
  template <typename T, typename U>
  static int propgetProxy (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T const* const t = ud->getConst <T> (L);
    U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (2)));
    tdstack <U>::push (L, t->*mp);
    return 1;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to set a class data member.

    @note The expected class name is in upvalue 1, and the pointer to the
          data member is in upvalue 2.
  */
  template <typename T, typename U>
  static int propsetProxy (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T* const t = ud->get <T> (L);
    U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (2)));
    t->*mp = tdstack <U>::get (L, 2);
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    Get a pointer to the class from the Lua stack.
  */
  template <class T>
  static T* get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::name(), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    return ud->get <T> (L);
  }

  //----------------------------------------------------------------------------
  /**
    Get a const pointer to the class from the Lua stack.
  */
  template <class T>
  static T const* getConst (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::const_name (), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    return ud->getConst <T> (L);
  }

public:
  //----------------------------------------------------------------------------
  /**
    Get the registered class name.
  */
  virtual char const* getName () const = 0;

  //----------------------------------------------------------------------------
  /**
    Get this object's concrete type name (compiler specific).
  */
  virtual char const* getTypename () const = 0;

protected:
  virtual ~Userdata () { }

  //----------------------------------------------------------------------------
  /**
    Get a pointer to the class.

    @note The lua_State is provided for diagnostics.
  */
  template <class T>
  T* get (lua_State* L)
  {
    //assert (classname <T>::name () == getName ());
    return static_cast <T*> (getPointer (L));
  }

  //----------------------------------------------------------------------------
  /**
    Get a const pointer to the class.

    @note The lua_State is provided for diagnostics.
  */
  template <class T>
  T const* getConst (lua_State* L)
  {
    //assert (classname <T>::name () == getName ());
    return static_cast <T const*> (getConstPointer (L));
  }

private:
  virtual void* getPointer (lua_State* L) = 0;
  virtual void const* getConstPointer (lua_State* L) = 0;
};

//------------------------------------------------------------------------------
/**
  Class passed by value.

  The object lifetime is fully managed by Lua.

  @note T must be copy-constructible.
*/
template <class T>
class UserdataByValue : public Userdata
{
public:
  char const* getName () const { return classname <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByValue (T t) : m_t (t)
  {
  }

  static void push (lua_State* L, T t)
  {
    assert (classname <T>::isRegistered ());
    void* const p = lua_newuserdata (L, sizeof (UserdataByValue <T>));
    new (p) UserdataByValue <T> (t);
    luaL_getmetatable (L, classname <T>::name ());
    lua_setmetatable (L, -2);
  }

  static T get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::name ());
    Userdata* const ud = static_cast <Userdata*> (p);
    return *ud->getClassPointer <T> ();
  }

private:
  void* getPointer (lua_State* L)
  {
    (void)L;
    return &m_t;
  }

  void const* getConstPointer (lua_State* L)
  {
    (void)L;
    return &m_t;
  }

private:
  T m_t;
};

//------------------------------------------------------------------------------
/**
  Class passed by reference.

  The object lifetime is fully managed by C++.
*/
template <class T>
class UserdataByReference : public Userdata
{
public:
  char const* getName () const { return classname <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByReference (T& t) : m_t (t)
  {
  }

  ~UserdataByReference ()
  {
  }

  static void push (lua_State* L, T& t)
  {
    assert (classname <T>::isRegistered ());
    void* const p = lua_newuserdata (L, sizeof (UserdataByReference <T>));
    new (p) UserdataByReference <T> (t);
    luaL_getmetatable (L, classname <T>::name ());
    lua_setmetatable (L, -2);
  }

  static T& get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::name ());
    Userdata* const pb = static_cast <Userdata*> (p);
    /** @todo Allow references from a other Userdata types? */
    UserdataByReference <T>* ud = dynamic_cast <UserdataByReference <T>*> (pb);
    if (ud == 0)
      luaL_argerror (L, index, lua_pushfstring (L,
        "%s expected, got %s", getTypename (), pb->getTypename ()));
    return ud->m_t;
  }

private:
  void* getPointer (lua_State* L)
  {
    (void)L;
    return &m_t;
  }

  void const* getConstPointer (lua_State* L)
  {
    (void)L;
    return &m_t;
  }

private:
  T& m_t;
};

//------------------------------------------------------------------------------
/**
  Class passed by const reference.

  The object lifetime is fully managed by C++.
*/
template <class T>
class UserdataByConstReference : public Userdata
{
public:
  char const* getName () const { return classname <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByConstReference (T const& t) : m_t (t)
  {
  }

  static void push (lua_State* L, T const& t)
  {
    assert (classname <T>::isRegistered ());
    void* const p = lua_newuserdata (L, sizeof (UserdataByConstReference <T>));
    new (p) UserdataByConstReference <T> (t);
    luaL_getmetatable (L, classname <T>::const_name ());
    lua_setmetatable (L, -2);
  }

  static T const& get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::const_name ());
    Userdata* const pb = static_cast <Userdata*> (p);
    /** @todo Allow references from a other Userdata types? */
    UserdataByReference <T>* ud = dynamic_cast <UserdataByReference <T>*> (pb);
    if (ud == 0)
      luaL_argerror (L, index, lua_pushfstring (L,
        "%s expected, got %s", getTypename (), pb->getTypename ()));
    return ud->m_t;
  }

private:
  void* getPointer (lua_State* L)
  {
    luaL_argerror (L, "illegal non-const use of %s", getName ());
    return 0; // never gets here
  }

  void const* getConstPointer ()
  {
    return &m_t;
  }

private:
  T const& m_t;
};

//------------------------------------------------------------------------------
/**
  Class passed by container.

  The object lifetime is managed by the container.

  @note Container must implement a strict subset of shared_ptr.
*/
template <class T, template <class> class Container = shared_ptr>
class UserdataByContainer : public Userdata
{
public:
  /**
    lua_CFunction to construct a class object.

    These are registered to Lua as global functions with the name of the class,
    with the appropriate metatable passed as an upvalue.
  */
  template <typename Params>
  static int ctorProxy (lua_State* L)
  {
    arglist <Params, 2> args (L);
    T* const t = constructor <T, Params>::call (args);
    push (L, t);
    return 1;
  }

public:
  char const* getName () const { return classname <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByContainer (T* const t) : m_p (t)
  {
  }

  template <class U>
  explicit UserdataByContainer (U* const u) : m_p (u)
  {
  }

  static void push (lua_State* L, T* const t)
  {
    assert (classname <T>::isRegistered ());
    void* const p = lua_newuserdata (L, sizeof (UserdataByContainer <T, Container>));
    new (p) UserdataByContainer <T, Container> (t);
    luaL_getmetatable (L, classname <T>::name ());
    lua_setmetatable (L, -2);
  }

  static Container <T> get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::name (), false);
    Userdata* const pb = static_cast <Userdata*> (p);
    UserdataByContainer <T>* ud = reinterpret_cast <UserdataByContainer <T>*> (pb);
    if (ud == 0)
      luaL_argerror (L, index, lua_pushfstring (L, "%s expected, got %s",
        typeid (UserdataByContainer <T, Container>).name (), pb->getTypename ()));
    return ud->m_p;
  }

private:
  void* getPointer (lua_State* L)
  {
    (void)L;
    return *m_p;
  }

  void const* getConstPointer (lua_State* L)
  {
    (void)L;
    return *m_p;
  }

private:
  typename Container <T> m_p;
};

//------------------------------------------------------------------------------
/**
  Class passed by const container.

  The object lifetime is managed by the container.

  @note Container must implement a strict subset of shared_ptr.
*/
template <class T, template <class> class Container = shared_ptr>
class UserdataByConstContainer : public Userdata
{
public:
  char const* getName () const { return classname <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByConstContainer (T const* const t) : m_p (t)
  {
  }

  template <class U>
  explicit UserdataByConstContainer (U const* const u) : m_p (u)
  {
  }

  static void push (lua_State* L, T const* const t)
  {
    assert (classname <T>::isRegistered ());
    void* const p = lua_newuserdata (L, sizeof (UserdataByContainer <T, Container>));
    new (p) UserdataByConstContainer <T, Container> (t);
    luaL_getmetatable (L, classname <T>::const_name ());
    lua_setmetatable (L, -2);
  }

  /*
  static Container <T> get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, classname <T>::name (), false);
    Userdata* const pb = static_cast <Userdata*> (p);
    UserdataByContainer <T>* ud = reinterpret_cast <UserdataByContainer <T>*> (pb);
    if (ud == 0)
      luaL_argerror (L, index, lua_pushfstring (L, "%s expected, got %s",
        typeid (UserdataByContainer <T, Container>).name (), pb->getTypename ()));
    return ud->m_p;
  }
  */

private:
  void* getPointer (lua_State* L)
  {
#if 0
    luaL_error (L, "illegal non-const use of %s", getName ());
    return 0; // never gets here
#else
    (void)L;
    void const* p = *m_p;
    return const_cast <void*> (p);
#endif
  }

  void const* getConstPointer (lua_State* L)
  {
    (void)L;
    return *m_p;
  }

private:
  typename Container <T const> m_p;
};

//==============================================================================
/**
  Lua stack objects with value semantics.

  @note T must be copy-constructible.
*/
template <typename T>
struct tdstack
{
public:
  static void push (lua_State* L, T t)
  {
    UserdataByValue <T>::push (L, t);
  }

  static T get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with pointer semantics.
*/
template <typename T>
struct tdstack <T*>
{
  static void push (lua_State* L, T* t)
  {
    UserdataByReference <T>::push (L, *t);
  }

  static T* get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with pointer semantics.
*/
template <typename T>
struct tdstack <T* const>
{
  static void push (lua_State* L, T* const t)
  {
    UserdataByReference <T>::push (L, *t);
  }

  static T* const get (lua_State* L, int index)
  {
    return Userdata::get <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const pointer semantics.
*/
template <typename T>
struct tdstack <T const*>
{
  static void push (lua_State* L, T const* t)
  {
    UserdataByConstReference <T>::push (L, *t);
  }
  static T const* get (lua_State* L, int index)
  {
    return Userdata::getConst <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const pointer semantics.
*/
template <typename T>
struct tdstack <T const* const>
{
  static void push (lua_State* L, T const* const t)
  {
    UserdataByConstReference <T>::push (L, *t);
  }
  static T const* const get (lua_State* L, int index)
  {
    return Userdata::getConst <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with reference semantics.
*/
template <typename T>
struct tdstack <T&>
{
  static void push (lua_State* L, T& t)
  {
    UserdataByReference <T>::push (L, t);
  }

  static T& get (lua_State* L, int index)
  {
    return *Userdata::get <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const reference semantics.
*/
template <typename T>
struct tdstack <T const&>
{
  static void push (lua_State* L, T const& t)
  {
    UserdataByConstReference <T>::push (L, t);
  }

  static T const& get (lua_State* L, int index)
  {
    return *Userdata::getConst <T> (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with shared_ptr semantics.
*/
template <typename T>
struct tdstack <shared_ptr <T> >
{
  static void push (lua_State* L, shared_ptr <T> p)
  {
    UserdataByContainer <T, shared_ptr>::push (L, *p);
  }

  static shared_ptr<T> get (lua_State* L, int index)
  {
    return UserdataByContainer <T, shared_ptr>::get (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const shared_ptr semantics.
*/
#if 0
template <typename T>
struct tdstack <shared_ptr<const T> >
{
  static void push (lua_State* L, shared_ptr <const T> p)
  {
    UserdataByContainer <T, shared_ptr>::push (L, const_cast <T*>(*p));
  }
  static shared_ptr <T> get (lua_State* L, int index)
  {
    return UserdataByContainer <const T, shared_ptr>::get (L, index);
  }
};
#else
template <typename T>
struct tdstack <shared_ptr <T const> >
{
  static void push (lua_State* L, shared_ptr <T const> p)
  {
    UserdataByConstContainer <T, shared_ptr>::push (L, *p);
  }
  static shared_ptr <T const> get (lua_State* L, int index)
  {
    return UserdataByConstContainer <T, shared_ptr>::get (L, index);
  }
};
#endif

//------------------------------------------------------------------------------

// int
template <> struct tdstack <
  int > { static void push (lua_State* L,
  int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  int get (lua_State* L, int index) { return static_cast <
  int > (luaL_checknumber (L, index)); } };

// unsigned int
template <> struct tdstack <
  unsigned int > { static void push (lua_State* L,
  unsigned int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned int get (lua_State* L, int index) { return static_cast <
  unsigned int > (luaL_checknumber (L, index)); } };

// unsigned char
template <> struct tdstack <
  unsigned char > { static void push (lua_State* L,
  unsigned char value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned char get (lua_State* L, int index) { return static_cast <
  unsigned char > (luaL_checknumber (L, index)); } };

// short
template <> struct tdstack <
  short > { static void push (lua_State* L,
  short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  short get (lua_State* L, int index) { return static_cast <
  short > (luaL_checknumber (L, index)); } };

// unsigned short
template <> struct tdstack <
  unsigned short > { static void push (lua_State* L,
  unsigned short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned short get (lua_State* L, int index) { return static_cast <
  unsigned short > (luaL_checknumber (L, index)); } };

// long
template <> struct tdstack <
  long > { static void push (lua_State* L,
  long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  long get (lua_State* L, int index) { return static_cast <
  long > (luaL_checknumber (L, index)); } };

// unsigned long
template <> struct tdstack <
  unsigned long > { static void push (lua_State* L,
  unsigned long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned long get (lua_State* L, int index) { return static_cast <
  unsigned long > (luaL_checknumber (L, index)); } };

// float
template <> struct tdstack <
  float > { static void push (lua_State* L,
  float value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  float get (lua_State* L, int index) { return static_cast <
  float > (luaL_checknumber (L, index)); } };

// double
template <> struct tdstack <
  double > { static void push (lua_State* L,
  double value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  double get (lua_State* L, int index) { return static_cast <
  double > (luaL_checknumber (L, index)); } };

// bool
template <>
struct tdstack <bool>
{
  static void push (lua_State* L, bool value)
  {
    lua_pushboolean (L, value ? 1 : 0);
  }
  static bool get (lua_State* L, int index)
  {
    luaL_checktype (L, index, LUA_TBOOLEAN);

    return lua_toboolean (L, index) ? true : false;
  }
};

// char
template <>
struct tdstack <char>
{
  static void push (lua_State* L, char value)
  {
    char str [2] = { value, 0 };
    lua_pushstring (L, str);
  }
  static char get (lua_State* L, int index)
  {
    return luaL_checkstring (L, index) [0];
  }
};

// null terminated string
template <>
struct tdstack <char const*>
{
  static void push (lua_State* L, char const* str)
  {
    lua_pushstring (L, str);
  }
  static char const* get (lua_State* L, int index)
  {
    return luaL_checkstring (L, index);
  }
};

// std::string
template <>
struct tdstack <std::string>
{
  static void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str ());
  }
  static std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};

// std::string const&
template <>
struct tdstack <std::string const&>
{
  static void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str());
  }
  static std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};

//------------------------------------------------------------------------------

/*
* Subclass of a type/value list, constructable from the Lua stack.
*/

template <typename Typelist, int start = 1>
struct arglist
{
};

template <int start>
struct arglist <nil, start> : public typevallist <nil>
{
  arglist (lua_State*)
  {
  }
};

template <typename Head, typename Tail, int start>
struct arglist <typelist <Head, Tail>, start>
  : public typevallist <typelist <Head, Tail> >
{
  arglist (lua_State* L)
    : typevallist <typelist <Head, Tail> > (tdstack <Head>::get (L, start),
                                            arglist <Tail, start + 1> (L))
  {
  }
};

//==============================================================================
/**
  lua_CFunction to call a function with a return value.
*/
template <typename Function,
          typename Retval = typename fnptr <Function>::resulttype>
struct functionProxy
{
  typedef typename fnptr <Function>::params params;
  static int f (lua_State* L)
  {
    // The upvalue contains the function pointer.
    Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
    arglist <params> args (L);
    tdstack <Retval>::push (L, fnptr <Function>::call (fp, args));
    return 1;
  }
};

//------------------------------------------------------------------------------
/**
  lua_CFunction to call a function with no return value.
*/
template <typename Function>
struct functionProxy <Function, void>
{
  typedef typename fnptr <Function>::params params;
  static int f (lua_State* L)
  {
    // The upvalue contains the function pointer.
    Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
    arglist <params> args (L);
    fnptr <Function>::call (fp, args);
    return 0;
  }
};

//------------------------------------------------------------------------------
/**
  lua_CFunction to get a variable.

  This is also used for static data members of classes
*/
template <typename T>
int propgetProxy (lua_State* L)
{
  // The upvalue holds a pointer to the variable.
  T* data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
  tdstack <T>::push (L, *data);
  return 1;
}

//------------------------------------------------------------------------------
/**
  lua_CFunction to set a variable.

  This is also used for static data members of classes.
*/

template <typename T>
int propsetProxy (lua_State* L)
{
  // The upvalue holds a pointer to the variable.
  T* data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
  *data = tdstack <T>::get (L, 1);
  return 0;
}

//------------------------------------------------------------------------------
/**
  Create a metatable.
*/

template <typename T>
void createMetaTable (lua_State* L)
{
  char const* const name = classname <T>::name ();
  luaL_newmetatable (L, name);
  lua_pushcfunction (L, &detail::indexer);
  rawsetfield (L, -2, "__index");                     // Use our __index.
  lua_pushcfunction (L, &detail::object_newindexer);
  rawsetfield (L, -2, "__newindex");                  // Use our __newindex.
  lua_pushstring (L, name);
  lua_pushcclosure (L, &Userdata::gcProxy <T>, 1);
  rawsetfield (L, -2, "__gc");                        // Use our __gc
  lua_pushstring (L, name);
  rawsetfield (L, -2, "__type");                      // Set __type to class name.
  lua_newtable (L);
  rawsetfield (L, -2, "__propget");                   // Create empty __propget.
  lua_newtable (L);
  rawsetfield (L, -2, "__propset");                   // Create empty __propset.
}

//------------------------------------------------------------------------------
/**
  Create a metatable suitable for a const object.
*/

template <typename T>
void createConstMetaTable (lua_State* L)
{
  char const* const name = classname <T>::const_name ();
  luaL_newmetatable (L, name);
  lua_pushcfunction (L, &detail::indexer);
  rawsetfield (L, -2, "__index");                     // Use our __index.
  lua_pushcfunction (L, &detail::object_newindexer);
  rawsetfield (L, -2, "__newindex");                  // Use our __newindex.
  lua_pushstring (L, name);
  lua_pushcclosure (L, &Userdata::gcProxy <T>, 1);
  rawsetfield (L, -2, "__gc");                        // Use our __gc.
  lua_pushstring (L, name);
  rawsetfield (L, -2, "__type");                      // Store the class type.
  lua_newtable (L);
  rawsetfield (L, -2, "__propget");                   // Create empty __propget.
}

//==============================================================================
/**
  Registration manager.

  Performs registration tasks for a specified Lua state.

  @todo namespace support.
*/
class scope : public detail
{
public:
  //----------------------------------------------------------------------------
  /**
    Construct a scope for global registrations.
  */
  explicit scope (lua_State *L_) : L (L_)
  {
    /** @todo Set up global metatable? */
  }

  //----------------------------------------------------------------------------
  /**
    Construct a scope with the specified dot-separated name.
  */
  scope (lua_State *L_, char const *name_) : L (L_), name (name_)
  {
    createStaticTables (L, name);
  }

  //----------------------------------------------------------------------------
  /**
    Register a function in this scope.
  */
  template <typename Function>
  scope& function (char const* const name, Function fp)
  {
    findStaticTable (L, this->name.c_str());
    lua_pushlightuserdata (L, reinterpret_cast <void*> (fp));
    lua_pushcclosure (L, &functionProxy <Function>::f, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 1);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read-only variable.

    The variable is retrieved through the provided pointer.

    @note The proxy function is stored in the __propget table.
  */
  template <typename T>
  scope& variable_ro (char const* name, T const* data)
  {
    // Currently can't register properties at global scope.
    assert (this->name.length() > 0);

    findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propget");
    lua_pushlightuserdata (L, const_cast <void*> (static_cast <void const*> (data)));
    lua_pushcclosure (L, &propgetProxy <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read-only variable.

    The variable is retrieved through the provided function.

    @note The proxy function is stored in the __propget table.
  */
  template <typename T>
  scope& variable_ro (char const* name, T (*getFunction) ())
  {
    // Currently can't register properties at global scope.
    assert (this->name.length() > 0);

    findStaticTable (L, this->name.c_str ());
    rawgetfield(L, -1, "__propget");
    lua_pushlightuserdata (L, reinterpret_cast <void*> (getFunction));
    lua_pushcclosure (L, &functionProxy<T (*) ()>::f, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read-write variable.

    The variable is retrieved and stored through the provided pointer.

    @note The proxy function is stored in the __propset table.
  */
  template <typename T>
  scope& variable_rw (char const* name, T* data)
  {
    // Currently can't register properties at global scope.
    assert (this->name.length() > 0);

    variable_ro <T> (name, data);
    findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushlightuserdata (L, static_cast <void*> (data));
    lua_pushcclosure (L, &propsetProxy <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read-write variable.

    The variable is retrieved and stored through the provided functions.

    @note The proxy function is stored in the __propset table.
  */
  template <typename T>
  scope& variable_rw (char const* name, T (*getFunction) (), void (*setFunction) (T))
  {
    // Currently can't register properties at global scope.
    assert (this->name.length() > 0);

    variable_ro <T> (name, getFunction);
    findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushlightuserdata (L, reinterpret_cast <void*> (setFunction));
    lua_pushcclosure (L, &functionProxy <void (*) (T)>::f, 1);
    rawsetfield(L, -2, name);
    lua_pop(L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a new class.
  */
  template <class T>
  class__ <T> class_ (char const* name)
  {
    return class__ <T> (L, name);
  }

  //----------------------------------------------------------------------------
  /**
    Add registrations to a class.

    The class must already be registered.
  */

  template <class T>
  class__ <T> class_ ()
  {
    return class__ <T> (L);
  }

  //----------------------------------------------------------------------------
  /**
    Register a subclass.

    @note The base class must be registered.
  */

  template <class T, class Base>
  class__ <T> subclass (char const *name)
  {
    assert (classname <Base>::isRegistered ());
    return class__ <T> (L, name, classname <Base>::name ());
  }

protected:
  lua_State* L;
  std::string name;
};

//==============================================================================
/**
  Perform registration for class members.
*/
template <class T>
class class__ : public scope
{
public:
  //----------------------------------------------------------------------------
  explicit class__ (lua_State *L_) : scope (L_, classname <T>::name ())
  {
    assert (classname <T>::isRegistered ());
  }

  //----------------------------------------------------------------------------
  class__ (lua_State *L_, char const *name_) : scope(L_, name_)
  {
    assert (!classname <T>::isConst ());
    classname <T>::registerClass (name_);

    // Create metatable for this class.  The metatable is stored in the Lua
    // registry, keyed by the given class name.
    createMetaTable <T> (L);

    // Create const metatable for this class.  This is identical to the
    // previous metatable, except that it has "const " prepended to the __type
    // field, and has no __propset field.  Const methods will be added to the
    // const metatable, non-const methods to the normal metatable.
    createConstMetaTable <T> (L);

    // Set __const metafield to point to the const metatable
    rawsetfield (L, -2, "__const");

    // Pop the original metatable
    lua_pop(L, 1);
  }

  //----------------------------------------------------------------------------
  class__ (lua_State *L_, char const *name_, char const *basename) : scope(L_, name_)
  {
    assert (!classname <T>::isConst ());
    classname <T>::registerClass (name_);

    // Create metatable for this class
    createMetaTable <T> (L);
    // Set the __parent metafield to the base class's metatable
    luaL_getmetatable(L, basename);
    rawsetfield(L, -2, "__parent");

    // Create const metatable for this class.  Its __parent field will point
    // to the const metatable of the parent class.
    createConstMetaTable <T> (L);
    std::string base_constname = std::string("const ") + basename;
    luaL_getmetatable(L, base_constname.c_str());
    rawsetfield(L, -2, "__parent");

    // Set __const metafield to point to the const metatable
    rawsetfield(L, -2, "__const");
    // Pop the original metatable
    lua_pop(L, 1);

    // Set the __parent metafield to the base class's static table
    findStaticTable(L, name_);
    findStaticTable(L, basename);
    rawsetfield(L, -2, "__parent");
    lua_pop(L, 1);
  }

  //----------------------------------------------------------------------------
  // Constructor registration.  The template parameter should be passed
  // a function pointer type; only the argument list will be used (since
  // you can't take the address of a ctor).
  template <typename FnPtr>
  class__ <T>& constructor ()
  {
    // Get a reference to the class's static table
    findStaticTable (L, name.c_str());

    // Push the constructor proxy, with the class's metatable as an upvalue
    luaL_getmetatable(L, name.c_str());
    lua_pushcclosure (L, &UserdataByContainer <T>::ctorProxy <typename fnptr <FnPtr>::params>, 1);

    // Set the constructor proxy as the __call metamethod of the static table
    rawsetfield(L, -2, "__call");
    lua_pop (L, 1);
    return *this;
  }

  //----------------------------------------------------------------------------
  /*
  * Perform method registration in a class.  The method proxies are all
  * registered as values in the class's metatable, which is searched by the
  * indexer function we've installed as __index metamethod.
  */
  template <typename FnPtr>
  class__ <T>& method (char const *name, FnPtr fp)
  {
    assert (fnptr <FnPtr>::mfp);
    std::string metatable_name = this->name;
    // Disable MSVC's warning 'conditional expression is constant'
  #ifdef _MSC_VER
  #  pragma warning (push)
  #  pragma warning (disable: 4127)
  #endif
    if (fnptr<FnPtr>::const_mfp)
      metatable_name.insert(0, "const ");
  #ifdef _MSC_VER
  #  pragma warning (pop)
  #endif
    luaL_getmetatable(L, metatable_name.c_str());
    lua_pushstring(L, metatable_name.c_str());
    void *v = lua_newuserdata(L, sizeof(FnPtr));
    memcpy(v, &fp, sizeof(FnPtr));
    lua_pushcclosure(L, &Userdata::methodProxy<FnPtr>::f, 2);
    rawsetfield(L, -2, name);
    lua_pop(L, 1);
    return *this;
  }

  //----------------------------------------------------------------------------
  // Property registration.  Properties can be read/write (rw)
  // or read-only (ro).  Varieties that access member pointers directly
  // and varieties that access through function calls are provided.
  /* Property registration.  Properties are stored in the class's __propget
  * metafield, with the property name as the get-function and property name
  * + "_set" as the set-function.  Note that property getters are stored
  * both in the regular metatable and the const metatable.
  */
  template <typename U>
  class__ <T>& property_ro (char const* name, const U T::* mp)
  {
    luaL_getmetatable(L, this->name.c_str());
    std::string cname = "const " + this->name;
    luaL_getmetatable(L, cname.c_str());
    rawgetfield(L, -2, "__propget");
    rawgetfield(L, -2, "__propget");
    lua_pushstring(L, cname.c_str());
    void *v = lua_newuserdata(L, sizeof(U T::*));
    memcpy(v, &mp, sizeof(U T::*));
    lua_pushcclosure(L, &Userdata::propgetProxy<T, U>, 2);
    lua_pushvalue(L, -1);
    rawsetfield(L, -3, name);
    rawsetfield(L, -3, name);
    lua_pop(L, 4);
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename U>
  class__ <T>& property_ro (char const *name, U (T::*get) () const)
  {
    luaL_getmetatable(L, this->name.c_str());
    std::string cname = "const " + this->name;
    luaL_getmetatable(L, cname.c_str());
    rawgetfield(L, -2, "__propget");
    rawgetfield(L, -2, "__propget");
    lua_pushstring(L, cname.c_str());
    typedef U (T::*FnPtr) () const;
    void *v = lua_newuserdata(L, sizeof(FnPtr));
    memcpy(v, &get, sizeof(FnPtr));
    lua_pushcclosure(L, &Userdata::methodProxy <FnPtr>::f, 2);
    lua_pushvalue(L, -1);
    rawsetfield(L, -3, name);
    rawsetfield(L, -3, name);
    lua_pop(L, 4);
    return *this;
  }

  //----------------------------------------------------------------------------
  template <typename U>
  class__ <T>& property_rw (char const *name, U T::* mp)
  {
    property_ro<U>(name, mp);
    luaL_getmetatable(L, this->name.c_str());
    rawgetfield(L, -1, "__propset");
    lua_pushstring(L, this->name.c_str());
    void *v = lua_newuserdata(L, sizeof(U T::*));
    memcpy(v, &mp, sizeof(U T::*));
    lua_pushcclosure(L, &Userdata::propsetProxy<T, U>, 2);
    rawsetfield(L, -2, name);
    lua_pop(L, 2);
    return *this;
  }

  template <typename U>
  class__ <T>& property_rw (char const *name, U (T::*get) () const, void (T::*set) (U))
  {
    property_ro<U>(name, get);
    luaL_getmetatable(L, this->name.c_str());
    rawgetfield(L, -1, "__propset");
    lua_pushstring(L, this->name.c_str());
    typedef void (T::*FnPtr) (U);
    void *v = lua_newuserdata(L, sizeof(FnPtr));
    memcpy(v, &set, sizeof(FnPtr));
    lua_pushcclosure(L, &Userdata::methodProxy <FnPtr>::f, 2);
    rawsetfield(L, -2, name);
    lua_pop(L, 2);
    return *this;
  }

  // Static method registration
  template <typename FnPtr>
  class__ <T>& static_method (char const *name, FnPtr fp)
  {
    return *(class__ <T>*)&(function (name, fp));
  }

  // Static property registration
  template <typename U>
  class__ <T>& static_property_ro (char const *name, const U *data)
  {
    return *(class__ <T>*)&(variable_ro <U> (name, data));
  }
  
  template <typename U>
  class__ <T>& static_property_ro (char const *name, U (*get) ())
  {
    return *(class__ <T>*)&(variable_ro <U> (name, get));
  }
  
  template <typename U>
  class__ <T>& static_property_rw (char const *name, U *data)
  {
    return *(class__ <T>*)&(variable_rw <U> (name, data));
  }
  
  template <typename U>
  class__ <T>& static_property_rw (char const *name, U (*get) (), void (*set) (U))
  {
    return *(class__ <T>*)&(variable_rw <U> (name, get, set));
  }

  /** @todo Inherit Lua classes from C++ classes */
};

}


#endif
