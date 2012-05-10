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

#ifndef USE_OTHER_SHARED_PTR
#include "shared_ptr.h"
#endif

namespace luabridge
{

#include "typelist.h"
#include "stack.h"

// forward declaration
template <typename T>
class class__;

//==============================================================================

/** Get a value, bypassing metamethods.
    @internal
*/  
inline void rawgetfield (lua_State* const L, int const index, char const* const key)
{
  lua_pushstring (L, key);
  if (index < 0)
    lua_rawget (L, index-1);
  else
    lua_rawget (L, index);
}

/** Set a value, bypassing metamethods.
    @internal
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

//==============================================================================
/**
  Utilities.

  These are provided as static class members so the definitions may be placed
  in the header rather than a source file.
*/
struct util
{
  //----------------------------------------------------------------------------
  /**
    Produce an error message.

    This is our version of luaL_typerror, which was removed in Lua 5.2.
  */
  static int typeError (lua_State *L, int narg, const char *tname)
  {
    const char *msg = lua_pushfstring(L, "%s expected, got %s",
      tname, luaL_typename(L, narg));

    return luaL_argerror(L, narg, msg);
  }

  //----------------------------------------------------------------------------
  /**
    Custom __index metamethod for C++ classes.

    If the given key is not found, the search will be delegated up the parent
    hierarchy.
  */
  static int meta_index (lua_State *L)
  {
    lua_getmetatable (L, 1);

    for (;;)
    {
      // Look for the key in the metatable
      lua_pushvalue(L, 2);
      lua_rawget(L, -2);
      // Did we get a non-nil result?  If so, return it
      if (!lua_isnil(L, -1))
        return 1;
      lua_pop(L, 1);

      // Look for the key in the __propget metafield
      rawgetfield(L, -1, "__propget");
      if (!lua_isnil(L, -1))
      {
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);
        // If we got a non-nil result, call it and return its value
        if (!lua_isnil(L, -1))
        {
          assert(lua_isfunction(L, -1));
          lua_pushvalue(L, 1);
          lua_call(L, 1, 1);
          return 1;
        }
        lua_pop(L, 1);
      }
      lua_pop(L, 1);

      // Look for the key in the __const metafield
      rawgetfield(L, -1, "__const");
      if (!lua_isnil(L, -1))
      {
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);
        if (!lua_isnil(L, -1))
          return 1;
        lua_pop(L, 1);
      }
      lua_pop(L, 1);

      // Look for a __parent metafield; if it doesn't exist, return nil;
      // otherwise, repeat the lookup on it.
      rawgetfield(L, -1, "__parent");
      if (lua_isnil(L, -1))
        return 1;
      lua_remove(L, -2);
    }

    // Control never gets here
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    Custom __newindex metamethod.

    This supports properties on scopes, and static properties of classes.
  */
  static int meta_newindex (lua_State *L)
  {
    lua_getmetatable(L, 1);

    for (;;)
    {
      // Look for the key in the __propset metafield
      rawgetfield(L, -1, "__propset");
      if (!lua_isnil(L, -1))
      {
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);
        // If we got a non-nil result, call it
        if (!lua_isnil(L, -1))
        {
          assert(lua_isfunction(L, -1));
          lua_pushvalue(L, 3);
          lua_call(L, 1, 0);
          return 0;
        }
        lua_pop(L, 1);
      }
      lua_pop(L, 1);

      // Look for a __parent metafield; if it doesn't exist, error;
      // otherwise, repeat the lookup on it.
      rawgetfield(L, -1, "__parent");
      if (lua_isnil(L, -1))
      {
        return luaL_error(L, "attempt to set %s, which isn't a property",
          lua_tostring(L, 2));
      }
      lua_remove(L, -2);
    }

