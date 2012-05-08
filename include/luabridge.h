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

template <typename T> class class__;

// Convenience functions: like lua_getfield and lua_setfield, but raw
inline void rawgetfield (lua_State *L, int idx, const char *key)
{
  lua_pushstring(L, key);
  if (idx < 0) --idx;
  lua_rawget(L, idx);
}
inline void rawsetfield (lua_State *L, int idx, const char *key)
{
  lua_pushstring(L, key);
  lua_insert(L, -2);
  if (idx < 0) --idx;
  lua_rawset(L, idx);
}

//==============================================================================
/**
  Registration manager.

  Performs registration tasks for a specified Lua state.

  @todo namespace support.
*/
class scope
{
public:
  scope (lua_State *L_, const char *name_ = "");

  // Function registration

  template <typename FnPtr>
  scope& function (const char *name, FnPtr fp);

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

private:
  /*
  * Index metamethod for C++ classes exposed to Lua.  This searches the
  * metatable for the given key, but if it can't find it, it looks for a
  * __parent key and delegates the lookup to that.
  */

  static int indexer (lua_State *L)
  {
    lua_getmetatable(L, 1);

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

  /*
  * Newindex metamethod for supporting properties on scopes and static
  * properties of classes.
  */

  static int newindexer (lua_State *L)
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
  */
  void create_static_table (lua_State *L) // [-0, +1]
  {
    lua_newtable (L);

    // Set it as its own metatable
    lua_pushvalue (L, -1);
    lua_setmetatable (L, -2);

    // Set indexer as the __index metamethod
    lua_pushcfunction(L, &indexer);
    rawsetfield(L, -2, "__index");

    // Set newindexer as the __newindex metamethod
    lua_pushcfunction(L, &newindexer);
    rawsetfield(L, -2, "__newindex");

    // Create the __propget and __propset metafields as empty tables
    lua_newtable(L);
    rawsetfield(L, -2, "__propget");
    lua_newtable(L);
    rawsetfield(L, -2, "__propset");
  }

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
void *checkclass (lua_State *L, int idx, const char *tname,
  bool exact = false);
//int indexer (lua_State *L);
//int newindexer (lua_State *L);
int m_newindexer (lua_State *L);
void create_static_table (lua_State *L);
void lookup_static_table (lua_State *L, const char *name);

// Predeclare classname struct since several implementation files use it
template <typename T>
struct classname;
extern const char *classname_unknown;

  // Include implementation files
#include "impl/typelist.h"
#include "impl/stack.h"
#include "impl/scope.h"
#include "impl/class.h"
}


#endif
