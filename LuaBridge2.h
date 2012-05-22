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

#ifndef LUABRIDGE_USE_APICHECK
#ifdef LUA_USE_APICHECK
#define LUABRIDGE_USE_APICHECK 1
#else
#define LUABRIDGE_USE_APICHECK 0
#endif
#endif

/** This turns on code that enforces const-correctness for member functions
    but I can't get it to compile - Vinnie
*/
#define LUABRIDGE_STRICT_CONST 0

//==============================================================================
/**
  @mainpage LuaBridge: Simple C++ to Lua bindings.

  @details

  # LuaBridge

  [LuaBridge][3] is a lightweight, dependency-free library for binding to C++.
  It works with Lua revisions starting from 5.1.2. [Lua][4] is a powerful, fast,
  lightweight, embeddable scripting language.

  ## Compiling

  LuaBridge is distributed as a single header file "LuaBridge.h" that you simply
  include where you want to register your functions, classes, and variables.
  There are no additional source files, no compilation settings, and no
  Makefiles or IDE-specific project files. LuaBridge is easy to integrate.

  ## Usage

  LuaBridge is based on C++ template metaprogramming.  It contains template code
  to automatically generate at compile-time the various Lua API calls necessary
  to export your program's classes and functions to the Lua environment.

  ### Registering functions

  If `L` is a pointer to an instance of `lua_State`, the following code creates
  a LuaBridge scope for registering C++ functions and classes to `L`:

      luabridge::scope s (L);

  Functions can then be registered as follows:

      s .function ("foo", &foo)
        .function ("bar", &bar);

  The `function` function returns a reference to `s`, so you can chain many
  definitions together.  The first argument is the name by which the function
  will be available in Lua, and the second is the function's address. LuaBridge
  will automatically detect the number (up to 8, by default) and type of the
  parameters.  Functions registered this way will be available at the global
  scope to Lua scripts executed by `L`.  Overloading of function names is not
  supported, nor is it likely to be supported in the future.

  ### Registering data

  Variables can also be registered.  You can expose a 'bare' variable to Lua, or
  wrap it in getter and setter functions:

      s .variable_rw ("var1", &var1)
        .variable_rw ("var2", &getter2, &setter2)
        .variable_ro ("var3", &var3)
        .variable_ro ("var4", &getter4)

  The first registration above gives Lua direct access to the `var1` variable.
  The second creates a variable which appears like any other variable to Lua
  code, but is retrieved and set through the `getter2` and `setter2` function.
  The getter must take no parameters and return a value, and the setter must
  take a value of the same type and return nothing.  The `variable_rw` function
  creates a readable and writeable variable, while `variable_ro` creates a
  read-only one. Obviously, there is no setter for the read-only variable.

  Basic types for supported variables, and function arguments and returns, are:
  
  - `bool`
  - `char`, converted to a string of length one.
  - Integers, `float`, and `double`, converted to Lua_number.
  - Strings: `char const*` and `std::string`

  Of course, LuaBridge supports passing objects of class type, in a variety of
  ways including dynamically allocated objects created with `new`. The behavior
  of the object with respect to lifetime management depends on the manner in
  which the object is passed. Given `class T`, these argument types are
  supported:

  - `T`, `T const` : Pass `T` by value. The lifetime is managed by Lua.
  - `T*`, `T&`, `T const*`, `T const&` : Pass `T` by reference. The lifetime
     is managed by C++.
  - `SharedPtr <T>`, `SharedPtr <T const>` : Pass `T` by container. The lifetime
     is managed by the container.

  When Lua manages the lifetime of the object, it is subjected to all of the
  normal garbage collection rules. C++ functions and member functions can
  receive pointers and references to these objects, but care must be taken
  to make sure that no attempt is made to access the object after it is
  garbage collected. Usually this is done by simply not storing a pointer to
  the object somewhere inside your C++.

  When C++ manages the lifetime of the object, the Lua garbage collector has
  no effect on it. Care must be taken to make sure that the C++ code does not
  destroy the object while Lua still has a reference to it.

  ### Shared Pointers

  A `SharedPtr` container template allows for object lifetime management that
  behaves like `std::shared_ptr`. That is, objects are dynamically allocated
  and reference counted. The object is not destroyed until the reference count
  drops to zero. Such objects are safe to store in C++ and Lua code. A
  garbage collection will only decrement the reference count.

  LuaBridge registraton templates will automatically detect arguments which
  behave like containers. For example, this registration is valid:

      extern void func (shared_ptr <T> p);

      s.function ("func", &func);

  Any container may be used. LuaBridge expects that the container in question
  is a class template with one template argument, and a member function called
  get() which returns a pointer to the underlying object. If you need to use
  a container with a different interface, you can specialize the `Container`
  class for your container type and provide an extraction function. Your
  specialization needs to be in the `luabridge` namespace. Here's an example
  specialization for a container called `ReferenceCountedObject` which provides
  a member function `getObject` that retrieves the pointer.

      namespace luabridge
      {
          template <>
          struct Container <ReferenceCountedObjectPtr>
          {
            template <class T>
            static T* get (ReferenceCountedObjectPtr <T> const& p)
            {
              return p.getObject ();
            }
         };
      }

  ### Registering classes

  C++ classes can be registered with Lua as follows:

      s .class_ <MyClass> ("MyClass")
        .constructor <void (*) (void)> ()
        .method ("method1", &MyClass::method1)
        .method ("method2", &MyClass::method2);

      s .subclass <MySubclass, MyBaseClass> ("MySubclass")
        .constructor <...>
        ...

  The `class_` function registers a class; its constructor will be available as
  a global function with name given as argument to `class_`.  The object
  returned can then be used to register the constructor (no overloading is
  supported, so there can only be one constructor) and methods.

  LuaBridge cannot automatically determine the number and types of constructor
  parameters like it can for functions and methods, so you must provide them.
  This is done by letting the `constructor` function take a template parameter,
  which must be a function pointer type.  The parameter types will be extracted
  from this (the return type is ignored).  For example, to register a
  constructor taking two parameters, one `int` and one `char const*`, you would
  write:

      s .class_ <MyClass> ()
        .constructor <void (*) (int, const char *)> ()

  Note that in the example above, the name of the class was ommitted from the
  argument to the `class` function. This allows you to add additional
  registrations to a class that has already been registered.

  Method registration works just like function registration.  Virtual methods
  work normally; no special syntax is needed. const methods are detected and
  const-correctness is enforced, so if a function returns a const object (or
  rather, a `shared_ptr` to a const object) to Lua, that reference to the object
  will be considered const and only const methods will be called on it.
  Destructors are registered automatically for each class.

  Static methods may be registered using the `static_method` function, which is
  simply an alias for the `function` function:

      s .class_ <MyClass> ()
        .static_method ("method3", &MyClass::method3)

  LuaBridge also supports properties, which allow class data members to be read
  and written from Lua as if they were variables.  Properties work much like
  variables do, and the syntax for registering them is as follows:

      s .class_ <MyClass> ()
        .property_rw ("property1", &MyClass::property1)
        .property_rw ("property2", &MyClass::getter2, &MyClass::setter2)
        .property_ro ("property3", &MyClass::property3)
        .property_ro ("property4", &MyClass::getter4)

  Static properties on classes are also supported, using `static_property_rw`
  and `static_property_ro`, which are simply aliases for `variable_rw` and
  `variable_ro`.

  To register a subclass of another class that has been registered with Lua, use
  the `subclass` function.  This takes two template arguments: the class to be
  registered, and its base class.  Inherited methods do not have to be
  re-declared and will function normally in Lua.  If a class has a base class
  that is *not* registered with Lua, there is no need to declare it as a
  subclass.

  ## Limitations 

  LuaBridge does not support:

  - More than 8 parameters on a function or method (although this can be
    increased by adding more `typelist` specializations).
  - Overloaded functions, methods, or constructors.
  - Global variables (variables must be wrapped in a named scope).
  - Automatic conversion between STL container types and Lua tables.
  - Inheriting Lua classes from C++ classes.

  ## Development

  [Github][3] is the new official home for LuaBridge. The old SVN repository is
  deprecated since it is no longer used, or maintained. The original author has
  graciously passed the reins to Vinnie Falco for maintaining and improving the
  project. To obtain the older official releases, checkout the tags from 2.1
  and earlier.

  We welcome contributions to LuaBridge. Feel free to fork the repository and
  issue a Pull Request. All questions, comments, suggestions, and/or proposed
  changes will be handled by the new maintainer.

  ## License

  Copyright (C) 2012, [Vinnie Falco][1] ([e-mail][0]) <br>
  Copyright (C) 2007, Nathan Reed <br>
  
  Portions from The Loki Library: <br>
  Copyright (C) 2001 by Andrei Alexandrescu

  License: The [MIT License][2]

  Older versions of LuaBridge up to and including 0.2 are distributed under the
  BSD 3-Clause License. See the corresponding license file in those versions
  for more details.

  [0]: mailto:vinnie.falco@gmail.com "Vinnie Falco (Email)"
  [1]: http://www.vinniefalco.com "Vinnie Falco"
  [2]: http://www.opensource.org/licenses/mit-license.html "The MIT License"
  [3]: https://github.com/vinniefalco/LuaBridge "LuaBridge"
  [4]: http://lua.org "The Lua Programming Language"
*/

#include <cassert>
#include <string>

namespace luabridge2
{

//==============================================================================
/**
  Provides a scope of implementation specific operations.
*/
class Detail
{
//protected:
public:
  //----------------------------------------------------------------------------
  /**
    Get a table value, bypassing metamethods.
  */  
  static inline void rawgetfield (lua_State* const L, int const index, char const* const key)
  {
    lua_pushstring (L, key);
    if (index < 0)
      lua_rawget (L, index-1);
    else
      lua_rawget (L, index);
  }

  //----------------------------------------------------------------------------
  /**
    Set a table value, bypassing metamethods.
  */  
  static inline void rawsetfield (lua_State* const L, int const index, char const* const key)
  {
    lua_pushstring (L, key);
    lua_insert (L, -2);
    if (index < 0)
      lua_rawset (L, index-1);
    else
      lua_rawset (L, index);
  }

