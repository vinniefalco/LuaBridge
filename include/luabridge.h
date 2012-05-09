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

#ifndef LUABRIDGE_LUABRIDGE_HEADER
#define LUABRIDGE_LUABRIDGE_HEADER

#include <cassert>
#include <string>

#ifndef USE_OTHER_SHARED_PTR
#include "shared_ptr.h"
#endif

namespace luabridge
{

#include "impl/typelist.h"
#include "impl/stack.h"

// forward declaration
template <typename T>
class class__;

//==============================================================================

/** Get a value, bypassing metamethods.
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
    Create a static table for a non-global scope.

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

  /**
    Look up a static table.

    The table is identified by its fully qualified dot-separated name. The
    resulting table is returned on the stack.

    @invariant The table must exist.
  */
  static void findStaticTable (lua_State* const L, char const* const name)
  {
    lua_getglobal (L, "_G");

    if (name && name[0] != '\0')
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

    @note This must be registered as a closure with the actual
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

  /**
    lua_CFunction for a function signature and a void return type.

    @note This must be registered as a closure with the actual
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
    Construct a scope in the specified namespace.

    Namespaces are separated with dots, for example "x.y" would create
    the "x" table in the global environment containing a child table "y".
  */
  scope (lua_State *L_, const char *name_) : L (L_), name (name_)
  {
    assert (name.length () > 0);

    lua_getglobal (L, "_G");

    // Process each dot-separated namespace identifier.
    size_t start = 0;
    size_t pos = 0;
    while ((pos = name.find ('.', start)) != std::string::npos)
    {
      /** @todo This is broken when there is more than one dot-separated
                component in the scope.
      */
      lua_getfield (L, -1, name.substr(start, pos - start).c_str());
      if (lua_isnil (L, -1))
      {
        lua_pop (L, 1);
        util::createStaticTable (L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, name.c_str() + start);
      }
      lua_remove(L, -2);
      start = pos + 1;
    }

    // Create a new table with the remaining portion of the name.
    util::createStaticTable (L);
    lua_setfield (L, -2, name.c_str() + start);
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
  // Variable registration.  Variables can be read/write (rw)
  // or read-only (ro).  Varieties that access pointers directly
  // and varieties that access through function calls are provided.

  template <typename T>
  scope& variable_ro (const char *name, const T *data);
  template <typename T>
  scope& variable_ro (const char *name, T (*get) ());
  template <typename T>
  scope& variable_rw (const char *name, T *data);
  template <typename T>
  scope& variable_rw (const char *name, T (*get) (), void (*set) (T));

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

#include "impl/scope.h"
#include "impl/class.h"
}


#endif