    // Control never gets here
    return 0;
  }

  //----------------------------------------------------------------------------
  /**
    Create a static table.

    The resulting table is placed on the stack.
  */
  static void createStaticTable (lua_State *L)
  {
    lua_newtable (L);                         // Create the table.
    lua_pushvalue (L, -1);
    lua_setmetatable (L, -2);                 // Set it as its own metatable.
    lua_pushcfunction (L, &util::meta_index);
    rawsetfield (L, -2, "__index");           // Use our __index.
    lua_pushcfunction (L, &util::meta_newindex);
    rawsetfield (L, -2, "__newindex");        // Use our __newindex.
    lua_newtable (L);
    rawsetfield (L, -2, "__propget");         // Set __propget as empty table.
    lua_newtable (L);
    rawsetfield (L, -2, "__propset");         // Set __propset as empty table.
  }

  //----------------------------------------------------------------------------
  /**
    Look up a static table.

    The table is identified by its fully qualified dot-separated name. The
    resulting table is returned on the stack.

    @invariant The table must exist.
  */
  static void findStaticTable (lua_State* const L, char const* const name)
  {
    lua_getglobal (L, "_G");

    if (name && name [0] != '\0')
    {
      std::string namestr (name);
      size_t start = 0;
      size_t pos = 0;
      while ((pos = namestr.find('.', start)) != std::string::npos)
      {
        lua_getfield(L, -1, namestr.substr(start, pos - start).c_str());
        assert(!lua_isnil(L, -1));
        lua_remove(L, -2);
        start = pos + 1;
      }
      lua_getfield(L, -1, namestr.substr(start).c_str());
      assert(!lua_isnil(L, -1));
      lua_remove(L, -2);
    }
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction for a function signature and a return type.

    @note This is registered as a closure with the actual
          function pointer in the first upvalue.
  */
  template <typename Function,
            typename Retval = typename fnptr <Function>::resulttype>
  struct functionProxy
  {
    typedef typename fnptr<Function>::params params;
    static int f (lua_State *L)
    {
      Function fp = static_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params> args (L);
      tdstack <Retval>::push (L, fnptr <Function>::apply (fp, args));
      return 1;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction for a function signature and a void return type.

    @note This is registered as a closure with the actual
          function pointer in the first upvalue.
  */
  template <typename Function>
  struct functionProxy <Function, void>
  {
    typedef typename fnptr <Function>::params params;
    static int f (lua_State *L)
    {
      Function fp = static_cast <Function> (lua_touserdata (L, lua_upvalueindex (1)));
      arglist <params> args (L);
      fnptr <Function>::apply (fp, args);
      return 0;
    }
  };

  //----------------------------------------------------------------------------
  /**
    lua_CFunction for getting a variable.

    This is also used for static properties.

    @note This is registered as a closure with a pointer to
          the variable in the first upvalue.
  */

  template <typename T>
  static int varget_proxy (lua_State *L)
  {
    T* data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
    tdstack <T>::push (L, *data);
    return 1;
  }

  //----------------------------------------------------------------------------
  /**
    lua_CFunction for setting a variable.

    This is also used for static properties.

    @note This is registered as a closure with a pointer to
          the variable in the first upvalue.
  */

  template <typename T>
  static int varset_proxy (lua_State *L)
  {
    T* data = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
    *data = tdstack <T>::get (L, 1);
    return 0;
  }
};

//==============================================================================
/**
  Registration manager.

  Performs registration tasks for a specified Lua state.

  @todo namespace support.
*/
class scope
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

    This creates a chain of tables registered into the global scope. For
    example, "x.y.z" produces _G["x"]["y"]["z"] with subsequent registrations
    going into z[].
  */
  scope (lua_State *L_, char const *name_) : L (L_), name (name_)
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
        util::createStaticTable (L);
        lua_pushvalue (L, -1);
        rawsetfield (L, -3, id.c_str ());
      }
      lua_remove(L, -2);
      start = pos + 1;
    }

    // Create a new table with the remaining portion of the name.
    util::createStaticTable (L);
    rawsetfield (L, -2, name.c_str() + start);
    lua_pop (L, 1);
  }

  //----------------------------------------------------------------------------
  /**
    Register a function in this scope.
  */
  template <typename Function>
  scope& function (char const* const name, Function fp)
  {
    util::findStaticTable (L, this->name.c_str());
    lua_pushlightuserdata (L, static_cast <void*> (fp));
    lua_pushcclosure (L, &util::functionProxy <Function>::f, 1);
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

    util::findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propget");
    lua_pushlightuserdata (L, const_cast <void*> (static_cast <void const*> (data)));
    lua_pushcclosure (L, &util::varget_proxy <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

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

    util::findStaticTable (L, this->name.c_str ());
    rawgetfield(L, -1, "__propget");
    lua_pushlightuserdata (L, static_cast <void*> (getFunction));
    lua_pushcclosure (L, &util::functionProxy<T (*) ()>::f, 1);
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
    util::findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushlightuserdata (L, static_cast <void*> (data));
    lua_pushcclosure (L, &util::varset_proxy <T>, 1);
    rawsetfield (L, -2, name);
    lua_pop (L, 2);
    return *this;
  }

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
    util::findStaticTable (L, this->name.c_str ());
    rawgetfield (L, -1, "__propset");
    lua_pushlightuserdata (L, static_cast <void*> (setFunction));
    lua_pushcclosure (L, &util::functionProxy <void (*) (T)>::f, 1);
    rawsetfield(L, -2, name);
    lua_pop(L, 2);
    return *this;
  }

  // Class registration

  // For registering a class that hasn't been registered before
  template <typename T>
  class__<T> class_ (const char *name);
  // For registering subclasses (the base class must also be registered)
  template <typename T, typename Base>
  class__<T> subclass (const char *name);
  // For registering additional methods of a previously registered class
  // (or subclass)
  template <typename T>
  class__<T> class_ ();