  //----------------------------------------------------------------------------
  /**
    Report an invalid type message.

    This is our version of luaL_typerror, which was removed in Lua 5.2.
  */
  /*
  static int typeError (lua_State* L, int narg, const char *tname)
  {
    const char *msg = lua_pushfstring (L, "%s expected, got %s",
      tname, luaL_typename (L, narg));

    return luaL_argerror (L, narg, msg);
  }
  */

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to report a write error.

    The name of the variable is in the first upvalue.
  */
  static int writeError (lua_State* L)
  {
    if (!lua_isstring (L, lua_upvalueindex (1)))
      throw std::logic_error ("bad upvalue");

    std::string s;

    lua_Debug ar;
    int result = lua_getstack (L, 2, &ar);
    if (result != 0)
    {
      lua_getinfo (L, "Sl", &ar);
      s = ar.short_src;
      if (ar.currentline != -1)
      {
        // poor mans int to string to avoid stringstream.
        lua_pushnumber (L, ar.currentline);
        s = s + ":" + lua_tostring (L, -1) + ": ";
        lua_pop (L, 1);
      }
    }

    s = s + "'" + lua_tostring (L, lua_upvalueindex (1))
          + "' is read-only";

    return luaL_error (L, s.c_str ());
  }
  
  //----------------------------------------------------------------------------
  /**
    Associates classes with unique registry keys.
  */
  template <class T>
  class ClassInfo
  {
  public:
    /**
      Return the registry key associated with the class static table.
    */
    static void* getStaticKey ()
    {
      static char value;
      return &value;
    }

    /**
      Return the registry key associated with the class metatable.
    */
    static void* getClassKey ()
    {
      static char value;
      return &value;
    }

    /**
      Return the registry key associated with the class const metatable.
    */
    static void* getConstKey ()
    {
      static char value;
      return &value;
    }
  };

  //----------------------------------------------------------------------------
  /*
  * Class type checker.  Given the index of a userdata on the stack, makes
  * sure that it's an object of the given classinfo or a subclass thereof.
  * If yes, returns the address of the data; otherwise, throws an error.
  * Works like the luaL_checkudata function.
  */

  static void* checkClass (lua_State* L, int narg, void* metatableKey, bool exact)
  {
    void* p = 0;
    int const index = lua_absindex (L, narg);

    lua_rawgetp (L, LUA_REGISTRYINDEX, metatableKey);
    if (!lua_istable (L, -1))
      throw std::logic_error ("expected table");

    if (!lua_isuserdata (L, index))
    {
      rawgetfield (L, -1, "__type");
      char const* tname = lua_tostring (L, -1);

      char const* msg = lua_pushfstring (L, "%s expected, got %s",
        tname, luaL_typename (L, index));

      if (index > 0)
        luaL_argerror (L, index, msg);
      else
        luaL_error (L, msg);
    }

    // Lookup the metatable of the given userdata
    lua_getmetatable (L, index);

    // If exact match required, simply test for identity.
    if (exact)
    {
      // Ignore "const" for exact tests (which are used for destructors).
      //if (!strncmp (tname, "const ", 6))
      //  tname += 6;

      if (lua_rawequal (L, -1, -2))
      {
        p = lua_touserdata (L, index);
      }
      else
      {
        rawgetfield (L, -1, "__type");
        luaL_argerror (L, index, lua_pushfstring (L,
          "'%s' expected, got '%s'", "typename" , lua_typename (L, lua_type (L, index))));
      }
    }
    else
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
        if (lua_isnil (L, -1))
        {
          // Lookup the __type field of the original metatable, so we can
          // generate an informative error message
          lua_getmetatable(L, index);
          rawgetfield(L, -1, "__type");

          luaL_argerror (L, index, lua_pushfstring (L,
            "'%s' expected, got '%s'", "typename" , lua_tostring (L, -1)));
        }

        // Remove the old metatable from the stack
        lua_remove(L, -2);
      }

      lua_pop (L, 2); // remove the metatables used for comparison
      p = lua_touserdata (L, index);
    }

    return p;
  }

  //----------------------------------------------------------------------------
};

//==============================================================================
 // forward declaration
template <class T>
class class__;

//==============================================================================
/**
  Extract the pointer from a container.

  The default template supports extraction from any shared_ptr compatible
  interface. If you need to use an incompatible container, specialize this
  template for your type and provide the get() function.
*/
template <template <class> class SharedPtr>
struct Container
{
  template <class T>
  static inline T* get (SharedPtr <T> const& p)
  {
    return p.get ();
  }
};

//==============================================================================
/**
  Holds the address of a unique string to identify unregistered classes.
*/
class classinfoBase
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
class classinfo : private classinfoBase
{
public:
  /** Register a class.
  */
  static void registerClass (char const* name)
  {
    assert (!isRegistered ());

    classinfo <T>::s_string = std::string ("const ") + std::string (name);
    classinfo <T>::s_constname = classinfo <T>::s_string.c_str ();
    classinfo <T>::s_name = classinfo <T>::s_constname + 6;
  }

  /** Determine if the class is registered to Lua.
  */
  static inline bool isRegistered ()
  {
    return classinfo <T>::s_name != unregisteredClassName ();
  }

  /** Retrieve the class name.

      @note The class must be registered.
  */
  static inline char const* name ()
  {
    assert (isRegistered ());
    return classinfo <T>::s_name;
  }

  /** Retrieve the class const name.

      @note The class must be registered.
  */
  static inline char const* const_name ()
  {
    assert (isRegistered ());
    return classinfo <T>::s_constname;
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
std::string classinfo <T>::s_string;

template <class T>
char const* classinfo <T>::s_constname = classinfoBase::unregisteredClassName ();

template <class T>
char const* classinfo <T>::s_name = classinfoBase::unregisteredClassName ();

//------------------------------------------------------------------------------
/**
  Container specialization for const types.

  The mapped name is the same.
*/
template <class T>
struct classinfo <T const> : public classinfo <T>
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
struct typelist
{
  /*
  static std::string const tostring ()
  {
    std::string s = ", " + typeid (Head).name ();
    return s;
  }
  */
};

/*
* Type/value list.
*/

template <typename Typelist>
struct typevallist
{
  static std::string const tostring (bool)
  {
    return "";
  }
};

template <typename Head, typename Tail>
struct typevallist <typelist <Head, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head hd_, typevallist <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name ();

    return s + typevallist <Tail>::tostring (true);
  }
};

// Specializations of type/value list for head types that are references and
// const-references.  We need to handle these specially since we can't count
// on the referenced object hanging around for the lifetime of the list.

template <typename Head, typename Tail>
struct typevallist <typelist <Head&, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head& hd_, typevallist <Tail> const& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + "&";

    return s + typevallist <Tail>::tostring (true);
  }
};

template <typename Head, typename Tail>
struct typevallist <typelist <Head const&, Tail> >
{
  Head hd;
  typevallist <Tail> tl;

  typevallist (Head const& hd_, const typevallist <Tail>& tl_)
    : hd (hd_), tl (tl_)
  {
  }

  static std::string const tostring (bool comma = false)
  {
    std::string s;

    if (comma)
      s = ", ";

    s = s + typeid (Head).name () + " const&";

    return s + typevallist <Tail>::tostring (true);
  }
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

template <typename MemFn>
struct FunctionPointer {};

/* Ordinary function pointers. */

template <typename Ret>
struct FunctionPointer <Ret (*) ()>
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
struct FunctionPointer <Ret (*) (P1)>
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
struct FunctionPointer <Ret (*) (P1, P2)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3, P4)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6, P7)>
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
struct FunctionPointer <Ret (*) (P1, P2, P3, P4, P5, P6, P7, P8)>
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

template <class T, typename Ret>
struct FunctionPointer <Ret (T::*) ()>
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

template <class T, typename Ret, typename P1>
struct FunctionPointer <Ret (T::*) (P1)>
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

template <class T, typename Ret, typename P1, typename P2>
struct FunctionPointer <Ret (T::*) (P1, P2)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3>
struct FunctionPointer <Ret (T::*) (P1, P2, P3)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7)>
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

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8)>
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

template <class T, typename Ret>
struct FunctionPointer <Ret (T::*) () const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef nil params;
  static Ret call (T const* const obj, Ret (T::*fp) () const,
    const typevallist<params> &tvl)
  {
    (void)tvl;
    return (obj->*fp)();
  }
};

template <class T, typename Ret, typename P1>
struct FunctionPointer <Ret (T::*) (P1) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1> params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2>
struct FunctionPointer <Ret (T::*) (P1, P2) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2> > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3>
struct FunctionPointer <Ret (T::*) (P1, P2, P3) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3> > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4> > > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3, P4) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4,
    typelist<P5> > > > > params;
  static Ret call (T const* const obj, Ret (T::*fp) (P1, P2, P3, P4, P5) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6> > > > > > params;
  static Ret call (T const* const obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7> > > > > > > params;
  static Ret call (T const* const obj,
    Ret (T::*fp) (P1, P2, P3, P4, P5, P6, P7) const,
    const typevallist<params> &tvl)
  {
    return (obj->*fp)(tvl.hd, tvl.tl.hd, tvl.tl.tl.hd, tvl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.hd, tvl.tl.tl.tl.tl.tl.hd,
      tvl.tl.tl.tl.tl.tl.tl.hd);
  }
};

template <class T, typename Ret, typename P1, typename P2, typename P3,
  typename P4, typename P5, typename P6, typename P7, typename P8>
struct FunctionPointer <Ret (T::*) (P1, P2, P3, P4, P5, P6, P7, P8) const>
{
  static const bool mfp = true;
  static const bool const_mfp = true;
  typedef T classtype;
  typedef Ret resulttype;
  typedef typelist<P1, typelist<P2, typelist<P3, typelist<P4, typelist<P5,
    typelist<P6, typelist<P7, typelist<P8> > > > > > > > params;
  static Ret call (T const* const obj,
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

/** Constructor generators.

    These templates call operator new with the contents of a type/value
    list passed to the constructor with up to 8 parameters. Two versions
    of call() are provided. One performs a regular new, the other performs
    a placement new.
*/
template <class T, typename Typelist>
struct constructor {};

template <class T>
struct constructor <T, nil>
{
  static T* call (typevallist <nil> const&)
  {
    return new T;
  }
  static T* call (void* mem, typevallist <nil> const&)
  {
    return new (mem) T;
  }
};

template <class T, class P1>
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

template <class T, class P1, class P2>
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

template <class T, class P1, class P2, class P3>
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

template <class T, class P1, class P2, class P3, class P4>
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

template <class T, class P1, class P2, class P3, class P4,
  class P5>
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

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6>
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

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7>
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

template <class T, class P1, class P2, class P3, class P4,
  class P5, class P6, class P7, class P8>
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

struct detail : Detail
{

//------------------------------------------------------------------------------
/**
  Custom __index metamethod for C++ classes.

  If the given key is not found, the search will be delegated up the parent
  hierarchy.
*/
static int indexMetaMethod (lua_State* L)
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
static int newindexMetaMethod (lua_State* L)
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
        "attempt to set '%s', which isn't a property", lua_tostring(L, 2));
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
        "attempt to set '%s', which isn't a property", lua_tostring (L, 2));
    }
    lua_remove (L, -2);
  }

  return result;
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
    Get a pointer to the class from the Lua stack.
  */
  template <class T>
  static T* get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index,
      Detail::ClassInfo <T>::getClassKey (), false);
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
    void* const p = detail::checkClass (L, index,
      Detail::ClassInfo <T>::getConstKey (), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    return ud->getConst <T> (L);
  }

