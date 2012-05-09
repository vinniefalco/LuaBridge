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

#include "typelist.h"
#include "stack.h"

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
    Produce an error message.

    luaL_typerror was removed in Lua 5.2 so we provide it here.
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

  //----------------------------------------------------------------------------
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

#include "scope.h"

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
  lookup_static_table(L, name_);
  lookup_static_table(L, basename);
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
  lookup_static_table(L, name.c_str());

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
#	pragma warning (push)
#	pragma warning (disable: 4127)
#endif
  if (fnptr<FnPtr>::const_mfp)
    metatable_name.insert(0, "const ");
#ifdef _MSC_VER
#	pragma warning (pop)
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