protected:
  lua_State *L;
  std::string name;
};

//==============================================================================

// class__ performs registration for members of a class
template <typename T>
class class__ : public scope
{
public:
  class__ (lua_State *L_);
  class__ (lua_State *L_, const char *name_);
  class__ (lua_State *L_, const char *name_, const char *basename);

  // Constructor registration.  The template parameter should be passed
  // a function pointer type; only the argument list will be used (since
  // you can't take the address of a ctor).
  template <typename FnPtr>
  class__<T>& constructor ();

  // Method registration
  template <typename FnPtr>
  class__<T>& method (const char *name, FnPtr fp);

  // Property registration.  Properties can be read/write (rw)
  // or read-only (ro).  Varieties that access member pointers directly
  // and varieties that access through function calls are provided.
  template <typename U>
  class__<T>& property_ro (const char *name, const U T::* mp);
  template <typename U>
  class__<T>& property_ro (const char *name, U (T::*get) () const);
  template <typename U>
  class__<T>& property_rw (const char *name, U T::* mp);
  template <typename U>
  class__<T>& property_rw (const char *name,
    U (T::*get) () const, void (T::*set) (U));

  // Static method registration
  template <typename FnPtr>
  class__<T>& static_method (const char *name, FnPtr fp)
  { return *(class__<T>*)&(function(name, fp)); }

  // Static property registration
  template <typename U>
  class__<T>& static_property_ro (const char *name, const U *data)
  { return *(class__<T>*)&(variable_ro<U>(name, data)); }
  template <typename U>
  class__<T>& static_property_ro (const char *name, U (*get) ())
  { return *(class__<T>*)&(variable_ro<U>(name, get)); }
  template <typename U>
  class__<T>& static_property_rw (const char *name, U *data)
  { return *(class__<T>*)&(variable_rw<U>(name, data)); }
  template <typename U>
  class__<T>& static_property_rw (const char *name, U (*get) (),
    void (*set) (U))
  { return *(class__<T>*)&(variable_rw<U>(name, get, set)); }

  // !!!UNDONE: allow inheriting Lua classes from C++ classes
};

// Prototypes for implementation functions implemented in luabridge.cpp
void *checkclass (lua_State *L, int idx, const char *tname, bool exact = false);
int m_newindexer (lua_State *L);

// Predeclare classname struct since several implementation files use it
template <typename T>
struct classname;
extern const char *classname_unknown;

/*
* Perform class registration in a scope.
*/

template <typename T>
class__<T> scope::class_ ()
{
  return class__<T>(L);
}