public:
  virtual ~Userdata () { }

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

  //----------------------------------------------------------------------------
  /**
    Get a pointer to the class.

    @note The lua_State is provided for diagnostics.
  */
  template <class T>
  T* get (lua_State* L)
  {
    //assert (classinfo <T>::name () == getName ());
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
    //assert (classinfo <T>::name () == getName ());
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
  char const* getName () const { return classinfo <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByValue (T const& t) : m_t (t)
  {
  }

  static void push (lua_State* L, T const& t)
  {
    void* const p = lua_newuserdata (L, sizeof (UserdataByValue <T>));
    lua_rawgetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getClassKey ());
    if (!lua_istable (L, -1))
      luaL_error (L, "missing metatable for '%s'", typeid (T).name ());
    new (p) UserdataByValue <T> (t);
    lua_setmetatable (L, -2);
  }

private:
  void* getPointer (lua_State*)
  {
    return &m_t;
  }

  void const* getConstPointer (lua_State*)
  {
    return &m_t;
  }

private:
  T m_t;
};

//------------------------------------------------------------------------------
/**
  Class passed by pointer.

  The object lifetime is fully managed by C++.
*/
template <class T>
class UserdataByReference : public Userdata
{
public:
  char const* getName () const { return classinfo <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByReference (T& t) : m_t (t)
  {
  }

  UserdataByReference (UserdataByReference <T> const& other) : m_t (other.m_t)
  {
  }

  template <class U>
  UserdataByReference (UserdataByReference <U> const& other) : m_t (other.m_t)
  {
  }

  ~UserdataByReference ()
  {
  }

  static void push (lua_State* L, T& t)
  {
    void* const p = lua_newuserdata (L, sizeof (UserdataByReference <T>));
    lua_rawgetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getClassKey ());
    if (!lua_istable (L, -1))
      luaL_error (L, "missing metatable for '%s'", typeid (T).name ());
    new (p) UserdataByReference <T> (t);
    lua_setmetatable (L, -2);
  }

private:
  void* getPointer (lua_State*)
  {
    return &m_t;
  }

  void const* getConstPointer (lua_State*)
  {
    return &m_t;
  }

private:
  UserdataByReference <T>& operator= (UserdataByReference <T> const& other);

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
  char const* getName () const { return classinfo <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByConstReference (T const& t) : m_t (t)
  {
  }

  UserdataByConstReference (UserdataByConstReference <T> const& other) : m_t (other.m_t)
  {
  }

  template <class U>
  UserdataByConstReference (UserdataByConstReference <U> const& other) : m_t (other.m_t)
  {
  }

  static void push (lua_State* L, T const& t)
  {
    void* const p = lua_newuserdata (L, sizeof (UserdataByConstReference <T>));
    lua_rawgetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getConstKey ());
    if (!lua_istable (L, -1))
      luaL_error (L, "missing const metatable for '%s'", typeid (T).name ());
    new (p) UserdataByConstReference <T> (t);
    lua_setmetatable (L, -2);
  }

private:
  void* getPointer (lua_State* L)
  {
    luaL_argerror (L, "illegal non-const use of '%s'", getName ());
    return 0; // never gets here
  }

  void const* getConstPointer ()
  {
    return &m_t;
  }

private:
  UserdataByConstReference <T>& operator= (UserdataByConstReference <T> const& other);

  T const& m_t;
};

//------------------------------------------------------------------------------
/**
  Class passed by container.

  The object lifetime is managed by the container.

  @note Container must implement a strict subset of shared_ptr.
*/
template <class T, template <class> class SharedPtr>
class UserdataBySharedPtr : public Userdata
{
public:
  char const* getName () const { return classinfo <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataBySharedPtr (T* const t) : m_p (t)
  {
  }

  template <class U>
  explicit UserdataBySharedPtr (U* const u) : m_p (u)
  {
  }

  static void push (lua_State* L, T* const t)
  {
    void* const p = lua_newuserdata (L, sizeof (UserdataBySharedPtr <T, SharedPtr>));
    lua_rawgetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getClassKey ());
    if (!lua_istable (L, -1))
      luaL_error (L, "missing metatable for '%s'", typeid (T).name ());
    new (p) UserdataBySharedPtr <T, SharedPtr> (t);
    lua_setmetatable (L, -2);
  }

  static SharedPtr <T> get (lua_State* L, int index)
  {
    void* const p = detail::checkClass (L, index, Detail::ClassInfo <T>::getClassKey (), false);
    Userdata* const pb = static_cast <Userdata*> (p);
    UserdataBySharedPtr <T, SharedPtr>* ud =
      reinterpret_cast <UserdataBySharedPtr <T, SharedPtr>*> (pb);
    if (ud == 0)
      luaL_argerror (L, index, lua_pushfstring (L, "'%s' expected, got '%s'",
        typeid (UserdataBySharedPtr <T, SharedPtr>).name (), pb->getTypename ()));
    return ud->m_p;
  }

private:
  void* getPointer (lua_State*)
  {
    return Container <SharedPtr>::get (m_p);
  }

  void const* getConstPointer (lua_State*)
  {
    return Container <SharedPtr>::get (m_p);
  }

private:
  SharedPtr <T> m_p;
};

//------------------------------------------------------------------------------
/**
  Class passed by const container.

  The object lifetime is managed by the container.

  @note Container must implement a strict subset of shared_ptr.
*/
template <class T, template <class> class SharedPtr>
class UserdataByConstSharedPtr : public Userdata
{
public:
  char const* getName () const { return classinfo <T>::name (); }
  char const* getTypename () const { return typeid (*this).name (); }

  explicit UserdataByConstSharedPtr (T const* const t) : m_p (t)
  {
  }

  template <class U>
  explicit UserdataByConstSharedPtr (U const* const u) : m_p (u)
  {
  }

  static void push (lua_State* L, T const* const t)
  {
    void* const p = lua_newuserdata (L, sizeof (UserdataByConstSharedPtr <T, SharedPtr>));
    lua_rawgetp (L, LUA_REGISTRYINDEX, Detail::ClassInfo <T>::getConstKey ());
    if (!lua_istable (L, -1))
      luaL_error (L, "missing const metatable for '%s'", typeid (T).name ());
    new (p) UserdataByConstSharedPtr <T, SharedPtr> (t);
    lua_setmetatable (L, -2);
  }

private:
  void* getPointer (lua_State* L)
  {
#if LUABRIDGE_STRICT_CONST
    luaL_error (L, "illegal non-const use of '%s'", getName ());
    return 0; // never gets here
#else
    (void)L;
    void const* p = Container <SharedPtr>::get (m_p);
    return const_cast <void*> (p);
#endif
  }

  void const* getConstPointer (lua_State*)
  {
    return Container <SharedPtr>::get (m_p);
  }

private:
  SharedPtr <T const> m_p;
};

//==============================================================================
/**
  Lua stack objects with value semantics.

  @note T must be copy-constructible.
*/
template <class T>
struct Stack
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
template <class T>
struct Stack <T*>
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
template <class T>
struct Stack <T* const>
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
template <class T>
struct Stack <T const*>
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
template <class T>
struct Stack <T const* const>
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
template <class T>
struct Stack <T&>
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
template <class T>
struct Stack <T const&>
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
  Lua stack objects with shared_ptr-like container semantics.
*/
template <class T, template <class> class SharedPtr>
struct Stack <SharedPtr <T> >
{
  static void push (lua_State* L, SharedPtr <T> p)
  {
    T* const t = Container <SharedPtr>::get (p);
    UserdataBySharedPtr <T, SharedPtr>::push (L, t);
  }