template <typename T, typename Base>
class__<T> scope::subclass (const char *name)
{
  assert(classname<Base>::name() != classname_unknown);
  return class__<T>(L, name, classname<Base>::name());
}

template <typename T>
class__<T> scope::class_ (const char *name)
{
  return class__<T>(L, name);
}

/*
* Container for registered class names, with awareness of const types
*/

template <typename T>
struct classname
{
  static const char *name_;
  static const char *name ()
  {
    return classname<T>::name_;
  }
  static bool is_const ()
  {
    return false;
  }
  static void set_name (const char *name)
  {
    classname<T>::name_ = name;
  }
};

// Initial type names are unknown
template <typename T>
const char *classname<T>::name_ = classname_unknown;

// Specialization for const types, mapping to same names
template <typename T>
struct classname <const T>: public classname<T>
{
  static bool is_const ()
  {
    return true;
  }
};

/*
* Lua-registerable C function template for destructors.  Objects are stored
* in Lua as userdata containing a shared_ptr, and this is registered as the
* __gc metamethod.  The expected classname is passed as an upvalue so that
* we can ensure that we are destructing the right kind of object.
*/

template <typename T>
int destructor_dispatch (lua_State *L)
{
  void *obj = checkclass(L, 1, lua_tostring(L, lua_upvalueindex(1)), true);
  shared_ptr<T> &ptr = *((shared_ptr<T> *)obj);
  ptr.~shared_ptr<T>();
  return 0;
}

/*
* Functions for metatable construction.  These functions create a metatable and
* leave it in the top element of the Lua stack (in addition to registering it
* wherever it needs to be registered).
*/

template <typename T>
void create_metatable (lua_State *L, const char *name)
{
  luaL_newmetatable(L, name);
  // Set indexer as the __index metamethod
  lua_pushcfunction(L, &util::meta_index);
  rawsetfield(L, -2, "__index");
  // Set m_newindexer as the __newindex metamethod
  lua_pushcfunction(L, &m_newindexer);
  rawsetfield(L, -2, "__newindex");
  // Set the __gc metamethod to call the class destructor
  lua_pushstring(L, name);
  lua_pushcclosure(L, &destructor_dispatch<T>, 1);
  rawsetfield(L, -2, "__gc");
  // Set the __type metafield to the name of the class
  lua_pushstring(L, name);
  rawsetfield(L, -2, "__type");
  // Create the __propget and __propset metafields as empty tables
  lua_newtable(L);
  rawsetfield(L, -2, "__propget");
  lua_newtable(L);
  rawsetfield(L, -2, "__propset");
}

template <typename T>
void create_const_metatable (lua_State *L, const char *name)
{
  std::string constname = std::string("const ") + name;
  luaL_newmetatable(L, constname.c_str());
  lua_pushcfunction(L, &util::meta_index);
  rawsetfield(L, -2, "__index");
  lua_pushcfunction(L, &m_newindexer);
  rawsetfield(L, -2, "__newindex");
  lua_pushstring(L, constname.c_str());
  lua_pushcclosure(L, &destructor_dispatch<T>, 1);
  rawsetfield(L, -2, "__gc");
  lua_pushstring(L, constname.c_str());
  rawsetfield(L, -2, "__type");
  lua_newtable(L);
  rawsetfield(L, -2, "__propget");
}

/*
* class__ constructors
*/

template <typename T>
class__<T>::class__ (lua_State *L_): scope(L_, classname<T>::name())
{
  assert(classname<T>::name() != classname_unknown);
}

template <typename T>
class__<T>::class__ (lua_State *L_, const char *name_): scope(L_, name_)
{
  assert(!classname<T>::is_const());
  classname<T>::set_name(name_);

  // Create metatable for this class.  The metatable is stored in the Lua
  // registry, keyed by the given class name.
  create_metatable<T>(L, name_);

  // Create const metatable for this class.  This is identical to the
  // previous metatable, except that it has "const " prepended to the __type
  // field, and has no __propset field.  Const methods will be added to the
  // const metatable, non-const methods to the normal metatable.
  create_const_metatable<T>(L, name_);

  // Set __const metafield to point to the const metatable
  rawsetfield(L, -2, "__const");
  // Pop the original metatable
  lua_pop(L, 1);
}