  static SharedPtr <T> get (lua_State* L, int index)
  {
    return UserdataBySharedPtr <T, SharedPtr>::get (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  Lua stack objects with const shared_ptr-like semantics.
*/
template <class T, template <class> class SharedPtr>
struct Stack <SharedPtr <T const> >
{
  static void push (lua_State* L, SharedPtr <T const> p)
  {
    T const* const t = Container <SharedPtr>::get (p);
    UserdataByConstSharedPtr <T, SharedPtr>::push (L, t);
  }
  static SharedPtr <T const> get (lua_State* L, int index)
  {
    return UserdataByConstSharedPtr <T, SharedPtr>::get (L, index);
  }
};

//------------------------------------------------------------------------------
/**
  void return values.
*/
template <>
struct Stack <void>
{
  void get (lua_State*, int)
  {
  }
};

//------------------------------------------------------------------------------

// int
template <> struct Stack <
  int > { static void push (lua_State* L,
  int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  int get (lua_State* L, int index) { return static_cast <
  int > (luaL_checknumber (L, index)); } };

// unsigned int
template <> struct Stack <
  unsigned int > { static void push (lua_State* L,
  unsigned int value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned int get (lua_State* L, int index) { return static_cast <
  unsigned int > (luaL_checknumber (L, index)); } };

// unsigned char
template <> struct Stack <
  unsigned char > { static void push (lua_State* L,
  unsigned char value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned char get (lua_State* L, int index) { return static_cast <
  unsigned char > (luaL_checknumber (L, index)); } };

// short
template <> struct Stack <
  short > { static void push (lua_State* L,
  short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  short get (lua_State* L, int index) { return static_cast <
  short > (luaL_checknumber (L, index)); } };

// unsigned short
template <> struct Stack <
  unsigned short > { static void push (lua_State* L,
  unsigned short value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned short get (lua_State* L, int index) { return static_cast <
  unsigned short > (luaL_checknumber (L, index)); } };

// long
template <> struct Stack <
  long > { static void push (lua_State* L,
  long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  long get (lua_State* L, int index) { return static_cast <
  long > (luaL_checknumber (L, index)); } };

// unsigned long
template <> struct Stack <
  unsigned long > { static void push (lua_State* L,
  unsigned long value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  unsigned long get (lua_State* L, int index) { return static_cast <
  unsigned long > (luaL_checknumber (L, index)); } };

// float
template <> struct Stack <
  float > { static void push (lua_State* L,
  float value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  float get (lua_State* L, int index) { return static_cast <
  float > (luaL_checknumber (L, index)); } };

// double
template <> struct Stack <
  double > { static void push (lua_State* L,
  double value) { lua_pushnumber (L, static_cast <lua_Number> (value)); } static
  double get (lua_State* L, int index) { return static_cast <
  double > (luaL_checknumber (L, index)); } };

// bool
template <>
struct Stack <bool>
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
struct Stack <char>
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
struct Stack <char const*>
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
struct Stack <std::string>
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
struct Stack <std::string const&>
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
    : typevallist <typelist <Head, Tail> > (Stack <Head>::get (L, start),
                                            arglist <Tail, start + 1> (L))
  {
  }
};

//==============================================================================
/**
  lua_CFunction to construct a class object.

  These are registered to Lua as global functions with the name of the class,
  with the appropriate metatable passed as an upvalue.
*/
template <class T, template <class> class SharedPtr, typename Params>
int ctorProxy (lua_State* L)
{
  arglist <Params, 2> args (L);
  T* const t = constructor <T, Params>::call (args);
  UserdataBySharedPtr <T, SharedPtr>::push (L, t);
  return 1;
}

//------------------------------------------------------------------------------
/**
  lua_CFunction to destroy a class object.

  This is used for the __gc metamethod.

  @note The expected class name is passed as an upvalue so that we can
        ensure that we are destroying the right kind of object.
*/
template <class T>
int dtorProxy (lua_State* L)
{
  void* const p = detail::checkClass (
    L, 1, lua_tostring (L, lua_upvalueindex (1)), true);
  Userdata* const ud = static_cast <Userdata*> (p);
  ud->~Userdata ();
  return 0;
}

//------------------------------------------------------------------------------
/**
  lua_CFunction to call a class member function with a return value.

  The argument list contains the 'this' pointer followed by the method
  arguments.

  @note The expected class name is in upvalue 1, and the member function
        pointer is in upvalue 2.
*/
template <typename MemFn,
          typename RetVal = typename FunctionPointer <MemFn>::resulttype>
struct methodProxy
{
  typedef typename FunctionPointer <MemFn>::classtype T;
  typedef typename FunctionPointer <MemFn>::params params;

  static int func (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T* const t = ud->get <T> (L);
    MemFn fp = *static_cast <MemFn*> (
      lua_touserdata (L, lua_upvalueindex (2)));
    arglist <params, 2> args(L);
    Stack <RetVal>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
    return 1;
  }

  // const class member functions
  static int const_func (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T const* const t = ud->getConst <T> (L);
    MemFn fp = *static_cast <MemFn*> (
      lua_touserdata (L, lua_upvalueindex (2)));
    arglist <params, 2> args(L);
    Stack <RetVal>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
    return 1;
  }
};

//------------------------------------------------------------------------------
/**
  lua_CFunction to call a class member function with no return value.

  The argument list contains the 'this' pointer followed by the method
  arguments.

  @note The expected class name is in upvalue 1, and the member function
        pointer is in upvalue 2.
*/
template <typename MemFn>
struct methodProxy <MemFn, void>
{
  typedef typename FunctionPointer <MemFn>::classtype T;
  typedef typename FunctionPointer <MemFn>::params params;

  static int func (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T* const t = ud->get <T> (L);
    MemFn fp = *static_cast <MemFn*> (
      lua_touserdata (L, lua_upvalueindex (2)));
    arglist <params, 2> args (L);
    FunctionPointer <MemFn>::call (t, fp, args);
    return 0;
  }

  // const class member functions
  static int const_func (lua_State* L)
  {
    void* const p = detail::checkClass (
      L, 1, lua_tostring (L, lua_upvalueindex (1)), false);
    Userdata* const ud = static_cast <Userdata*> (p);
    T const* const t = ud->getConst <T> (L);
    MemFn fp = *static_cast <MemFn*> (
      lua_touserdata (L, lua_upvalueindex (2)));
    arglist <params, 2> args (L);
    FunctionPointer <MemFn>::call (t, fp, args);
    return 0;
  }
};

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
  explicit class__ (lua_State *L_) : scope (L_, classinfo <T>::name ())
  {
    assert (classinfo <T>::isRegistered ());
  }

  //----------------------------------------------------------------------------
  class__ (lua_State *L_, char const *name_) : scope(L_, name_)
  {
    assert (!classinfo <T>::isConst ());
    classinfo <T>::registerClass (name_);

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
    assert (!classinfo <T>::isConst ());
    classinfo <T>::registerClass (name_);

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
  template <typename MemFn, template <class> class SharedPtr>
  class__ <T>& constructor ()
  {
    findStaticTable (L, name.c_str());
    luaL_getmetatable(L, name.c_str());
    lua_pushcclosure (L,
      &ctorProxy <T, SharedPtr, typename FunctionPointer <MemFn>::params>, 1);
    rawsetfield(L, -2, "__call");
    lua_pop (L, 1);
    return *this;
  }

  //----------------------------------------------------------------------------
  /*
  * Perform method registration in a class.  The method proxies are all
  * registered as values in the class's metatable, which is searched by the
  * indexMetaMethod function we've installed as __index metamethod.
  */
  template <typename MemFn>
  class__ <T>& method (char const* name, MemFn fp)
  {
    assert (FunctionPointer <MemFn>::mfp);
    std::string metatable_name = this->name;

    #ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4127) // constant conditional expression
    #endif
    if (FunctionPointer <MemFn>::const_mfp)
      metatable_name.insert (0, "const ");
    #ifdef _MSC_VER
    #pragma warning (pop)
    #endif

    luaL_getmetatable (L, metatable_name.c_str ());
    lua_pushstring (L, metatable_name.c_str ());
    void* const v = lua_newuserdata (L, sizeof (MemFn));
    memcpy (v, &fp, sizeof (MemFn));

    #ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable: 4127) // constant conditional expression
    #endif
    if (FunctionPointer <MemFn>::const_mfp)
    #if LUABRIDGE_STRICT_CONST
      lua_pushcclosure (L, &methodProxy <MemFn>::const_func, 2);
    #else
      lua_pushcclosure (L, &methodProxy <MemFn>::func, 2);
    #endif
    else
      lua_pushcclosure (L, &methodProxy <MemFn>::func, 2);
    #ifdef _MSC_VER
    #pragma warning (pop)
    #endif

    rawsetfield (L, -2, name);
    lua_pop (L, 1);
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
    luaL_getmetatable (L, this->name.c_str());
    std::string cname = "const " + this->name;
    luaL_getmetatable (L, cname.c_str());
    rawgetfield (L, -2, "__propget");
    rawgetfield (L, -2, "__propget");
    lua_pushstring (L, cname.c_str ());
    void* const v = lua_newuserdata(L, sizeof (U T::*));
    memcpy (v, &mp, sizeof (U T::*));
    lua_pushcclosure (L, &propgetProxy <T, U>, 2);
    lua_pushvalue (L, -1);
    rawsetfield (L, -3, name);
    rawsetfield (L, -3, name);
    lua_pop (L, 4);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read-only property using a get function.
  */
  template <typename U>
  class__ <T>& property_ro (char const* name, U (T::* get) () const)
  {
    luaL_getmetatable (L, this->name.c_str ());
    /** @todo Why not use classinfo <T>::const_name () ? */
    std::string cname = "const " + this->name;
    luaL_getmetatable (L, cname.c_str ());
    rawgetfield (L, -2, "__propget");
    rawgetfield (L, -2, "__propget");
    lua_pushstring (L, cname.c_str ());
    typedef U (T::*MemFn) () const;
    void* const v = lua_newuserdata (L, sizeof (MemFn));
    memcpy (v, &get, sizeof (MemFn));
    lua_pushcclosure (L, &methodProxy <MemFn>::const_func, 2);
    lua_pushvalue (L, -1);
    rawsetfield (L, -3, name);
    rawsetfield (L, -3, name);
    lua_pop (L, 4);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read/write data member.
  */
  template <class U>
  class__ <T>& property_rw (char const *name, U T::* mp)
  {
    property_ro <U> (name, mp);
    luaL_getmetatable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushstring (L, this->name.c_str());
    void* v = lua_newuserdata (L, sizeof (U T::*));
    memcpy (v, &mp, sizeof (U T::*));
    lua_pushcclosure (L, &propsetProxy <T, U>, 2);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a read/write property using get/set functions.
  */
  template <class U>
  class__ <T>& property_rw (char const* name, U (T::* get) () const, void (T::* set) (U))
  {
    property_ro <U> (name, get);
    luaL_getmetatable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushstring (L, this->name.c_str ());
    typedef void (T::* MemFn) (U);
    void* const v = lua_newuserdata (L, sizeof (MemFn));
    memcpy (v, &set, sizeof (MemFn));
    lua_pushcclosure (L, &methodProxy <MemFn>::func, 2);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

  //----------------------------------------------------------------------------

  // Static method registration
  template <typename MemFn>
  class__ <T>& static_method (char const *name, MemFn fp)
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

//==============================================================================

/**
  A complete rewrite of LuaBridge.

  This re-implementation builds a run-time structure that describes all of
  the registrations. This structure can be "replayed" into a lua_State to
  produce all the registry entries.

  Since we have the full run-time structure available, we can implement the
  __tostring metamethod to allow reflection of all registered classes from
  Lua scripts.

  Namespaces are nested logically according to the way they are registered.
  The resulting names are available to the Lua the same way. Class types appear
  as tables inside the corresponding namespace instead of only in the registry.

  Property concept:

    Looks like a variable or data member to Lua, but is implemented using
      get and set functions on the C++ side.
*/

//==============================================================================

/**
  Simple intrusive linked list.
*/
template <class T>
class List
{
public:
  class Node
  {
  public:
    inline T* const next () const
    {
      return m_next;
    }

  private:
    friend class List <T>;

    T* m_next;
  };

public:
  List () : m_head (0), m_tail (0)
  {
  }

  inline bool const empty () const
  {
    return m_head == 0;
  }

  inline T* const head () const
  {
    return m_head;
  }

  void append (T* const t)
  {
    t->m_next = 0;

    if (m_head != 0)
      m_tail->m_next = t;
    else
      m_head = t;

    m_tail = t;
  }

private:
  T* m_head;
  T* m_tail;
};

//==============================================================================

/**
  A generic C++ name registerable to Lua.
*/
class Symbol : public List <Symbol>::Node
{
protected:
  Symbol (char const* name, Symbol* parent)
    : m_name (name)
    , m_parent (parent)
  {
  }

public:
  virtual ~Symbol ()
  {
  }

  /** Return the Lua name of this symbol.
  */
  std::string const& getName () const
  {
    return m_name;
  }

  /** Return a formatted signature for this symbol.
  */
  virtual std::string const tostring (bool includeChildren) const = 0;

  /** Register this function into the lua_State.
  */
  virtual void addToState (lua_State* L) = 0;

private:
  Symbol (Symbol const&);
  Symbol& operator= (Symbol const&);

protected:
  std::string const m_name;
  Symbol* const m_parent;
};

//==============================================================================
/**
  A namespace registerable to Lua.
*/
class Namespace : public Symbol, protected Detail
{
private:
  Namespace (Namespace const&);
  Namespace& operator= (Namespace const&);

  List <Symbol> m_symbols;

private:
  //----------------------------------------------------------------------------
  /**
    lua_CFunction to get a variable.

    This is also used for static data members of classes
  */
  template <class T>
  static int vargetProxy (lua_State* L)
  {
    #if LUABRIDGE_USE_APICHECK
    if (!lua_islightuserdata (L, lua_upvalueindex (1)))
      throw std::logic_error ("bad upvalue");
    #endif

    T* const data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
    Stack <T>::push (L, *data);
    return 1;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to set a variable.

    This is also used for static data members of classes.
  */

  template <class T>
  static int varsetProxy (lua_State* L)
  {
    #if LUABRIDGE_USE_APICHECK
    if (!lua_islightuserdata (L, lua_upvalueindex (1)))
      throw std::logic_error ("bad upvalue");
    #endif

    T* const data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
    *data = Stack <T>::get (L, 1);
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a function with a return value.

    This is also used for property get functions.
  */
  template <typename Function,
            typename Retval = typename FunctionPointer <Function>::resulttype>
  struct functionProxy
  {
    typedef typename FunctionPointer <Function>::params params;
    static int f (lua_State* L)
    {
      #if LUABRIDGE_USE_APICHECK
      if (!lua_islightuserdata (L, lua_upvalueindex (1)))
        throw std::logic_error ("bad upvalue");
      #endif

      Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params> args (L);
      Stack <Retval>::push (L, FunctionPointer <Function>::call (fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction to call a function with no return value.

    This is also used for property get functions.
  */
  template <typename Function>
  struct functionProxy <Function, void>
  {
    typedef typename FunctionPointer <Function>::params params;
    static int f (lua_State* L)
    {
      #if LUABRIDGE_USE_APICHECK
      if (!lua_islightuserdata (L, lua_upvalueindex (1)))
        throw std::logic_error ("bad upvalue");
      #endif

      Function fp = reinterpret_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params> args (L);
      FunctionPointer <Function>::call (fp, args);
      return 0;
    }
  };

  //----------------------------------------------------------------------------
  /**
    __print metamethod for a Namespace.
  */
  static int tostringMetaMethod (lua_State* L)
  {
    if (!lua_islightuserdata (L, lua_upvalueindex (1)))
      throw std::logic_error ("bad upvalue");
    
    Namespace const* const n = static_cast <Namespace*> (
      lua_touserdata (L, lua_upvalueindex (1)));

    std::string const s = n->tostring (true);

    lua_pushstring (L, s.c_str ());

    return 1;
  }

  //----------------------------------------------------------------------------
  /**
    __index metamethod for a Namespace or static Class table.

    This implements functions, variables, and properties. Functions go into the
    metatable, while variables and properties are located in the __propget table.
  */
  static int indexMetaMethod (lua_State* L)
  {
    int result = 0;

    lua_getmetatable (L, 1);                  // push metatable of arg1

    for (;;)
    {
      lua_pushvalue (L, 2);                   // push key arg2
      lua_rawget (L, -2);                     // lookup key in metatable
      if (lua_isnil (L, -1))                  // not found
      {
        lua_pop (L, 1);                       // discard nil
        rawgetfield (L, -1, "__propget");     // lookup __propget in metatable
        if (lua_istable (L, -1))              // ensure __propget is a table
        {
          lua_pushvalue (L, 2);               // push key arg2
          lua_rawget (L, -2);                 // lookup key in __propget
          lua_remove (L, -2);                 // discard __propget
          if (lua_iscfunction (L, -1))        // ensure value is a C function
          {
            lua_remove (L, -2);               // discard metatable
            lua_pushvalue (L, 1);             // push arg1
            lua_call (L, 1, 1);               // call cfunction
            result = 1;
            break;
          }
          else if (lua_isnil (L, -1))         // not found
          {
            lua_pop (L, 1);                   // discard nil and fall through
          }
          else
          {
            lua_pop (L, 2);                   // discard unknown and metatable

            throw std::logic_error ("expected cfunction");
          }
        }
        else
        {
          lua_pop (L, 2);

          throw std::logic_error ("expected table '__propget'");
        }
      }
      else if (lua_istable (L, -1) || lua_iscfunction (L, -1))
      {
        // found
        lua_remove (L, -2);
        result = 1;
        break;
      }
      else
      {
        lua_pop (L, 2);

        throw std::logic_error ("expected table or function");
      }

      // Repeat the lookup in the __parent metafield,
      // or return nil if the field doesn't exist.
      rawgetfield (L, -1, "__parent");
      if (lua_istable (L, -1))
      {
        // Remove metatable and repeat the search in __parent.
        lua_remove (L, -2);
      }
      else if (lua_isnil (L, -1))
      {
        result = 1;
        break;
      }
      else
      {
        lua_pop (L, 2);

        throw std::logic_error ("expected table '__parent'");
      }
    }

    return result;
  }

  //----------------------------------------------------------------------------
  /**
    __newindex metamethod for a Namespace.

    This implements assignment to variables and properties, as well as
    static data members and static properties of classes. Everything goes
    into the __propset table of the metatable.
  */
  static int newindexMetaMethod (lua_State* L)
  {
    int result = 0;

    lua_getmetatable (L, 1);                  // push metatable of arg1
    assert (lua_rawequal (L, -1, 1));
    rawgetfield (L, -1, "__propset");         // lookup __propset in metatable
    lua_remove (L, -2);                       // discard metatable
    if (lua_istable (L, -1))                  // ensure __propset is a table
    {
      lua_pushvalue (L, 2);                   // push key arg2
      lua_rawget (L, -2);                     // lookup key in __propset
      if (lua_iscfunction (L, -1))            // ensure value is a cfunction
      {
        lua_pushvalue (L, 3);                 // push new value arg3
        lua_call (L, 1, 0);                   // call cfunction
        result = 0;
      }
      else if (lua_isnil (L, -1))             // not found
      {
        lua_pop (L, 1);

        result = luaL_error (L,
          "no writable variable '%s'", lua_tostring (L, 2));
      }
      else
      {
        lua_pop (L, 1);

        // We only put cfunctions into __propget.
        throw std::logic_error ("not a cfunction");
      }
    }
    else
    {
      lua_pop (L, 1);

      // __propset is missing, or not a table.
      throw std::logic_error ("missing __propset table");
    }

    return result;
  }

  //----------------------------------------------------------------------------
  /**
    Find a symbol in this namespace.
  */
  Symbol* findSymbol (char const* name)
  {
    Symbol* result = 0;

    for (Symbol* s = m_symbols.head (); s; s = s->next ())
    {
      if (s->getName () == name)
      {
        result = s;
        break;
      }
    }

    return result;
  }

private:
  //============================================================================
  /**
    A free function registerable to Lua.
  */
  template <class FP>
  class Function : public Symbol
  {
  private:
    Function (Function const&);
    Function& operator= (Function const&);

    typedef typename FunctionPointer <FP>::resulttype ReturnType;
    typedef typename FunctionPointer <FP>::params Params;
    typedef typevallist <Params> TypeList;

    FP const m_fp;

  public:
    //--------------------------------------------------------------------------
    /**
      Create a Function.
    */
    Function (char const* name, FP fp, Symbol* parent)
      : Symbol (name, parent)
      , m_fp (fp)
    {
    }

    //--------------------------------------------------------------------------
    /**
      Return the declaration string.
    */
    std::string const tostring (bool) const
    {
      std::string s = typeid (ReturnType).name ();

      s = s + " " + m_name + " (" + TypeList::tostring (false) + ")";

      return s;
    }

    //--------------------------------------------------------------------------
    /**
      Add this function to the lua_State.
    */
    void addToState (lua_State* L)
    {
      // The parent table must be at the top of the Lua stack.
      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      lua_pushlightuserdata (L, m_fp);
      lua_pushcclosure (L, &functionProxy <FP>::f, 1);
      rawsetfield (L, -2, m_name.c_str ());
    }

  };

  //============================================================================
  /**
    A global variable registerable to Lua.
  */
  template <class T>
  class Variable : public Symbol
  {
  public:
    Variable (char const* name, T* pt, bool isWritable, Symbol* parent)
      : Symbol (name, parent)
      , m_pt (pt)
      , m_isWritable (isWritable)
    {
      if (m_parent == 0)
        throw std::logic_error ("illegal variable in global namespace");
    }

    //--------------------------------------------------------------------------
    /**
      Return the declaration string.
    */
    std::string const tostring (bool) const
    {
      std::string s = std::string (typeid (T).name ()) + " " + m_name;

      if (!m_isWritable)
        s = s + " (read-only)";

      return s;
    }

    //--------------------------------------------------------------------------
    /**
      Add this variable to the lua_State.
    */
    void addToState (lua_State* L)
    {
      // The parent table must be at the top of the Lua stack.
      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      rawgetfield (L, -1, "__propget");

      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      lua_pushlightuserdata (L, m_pt);
      lua_pushcclosure (L, &vargetProxy <T>, 1);
      rawsetfield (L, -2, m_name.c_str ());
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");

      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      if (m_isWritable)
      {
        lua_pushlightuserdata (L, m_pt);
        lua_pushcclosure (L, &varsetProxy <T>, 1);
      }
      else
      {
        lua_pushstring (L, m_name.c_str ());
        lua_pushcclosure (L, &writeError, 1);
      }

      rawsetfield (L, -2, m_name.c_str ());
      lua_pop (L, 1);
    }

  private:
    T* const m_pt;
    bool const m_isWritable;
  };

  //============================================================================
  /**
    A global property registerable to Lua.
  */
  template <class T>
  class Property : public Symbol
  {
  public:
    //--------------------------------------------------------------------------
    /**
      Create a property.

      If the set function is null, the property is read-only.
    */
    Property (char const* name, T (*get) (), void (*set)(T), Symbol* parent)
      : Symbol (name, parent)
      , m_get (get)
      , m_set (set)
    {
      if (m_parent == 0)
        throw std::logic_error ("illegal property in global namespace");
    }

    //--------------------------------------------------------------------------
    /**
      Return the declaration string.
    */
    std::string const tostring (bool) const
    {
      std::string s = std::string (typeid (T).name ()) + " " + m_name;

      if (m_set == 0)
        s = s + " (read-only)";

      return s;
    }

    //--------------------------------------------------------------------------
    /**
      Add this property to the lua_State.
    */
    void addToState (lua_State* L)
    {
      // The parent table must be at the top of the Lua stack.
      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      rawgetfield (L, -1, "__propget");

      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      lua_pushlightuserdata (L, m_get);
      lua_pushcclosure (L, &functionProxy <T (*) (void)>::f, 1);
      rawsetfield (L, -2, m_name.c_str ());
      lua_pop (L, 1);

      rawgetfield (L, -1, "__propset");

      if (!lua_istable (L, -1))
        throw std::logic_error ("expected table");

      if (m_set != 0)
      {
        lua_pushlightuserdata (L, m_set);
        lua_pushcclosure (L, &functionProxy <void (*) (T)>::f, 1);
      }
      else
      {
        lua_pushstring (L, m_name.c_str ());
        lua_pushcclosure (L, &writeError, 1);
      }

      rawsetfield (L, -2, m_name.c_str ());
      lua_pop (L, 1);
    }

  private:
    T (*const m_get)();
    void (*const m_set)(T);
  };

  //============================================================================
  /**
    A class T registerable to Lua.

    Classes are implemented using three tables:
    
    - A static table for holding the type information and static functions,
      static data, and static properties.

    - A metatable for holding member functions, data members, and property
      members.

    - A const metatable for holding const member functions, read only data
      members, and read only properties.
  */
  template <class T>
  class Class : public Symbol
  {
  private:
    void* const m_baseStaticKey;
    void* const m_baseClassKey;
    void* const m_baseConstKey;
    List <Symbol> m_symbols;

  private:
    //==========================================================================
    /**
      A static class data member.
    */
    template <class T>
    class StaticData : public Variable <T>
    {
    public:
      StaticData (char const* name, T* pt, bool isWritable, Symbol* parent)
        : Variable <T> (name, pt, isWritable, parent)
      {
      }

      std::string const tostring (bool includeChildren) const
      {
        std::string s = "static " + Variable <T>::tostring (includeChildren);

        return s;
      }
    };

    //==========================================================================
    /**
      A static class property.
    */
    template <class T>
    class StaticProperty : public Property <T>
    {
    public:
      StaticProperty (char const* name, T (*get) (), void (*set)(T), Symbol* parent)
        : Property <T> (name, get, set, parent)
      {
      }

      std::string const tostring (bool includeChildren) const
      {
        std::string s = "static " + Property <T>::tostring (includeChildren);

        return s;
      }
    };

    //==========================================================================
    /**
      A static class method.
    */
    template <class FP>
    class StaticMethod : public Function <FP>
    {
    public:
      //--------------------------------------------------------------------------
      /**
        Create a static method.
      */
      StaticMethod (char const* name, FP fp, Symbol* parent)
        : Function <FP> (name, fp, parent)
      {
      }

      //--------------------------------------------------------------------------
      /**
        Return the declaration string.
      */
      std::string const tostring (bool includeChildren) const
      {
        std::string s = "static " + Function <FP>::tostring (includeChildren);

        return s;
      }
    };

    //==========================================================================
    /**
      A class data member.
    */
    template <class U>
    class DataMember : public Symbol
    {
    private:
      const U T::* m_mp;
      bool const m_isWritable;

    private:
      //------------------------------------------------------------------------
      /**
        lua_CFunction to get a class data member.

        @note The expected class name is in upvalue 1, and the pointer to the
              data member is in upvalue 2.
      */
      template <class T, typename U>
      static int propgetProxy (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T const* const t = ud->getConst <T> (L);
        U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
        Stack <U>::push (L, t->*mp);
        return 1;
      }

      //------------------------------------------------------------------------
      /**
        lua_CFunction to set a class data member.

        @note The expected class name is in upvalue 1, and the pointer to the
              data member is in upvalue 2.
      */
      template <class T, typename U>
      static int propsetProxy (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T* const t = ud->get <T> (L);
        U T::* mp = *static_cast <U T::**> (lua_touserdata (L, lua_upvalueindex (1)));
        t->*mp = Stack <U>::get (L, 2);
        return 0;
      }

    public:
      //========================================================================
      /**
        Create a class data member.
      */
      DataMember (char const* name, const U T::* mp, bool isWritable, Symbol* parent)
        : Symbol (name, parent)
        , m_mp (mp)
        , m_isWritable (isWritable)
      {
      }

      //------------------------------------------------------------------------
      /**
        Return the declaration string.
      */
      std::string const tostring (bool) const
      {
        std::string s = std::string (typeid (U).name ()) + " " + m_name;

        if (!m_isWritable)
          s = s + " (read-only)";

        return s;
      }

      //------------------------------------------------------------------------
      /**
        Add this data member to the lua_State.

        The stack has the static table, meta table, and const metatable
        at -1, -2, and -3.
      */
      void addToState (lua_State* L)
      {
        char const* const name = m_name.c_str ();

        // add to __propget
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        void* const v = lua_newuserdata (L, sizeof (U T::*));
        memcpy (v, &m_mp, sizeof (U T::*));
        lua_pushcclosure (L, &propgetProxy <T, U>, 1);
        lua_pushvalue (L, -1);
        rawsetfield (L, -4, name);
        rawsetfield (L, -2, name);
        lua_pop (L, 2);

        if (m_isWritable)
        {
          // add to __propset
          rawgetfield (L, -2, "__propset");
          if (!lua_istable (L, -1))
            throw std::logic_error ("expected table");
          void* const v = lua_newuserdata (L, sizeof (U T::*));
          memcpy (v, &m_mp, sizeof (U T::*));
          lua_pushcclosure (L, &propsetProxy <T, U>, 1);
          rawsetfield (L, -2, name);
          lua_pop (L, 1);
        }
      }
    };

    //==========================================================================
    /**
      lua_CFunction to call a class member function with a return value.

      The argument list contains the 'this' pointer followed by the method
      arguments.

      @note The expected class name is in upvalue 1, and the member function
            pointer is in upvalue 2.
    */
    template <class MemFn,
              typename RetVal = typename FunctionPointer <MemFn>::resulttype>
    struct methodProxy
    {
      typedef typename FunctionPointer <MemFn>::classtype T;
      typedef typename FunctionPointer <MemFn>::params params;

      static int func (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T* const t = ud->get <T> (L);
        MemFn fp = *static_cast <MemFn*> (
          lua_touserdata (L, lua_upvalueindex (1)));
        arglist <params, 2> args(L);
        Stack <RetVal>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
        return 1;
      }

      // const class member functions
      static int const_func (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T const* const t = ud->getConst <T> (L);
        MemFn fp = *static_cast <MemFn*> (
          lua_touserdata (L, lua_upvalueindex (1)));
        arglist <params, 2> args(L);
        Stack <RetVal>::push (L, FunctionPointer <MemFn>::call (t, fp, args));
        return 1;
      }
    };

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to call a class member function with no return value.

      The argument list contains the 'this' pointer followed by the method
      arguments.

      @note The expected class name is in upvalue 1, and the member function
            pointer is in upvalue 2.
    */
    template <typename MemFn>
    struct methodProxy <MemFn, void>
    {
      typedef typename FunctionPointer <MemFn>::classtype T;
      typedef typename FunctionPointer <MemFn>::params params;

      static int func (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T* const t = ud->get <T> (L);
        MemFn fp = *static_cast <MemFn*> (
          lua_touserdata (L, lua_upvalueindex (1)));
        arglist <params, 2> args (L);
        FunctionPointer <MemFn>::call (t, fp, args);
        return 0;
      }

      // const class member functions
      static int const_func (lua_State* L)
      {
        void* const p = detail::checkClass (
          L, 1, ClassInfo <T>::getClassKey (), false);
        Userdata* const ud = static_cast <Userdata*> (p);
        T const* const t = ud->getConst <T> (L);
        MemFn fp = *static_cast <MemFn*> (
          lua_touserdata (L, lua_upvalueindex (1)));
        arglist <params, 2> args (L);
        FunctionPointer <MemFn>::call (t, fp, args);
        return 0;
      }
    };

    //==========================================================================
    /**
      A class property member.

      These are implemented using a methodProxy.
    */
    template <class U>
    class PropertyMember : public Symbol
    {
    private:
      U (T::* m_get) () const;
      void (T::* m_set) (U);

    private:

    public:
      //------------------------------------------------------------------------
      /**
        Create a class property member.
      */
      PropertyMember (char const* name,
        U (T::* get) () const, void (T::* set) (U), Symbol* parent)
        : Symbol (name, parent)
        , m_get (get)
        , m_set (set)
      {
      }

      //------------------------------------------------------------------------
      /**
        Return the declaration string.
      */
      std::string const tostring (bool) const
      {
        std::string s = std::string (typeid (U).name ()) + " " + m_name;

        if (m_set == 0)
          s = s + " (read-only)";

        return s;
      }

      void addToState (lua_State* L)
      {
        char const* name = m_name.c_str ();

        // add to __propget
        rawgetfield (L, -2, "__propget");
        rawgetfield (L, -4, "__propget");
        typedef U (T::*MemFn) () const;
        void* const v = lua_newuserdata (L, sizeof (MemFn));
        memcpy (v, &m_get, sizeof (MemFn));
        lua_pushcclosure (L, &methodProxy <MemFn>::const_func, 1);
        lua_pushvalue (L, -1);
        rawsetfield (L, -4, name);
        rawsetfield (L, -2, name);
        lua_pop (L, 2);

        if (m_set != 0)
        {
          // add to __propset
          rawgetfield (L, -2, "__propset");
          typedef void (T::* MemFn) (U);
          void* const v = lua_newuserdata (L, sizeof (MemFn));
          memcpy (v, &m_set, sizeof (MemFn));
          lua_pushcclosure (L, &methodProxy <MemFn>::func, 1);
          rawsetfield (L, -2, name);
          lua_pop (L, 1);
        }
      }
    };

    //==========================================================================
    /**
      A class member function.
    */
    template <class MemFn>
    class MemberFunction : public Symbol
    {
    private:
      typedef typename FunctionPointer <MemFn>::resulttype ReturnType;
      typedef typename FunctionPointer <MemFn>::params Params;
      typedef typevallist <Params> TypeList;

      MemFn const m_mfp;
      bool const m_isConst;

    public:
      MemberFunction (char const* name, MemFn mfp, Symbol* parent)
        : Symbol (name, parent)
        , m_mfp (mfp)
        , m_isConst (FunctionPointer <MemFn>::const_mfp)
      {
        assert (FunctionPointer <MemFn>::mfp);
      }

      //------------------------------------------------------------------------
      /**
        Return the declaration string.
      */
      std::string const tostring (bool) const
      {
        std::string s = typeid (ReturnType).name ();

        s = s + " " + m_name + " (" + TypeList::tostring (false) + ")";

        if (m_isConst)
          s = s + " const";

        return s;
      }

      //------------------------------------------------------------------------
      /**
        Add this class to the lua_State.

        The stack holds the static, meta, and const metatables for the class.
      */
      void addToState (lua_State* L)
      {
        if (m_isConst)
        {
          // add to const metatable
          void* const v = lua_newuserdata (L, sizeof (MemFn));
          memcpy (v, &m_mfp, sizeof (MemFn));
          lua_pushcclosure (L, &methodProxy <MemFn>::func, 1);
          rawsetfield (L, -4, m_name.c_str ());
        }

        // add to metatable
        void* const v = lua_newuserdata (L, sizeof (MemFn));
        memcpy (v, &m_mfp, sizeof (MemFn));
        lua_pushcclosure (L, &methodProxy <MemFn>::func, 1);
        rawsetfield (L, -3, m_name.c_str ());
      }
    };

  private:
    //--------------------------------------------------------------------------
    /**
      __print metamethod for a Class.
    */
    static int tostringMetaMethod (lua_State* L)
    {
      if (!lua_islightuserdata (L, lua_upvalueindex (1)))
        throw std::logic_error ("bad upvalue");
    
      Class <T> const* const c = static_cast <Class <T>*> (
        lua_touserdata (L, lua_upvalueindex (1)));

      std::string const s = c->tostring (true);

      lua_pushstring (L, s.c_str ());

      return 1;
    }

    //--------------------------------------------------------------------------
    /**
      __index metamethod for a Class.

      This implements member functions, data members, and property members.
      Functions are stored in the metatable and const metatable. Data members
      and property members are in the __propget table.

      If the key is not found, the search proceeds up the hierarchy of base
      classes.
    */
    static int indexMetaMethod (lua_State* L)
    {
      int result = 0;

      lua_getmetatable (L, 1);                      // get metatable for object

      for (;;)
      {
        lua_pushvalue (L, 2);                       // push key arg2
        lua_rawget (L, -2);                         // lookup key in metatable
        if (lua_iscfunction (L, -1))                // ensure its a cfunction
        {
          lua_remove (L, -2);                       // remove metatable
          result = 1;
          break;
        }
        else if (lua_isnil (L, -1))
        {
          lua_pop (L, 1);
        }
        else
        {
          lua_pop (L, 2);

          // We only put cfunctions into the metatable.
          throw std::logic_error ("not a cfunction");
        }

        rawgetfield (L, -1, "__propget");           // get __propget table
        if (lua_istable (L, -1))                    // ensure it is a table
        {
          lua_pushvalue (L, 2);                     // push key arg2
          lua_rawget (L, -2);                       // lookup key in __propget
          lua_remove (L, -2);                       // remove __propget
          if (lua_iscfunction (L, -1))              // ensure its a cfunction
          {
            lua_remove (L, -2);                     // remove metatable
            lua_pushvalue (L, 1);                   // push class arg1
            lua_call (L, 1, 1);
            result = 1;
            break;
          }
          else if (lua_isnil (L, -1))
          {
            lua_pop (L, 1);
          }
          else
          {
            lua_pop (L, 2);

            // We only put cfunctions into __propget.
            throw std::logic_error ("not a cfunction");
          }
        }
        else
        {
          lua_pop (L, 2);

          // __propget is missing, or not a table.
          throw std::logic_error ("missing __propget table");
        }

        // Repeat the lookup in the __parent metafield,
        // or return nil if the field doesn't exist.
        rawgetfield (L, -1, "__parent");
        if (lua_istable (L, -1))
        {
          // Remove metatable and repeat the search in __parent.
          lua_remove (L, -2);
        }
        else if (lua_isnil (L, -1))
        {
          result = 1;
          break;
        }
        else
        {
          lua_pop (L, 2);

          throw std::logic_error ("__parent is not a table");
        }
      }

      return result;
    }

    //--------------------------------------------------------------------------
    /**
      __newindex metamethod for classes.

      This supports writable variables and properties on class objects. The
      corresponding object is passed in the first parameter to the set function.
    */
    static int newindexMetaMethod (lua_State* L)
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
            "attempt to set '%s', which isn't a property", lua_tostring (L, 2));
        }
        lua_remove (L, -2);
      }

      return result;
    }

    //--------------------------------------------------------------------------
    /**
      lua_CFunction to destroy a class object.

      This is used for the __gc metamethod.
    */
    template <class T>
    static int dtorProxy (lua_State* L)
    {
      void* const p = detail::checkClass (L, 1, ClassInfo <T>::getClassKey (), true);
      Userdata* const ud = static_cast <Userdata*> (p);
      ud->~Userdata ();
      return 0;
    }

    //--------------------------------------------------------------------------
    /**
      Find the symbol in this class.
    */
    Symbol* findSymbol (char const* name)
    {
      Symbol* result = 0;

      for (Symbol* s = m_symbols.head (); s; s = s->next ())
      {
        if (s->getName () == name)
        {
          result = s;
          break;
        }
      }

      return result;
    }

  public:
    //==========================================================================
    /**
      Create a class.
    */
    Class (char const* name, Namespace* parent,
           void* baseStaticKey,
           void* baseClassKey,
           void* baseConstKey)
      : Symbol (name, parent)
      , m_baseStaticKey (baseStaticKey)
      , m_baseClassKey (baseClassKey)
      , m_baseConstKey (baseConstKey)
    {
    }

    //--------------------------------------------------------------------------
    /**
      Add a static data member.
    */
    template <class U>
    Class <T>& addStaticData (char const* name, U* pu, bool isWritable = true)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new StaticData <U> (name, pu, isWritable, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add a static property.
    */
    template <class U>
    Class <T>& addStaticProperty (char const* name, U (*get)(), void (*set)(U) = 0)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new StaticProperty <U> (name, get, set, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add a static method.
    */
    template <class FP>
    Class <T>& addStaticMethod (char const* name, FP fp)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new StaticMethod <FP> (name, fp, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add a data member.
    */
    template <class U>
    Class <T>& addData (char const* name, const U T::* mp, bool isWritable = true)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new DataMember <U> (name, mp, isWritable, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add a property member.
    */
    template <class U>
    Class <T>& addProperty (char const* name,
      U (T::* get) () const, void (T::* set) (U) = 0)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new PropertyMember <U> (name, get, set, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Add a member function.
    */
    template <class MemFn>
    Class <T>& addMethod (char const* name, MemFn mf)
    {
      Symbol* symbol = findSymbol (name);
      if (symbol != 0)
        throw std::logic_error ("duplicate symbol");

      symbol = new MemberFunction <MemFn> (name, mf, this);
      m_symbols.append (symbol);

      return *this;
    }

    //--------------------------------------------------------------------------
    /**
      Return the declaration string.
    */
    std::string const tostring (bool includeChildren) const
    {
      std::string s = "class " + m_name;

      if (includeChildren)
      {
        if (!m_symbols.empty ())
        {
          for (Symbol* symbol = m_symbols.head (); symbol; symbol = symbol->next ())
            s = s + "\n " + symbol->tostring (false);
        }
      }

      return s;
    }

    //--------------------------------------------------------------------------
    /**
      Add this class to the lua_State.

      The parent table is at the top of the stack.
    */
    void addToState (lua_State* L)
    {
      char const* const name = m_name.c_str ();

      rawgetfield (L, -1, name);
      if (!lua_isnil (L, -1))
        throw std::logic_error ("duplicate symbol");
      lua_pop (L, 1);

      // Create const metatable

      lua_newtable (L);

      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);

      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");

      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");

      lua_pushcclosure (L, &dtorProxy <T>, 0);
      rawsetfield (L, -2, "__gc");

      lua_pushstring (L, (std::string ("const ") + m_name).c_str ());
      rawsetfield (L, -2, "__type");

      lua_newtable (L);
      rawsetfield (L, -2, "__propget");

      if (m_baseConstKey != 0)
      {
        lua_rawgetp (L, LUA_REGISTRYINDEX, m_baseConstKey);
        rawsetfield (L, -2, "__parent");
      }

      // Create metatable

      lua_newtable (L);

      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);

      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");

      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");

      lua_pushcclosure (L, &dtorProxy <T>, 0);
      rawsetfield (L, -2, "__gc");

      lua_pushstring (L, name);
      rawsetfield (L, -2, "__type");

      lua_newtable (L);
      rawsetfield (L, -2, "__propget");
      
      lua_newtable (L);
      rawsetfield (L, -2, "__propset");

      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__const"); // points to const metatable

      lua_pushvalue (L, -1);
      rawsetfield (L, -3, "__class"); // pointer from const table to metatable

      if (m_baseClassKey != 0)
      {
        lua_rawgetp (L, LUA_REGISTRYINDEX, m_baseClassKey);
        rawsetfield (L, -2, "__parent");
      }

      // Create static table

      lua_newtable (L);

      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);

      lua_pushlightuserdata (L, this);
      lua_pushcclosure (L, &tostringMetaMethod, 1);
      rawsetfield (L, -2, "__tostring");

      lua_pushcfunction (L, &Namespace::indexMetaMethod);
      rawsetfield (L, -2, "__index");

      lua_pushcfunction (L, &Namespace::newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");

      lua_newtable (L);
      rawsetfield (L, -2, "__propget");

      lua_newtable (L);
      rawsetfield (L, -2, "__propset");

      lua_pushvalue (L, -2);
      rawsetfield (L, -2, "__class"); // points to metatable

      lua_pushvalue (L, -1);
      rawsetfield (L, -5, name);

      if (m_baseStaticKey != 0)
      {
        lua_rawgetp (L, LUA_REGISTRYINDEX, m_baseStaticKey);
        rawsetfield (L, -2, "__parent");
      }

      // Add symbols

      for (Symbol* s = m_symbols.head (); s; s = s->next ())
        s->addToState (L);
 
      // Map T to static table
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getStaticKey ());

      // Map T to metatable
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getClassKey ());

      // Map T to const metatable
      lua_rawsetp (L, LUA_REGISTRYINDEX, ClassInfo <T>::getConstKey ());
    }

    //--------------------------------------------------------------------------
    /**
      End the class declaration.

      It is still possible to add declarations later.
    */
    Namespace& endClass () const
    {
      Namespace* n = dynamic_cast <Namespace*> (m_parent);
      if (n == 0)
        throw std::logic_error ("expected Namespace");
      return *n;
    }
  };

private:
  //============================================================================
  /**
    Creates a global namespace.
  */
  Namespace () : Symbol ("", 0)
  {
  }

  //----------------------------------------------------------------------------
  /**
    Creates a named namespace.
  */
  Namespace (char const* name, Namespace* const parent)
    : Symbol (name, parent)
  {
  };

  //----------------------------------------------------------------------------
  /**
    Destroys the namespace.
  */
  ~Namespace ()
  {
    Symbol* s = m_symbols.head ();
    while (s != 0)
    {
      Symbol* next = s->next ();
      delete s;
      s = next;
    }
  }

public:
  //----------------------------------------------------------------------------
  /**
    Retrieve a global namespace.

    The Tag template parameter allows for multiple global namespaces. This is
    only useful when there is more than one lua_State, and even then, only
    rare circumstances.
  */
  template <class Tag>
  static Namespace& getGlobalNamespace ()
  {
    static Namespace globalNamespace;
    return globalNamespace;
  }

  //----------------------------------------------------------------------------
  /**
    Open a namespace for registration.

    The namespace is created if it doesn't already exist.
  */
  Namespace& beginNamespace (char const* name)
  {
    Namespace* n = 0;
    Symbol* symbol = findSymbol (name);

    if (symbol == 0)
    {
      n = new Namespace (name, this);
      m_symbols.append (n);
    }
    else
    {
      n = dynamic_cast <Namespace*> (symbol);
      if (n == 0)
        throw (std::invalid_argument ("not a namespace"));
    }

    return *n;
  }

  //----------------------------------------------------------------------------
  /**
    End registration for the namespace.

    @return The parent namespace.
  */
  Namespace& endNamespace ()
  {
    Namespace* n = dynamic_cast <Namespace*> (m_parent);

    if (n == 0)
      throw std::logic_error ("expected Namespace");

    return *n;
  }

  //----------------------------------------------------------------------------
  /**
    Register a function in this namespace.
  */
  template <class FP>
  Namespace& addFunction (char const* name, FP const fp)
  {
    Symbol* symbol = findSymbol (name);
    if (symbol != 0)
      throw std::logic_error ("duplicate symbol");
    
    symbol = new Function <FP> (name, fp, this);
    m_symbols.append (symbol);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a variable in this namespace.
  */
  template <class T>
  Namespace& addVariable (char const* name, T* pt, bool isWritable = true)
  {
    Symbol* symbol = findSymbol (name);
    if (symbol != 0)
      throw std::logic_error ("duplicate symbol");

    symbol = new Variable <T> (name, pt, isWritable, this);
    m_symbols.append (symbol);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Register a property in this namespace.
  */
  template <class T>
  Namespace& addProperty (char const* name, T (*get)(), void (*set)(T) = 0)
  {
    Symbol* symbol = findSymbol (name);
    if (symbol != 0)
      throw std::logic_error ("duplicate symbol");

    symbol = new Property <T> (name, get, set, this);
    m_symbols.append (symbol);

    return *this;
  }

  //----------------------------------------------------------------------------
  /**
    Declare a class in this namespace.
  */
  template <class T>
  Class <T>& beginClass (char const* name)
  {
    Class <T>* c = 0;
    Symbol* symbol = findSymbol (name);

    if (symbol == 0)
    {
      c = new Class <T> (name, this, 0, 0, 0);
      m_symbols.append (c);
    }
    else
    {
      c = dynamic_cast <Class <T>*> (symbol);
      if (c == 0)
        throw (std::invalid_argument ("not a class"));
    }

    return *c;
  }

  //----------------------------------------------------------------------------
  /**
    Declare a derived class in this namespace.

    This should only be used once on a derived class. To add more members
    to an existing subclass, use beginClass ().
  */
  template <class T, class B>
  Class <T>& deriveClass (char const* name)
  {
    Symbol* symbol = findSymbol (name);
    if (symbol != 0)
      throw std::logic_error ("duplicate symbol");

    Class <T>* c = new Class <T> (name, this,
      ClassInfo <B>::getStaticKey (),
      ClassInfo <B>::getClassKey (),
      ClassInfo <B>::getConstKey ()
      );
    m_symbols.append (c);

    return *c;
  }

  //----------------------------------------------------------------------------
  /**
    Produce a string with the classes and namespaces in this namespace.
  */
  std::string const tostring (bool includeChildren) const
  {
    std::string s = "namespace " + m_name;

    if (includeChildren)
    {
      if (!m_symbols.empty ())
      {
        for (Symbol* symbol = m_symbols.head (); symbol; symbol = symbol->next ())
          s = s + "\n " + symbol->tostring (false);
      }
    }

    return s;
  }

  //----------------------------------------------------------------------------
  /**
    Add everything in this namespace to the given lua_State.
  */
  void addToState (lua_State* L)
  {
    if (m_name == "")
    {
      lua_getglobal (L, "_G");
    }
    else
    {
      rawgetfield (L, -1, m_name.c_str ());
      if (!lua_isnil (L, -1))
        throw std::logic_error ("duplicate symbol");
      lua_pop (L, 1);

      lua_newtable (L);

      lua_pushvalue (L, -1);
      lua_setmetatable (L, -2);

      lua_pushlightuserdata (L, this);
      lua_pushcclosure (L, &tostringMetaMethod, 1);
      rawsetfield (L, -2, "__tostring");

      lua_pushcfunction (L, &indexMetaMethod);
      rawsetfield (L, -2, "__index");

      lua_pushcfunction (L, &newindexMetaMethod);
      rawsetfield (L, -2, "__newindex");

      lua_newtable (L);
      rawsetfield (L, -2, "__propget");

      lua_newtable (L);
      rawsetfield (L, -2, "__propset");

      lua_pushvalue (L, -1);
      rawsetfield (L, -3, m_name.c_str ());
    }

    for (Symbol* s = m_symbols.head (); s; s = s->next ())
      s->addToState (L);

    lua_pop (L, 1);
  }
};

//==============================================================================
/**
  Uniquely identifies a global namespace.

  In the rare case where two global namespaces are needed (for different
  lua_State objects), they can be distinguished using a named type.
*/
struct DefaultTag { };

/**
  Retrieve a global namespace.
*/
template <class Tag>
inline Namespace& getGlobalNamespace ()
{
  return Namespace::getGlobalNamespace <Tag> ();
}

/**
  Retrieve the default global namespace.

  Almost everyone will use this.
*/
inline Namespace& getGlobalNamespace ()
{
  return getGlobalNamespace <DefaultTag> ();
}

}

//==============================================================================

#endif