template <typename T>
class__<T>::class__ (lua_State *L_, const char *name_,
  const char *basename): scope(L_, name_)
{
  assert(!classname<T>::is_const());
  classname<T>::set_name(name_);

  // Create metatable for this class
  create_metatable<T>(L, name_);
  // Set the __parent metafield to the base class's metatable
  luaL_getmetatable(L, basename);
  rawsetfield(L, -2, "__parent");

  // Create const metatable for this class.  Its __parent field will point
  // to the const metatable of the parent class.
  create_const_metatable<T>(L, name_);
  std::string base_constname = std::string("const ") + basename;
  luaL_getmetatable(L, base_constname.c_str());
  rawsetfield(L, -2, "__parent");

  // Set __const metafield to point to the const metatable
  rawsetfield(L, -2, "__const");
  // Pop the original metatable
  lua_pop(L, 1);

  // Set the __parent metafield to the base class's static table
  util::findStaticTable(L, name_);
  util::findStaticTable(L, basename);
  rawsetfield(L, -2, "__parent");
  lua_pop(L, 1);
}

/*
* Lua-registerable C function template for constructor proxies.  These are
* registered to Lua as global functions with the name of the class, with the
* appropriate metatable passed as an upvalue.  They allocate a new userdata,
* initialize it with a shared_ptr to an appropriately constructed new class
* object, and set the metatable so that Lua can use the object.
*/

template <typename T, typename Params>
int constructor_proxy (lua_State *L)
{
  // Allocate a new userdata and construct a shared_ptr<T> in-place there
  void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
  arglist<Params, 2> args(L);
  new(block) shared_ptr<T>(constructor<T, Params>::apply(args));

  // Set the userdata's metatable
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_setmetatable(L, -2);

  return 1;
}

/*
* Perform constructor registration in a class.
*/

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::constructor ()
{
  // Get a reference to the class's static table
  util::findStaticTable(L, name.c_str());

  // Push the constructor proxy, with the class's metatable as an upvalue
  luaL_getmetatable(L, name.c_str());
  lua_pushcclosure(L,
    &constructor_proxy<T, typename fnptr<FnPtr>::params>, 1);

  // Set the constructor proxy as the __call metamethod of the static table
  rawsetfield(L, -2, "__call");
  lua_pop(L, 1);
  return *this;
}

/*
* Lua-registerable C function templates for method proxies.  These are
* registered with the expected classname as upvalue 1 and the member function
* pointer as upvalue 2.  When called from Lua, they will receive the object
* on which they are called as argument 1 and all the method arguments as
* args 2 and up.
*/

template <typename FnPtr, typename Ret = typename fnptr<FnPtr>::resulttype>
struct method_proxy
{
  typedef typename fnptr<FnPtr>::classtype classtype;
  typedef typename fnptr<FnPtr>::params params;
  static int f (lua_State *L)
  {
    classtype *obj = ((shared_ptr<classtype> *)checkclass(L, 1,
      lua_tostring(L, lua_upvalueindex(1))))->get();
    FnPtr fp = *(FnPtr *)lua_touserdata(L, lua_upvalueindex(2));
    arglist<params, 2> args(L);
    tdstack<Ret>::push(L, fnptr<FnPtr>::apply(obj, fp, args));
    return 1;
  }
};

template <typename FnPtr>
struct method_proxy <FnPtr, void>
{
  typedef typename fnptr<FnPtr>::classtype classtype;
  typedef typename fnptr<FnPtr>::params params;
  static int f (lua_State *L)
  {
    classtype *obj = ((shared_ptr<classtype> *)checkclass(L, 1,
      lua_tostring(L, lua_upvalueindex(1))))->get();
    FnPtr fp = *(FnPtr *)lua_touserdata(L, lua_upvalueindex(2));
    arglist<params, 2> args(L);
    fnptr<FnPtr>::apply(obj, fp, args);
    return 0;
  }
};

/*
* Perform method registration in a class.  The method proxies are all
* registered as values in the class's metatable, which is searched by the
* indexer function we've installed as __index metamethod.
*/

template <typename T>
template <typename FnPtr>
class__<T>& class__<T>::method (const char *name, FnPtr fp)
{
  assert(fnptr<FnPtr>::mfp);
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
  lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
  rawsetfield(L, -2, name);
  lua_pop(L, 1);
  return *this;
}

/*
* Lua-registerable C function templates for getting and setting the value of
* an object member through a member pointer; similiar to the global property
* proxies, but they take both the expected classname for type-checking and
* the member pointer as upvalues.
*/

template <typename T, typename U>
int m_propget_proxy (lua_State *L)
{
  T *obj = ((shared_ptr<T> *)checkclass(L, 1,
    lua_tostring(L, lua_upvalueindex(1))))->get();
  U T::* mp = *(U T::**)lua_touserdata(L, lua_upvalueindex(2));
  tdstack<U>::push(L, obj->*mp);
  return 1;
}

template <typename T, typename U>
int m_propset_proxy (lua_State *L)
{
  T *obj = ((shared_ptr<T> *)checkclass(L, 1,
    lua_tostring(L, lua_upvalueindex(1))))->get();
  U T::* mp = *(U T::**)lua_touserdata(L, lua_upvalueindex(2));
  obj->*mp = tdstack<U>::get(L, 2);
  return 0;
}

/* Property registration.  Properties are stored in the class's __propget
* metafield, with the property name as the get-function and property name
* + "_set" as the set-function.  Note that property getters are stored
* both in the regular metatable and the const metatable.
*/

template <typename T>
template <typename U>
class__<T>& class__<T>::property_ro (const char *name, const U T::* mp)
{
  luaL_getmetatable(L, this->name.c_str());
  std::string constname = "const " + this->name;
  luaL_getmetatable(L, constname.c_str());
  rawgetfield(L, -2, "__propget");
  rawgetfield(L, -2, "__propget");
  lua_pushstring(L, constname.c_str());
  void *v = lua_newuserdata(L, sizeof(U T::*));
  memcpy(v, &mp, sizeof(U T::*));
  lua_pushcclosure(L, &m_propget_proxy<T, U>, 2);
  lua_pushvalue(L, -1);
  rawsetfield(L, -3, name);
  rawsetfield(L, -3, name);
  lua_pop(L, 4);
  return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_ro (const char *name, U (T::*get) () const)
{
  luaL_getmetatable(L, this->name.c_str());
  std::string constname = "const " + this->name;
  luaL_getmetatable(L, constname.c_str());
  rawgetfield(L, -2, "__propget");
  rawgetfield(L, -2, "__propget");
  lua_pushstring(L, constname.c_str());
  typedef U (T::*FnPtr) () const;
  void *v = lua_newuserdata(L, sizeof(FnPtr));
  memcpy(v, &get, sizeof(FnPtr));
  lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
  lua_pushvalue(L, -1);
  rawsetfield(L, -3, name);
  rawsetfield(L, -3, name);
  lua_pop(L, 4);
  return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_rw (const char *name, U T::* mp)
{
  property_ro<U>(name, mp);
  luaL_getmetatable(L, this->name.c_str());
  rawgetfield(L, -1, "__propset");
  lua_pushstring(L, this->name.c_str());
  void *v = lua_newuserdata(L, sizeof(U T::*));
  memcpy(v, &mp, sizeof(U T::*));
  lua_pushcclosure(L, &m_propset_proxy<T, U>, 2);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

template <typename T>
template <typename U>
class__<T>& class__<T>::property_rw (const char *name,
  U (T::*get) () const, void (T::*set) (U))
{
  property_ro<U>(name, get);
  luaL_getmetatable(L, this->name.c_str());
  rawgetfield(L, -1, "__propset");
  lua_pushstring(L, this->name.c_str());
  typedef void (T::*FnPtr) (U);
  void *v = lua_newuserdata(L, sizeof(FnPtr));
  memcpy(v, &set, sizeof(FnPtr));
  lua_pushcclosure(L, &method_proxy<FnPtr>::f, 2);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

}


#endif
