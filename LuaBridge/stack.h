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

/*
* Type-dispatch functions for manipulating the Lua stack.
*/

/*
* Generic functions for data with value semantics: these make a copy on
* the heap of the data being passed, and stick it in a shared_ptr so Lua
* scripts can keep references to it.
* NOTE: for type-checking purposes, the data must be of a class or struct
* type registered with Lua, although it is allowed to be an opaque type.
* NOTE: data with reference semantics, i.e. pointers and references
* to data, are handled differently - see the specializations below.
*/

template <typename T>
struct tdstack
{
public:
  static void push (lua_State *L, T data)
  {
    // Make sure we don't try to push objects of
    // unregistered classes or primitive types
    assert(classname<T>::name() != classname_unknown);

    // Allocate a new userdata and construct the pointer in-place there
    void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
    new(block) shared_ptr<T>(new T(data));

    // Set the userdata's metatable
    luaL_getmetatable(L, classname<T>::name());
    lua_setmetatable(L, -2);
  }
  static T get (lua_State *L, int index)
  {
    // Make sure we don't try to retrieve objects of
    // unregistered classes or primitive types
    assert(classname<T>::name() != classname_unknown);

    return *((shared_ptr<T> *)
      checkclass(L, index, classname<T>::name()))->get();
  }
};

/*
* Pointers and references: getting is done by retrieving the address from
* the Lua-owned shared_ptr, but pushing is not allowed since luabridge
* has no idea of the ownership semantics of these objects.  You can only
* push shared_ptrs, not naked pointers and references.
*/

template <typename T>
struct tdstack <T*>
{
private:
  static void push (lua_State *L, T *data);
public:
  static T* get (lua_State *L, int index)
  {
    return ((shared_ptr<T> *)
      checkclass(L, index, classname<T>::name()))->get();
  }
};

template <typename T>
struct tdstack <const T *>
{
private:
  static void push (lua_State *L, const T *data);
public:
  static const T* get (lua_State *L, int index)
  {
    std::string constname = std::string("const ") + classname<T>::name();
    return ((shared_ptr<const T> *)
      checkclass(L, index, constname.c_str()))->get();
  }
};

template <typename T>
struct tdstack <T* const>
{
private:
  static void push (lua_State *L, T * const data);
public:
  static T* const get (lua_State *L, int index)
  {
    return ((shared_ptr<T> *)
      checkclass(L, index, classname<T>::name()))->get();
  }
};

template <typename T>
struct tdstack <const T* const>
{
private:
  static void push (lua_State *L, const T * const data);
public:
  static const T* const get (lua_State *L, int index)
  {
    std::string constname = std::string("const ") + classname<T>::name();
    return ((shared_ptr<const T> *)
      checkclass(L, index, constname.c_str()))->get();
  }
};

template <typename T>
struct tdstack <T&>
{
private:
  static void push (lua_State *L, T &data);
public:
  static T& get (lua_State *L, int index)
  {
    return *((shared_ptr<T> *)
      checkclass(L, index, classname<T>::name()))->get();
  }
};

template <typename T>
struct tdstack <const T&>
{
private:
  static void push (lua_State *L, const T &data);
public:
  static const T& get (lua_State *L, int index)
  {
    std::string constname = std::string("const ") + classname<T>::name();
    return *((shared_ptr<const T> *)
      checkclass(L, index, constname.c_str()))->get();
  }
};

/*
* shared_ptr: we can push these.
* There is a specialization for const types, which produces a Lua userdata
* whose metatable is the class's const metatable.
*/

template <typename T>
struct tdstack <shared_ptr<T> >
{
  static void push (lua_State *L, shared_ptr<T> data)
  {
    // Make sure we don't try to push ptrs to objects of
    // unregistered classes or primitive types
    assert(classname<T>::name() != classname_unknown);

    // Allocate a new userdata and construct the pointer in-place there
    void *block = lua_newuserdata(L, sizeof(shared_ptr<T>));
    new(block) shared_ptr<T>(data);

    // Set the userdata's metatable
    luaL_getmetatable(L, classname<T>::name());
    lua_setmetatable(L, -2);
  }
  static shared_ptr<T> get (lua_State *L, int index)
  {
    // Make sure we don't try to retrieve ptrs to objects of
    // unregistered classes or primitive types
    assert(classname<T>::name() != classname_unknown);

    return *(shared_ptr<T> *)
      checkclass(L, index, classname<T>::name());
  }
};

template <typename T>
struct tdstack <shared_ptr<const T> >
{
  static void push (lua_State *L, shared_ptr<const T> data)
  {
    // Make sure we don't try to push ptrs to objects of
    // unregistered classes or primitive types
    assert(classname<T>::name() != classname_unknown);

    // Allocate a new userdata and construct the pointer in-place there
    void *block = lua_newuserdata(L, sizeof(shared_ptr<const T>));
    new(block) shared_ptr<const T>(data);

    // Set the userdata's metatable
    std::string constname = std::string("const ") + classname<T>::name();
    luaL_getmetatable(L, constname.c_str());
    lua_setmetatable(L, -2);
  }
  static shared_ptr<const T> get (lua_State *L, int index)
  {
    std::string constname = std::string("const ") + classname<T>::name();
    return *(shared_ptr<const T> *)
      checkclass(L, index, constname.c_str());
  }
};

/*
* Primitive types, including const char * and std::string
*/

// Create a macro for handling numeric types,
// since they follow the same pattern

#define TDSTACK_NUMERIC(T) \
  template <> \
struct tdstack <T> \
{ \
  static void push (lua_State *L, T data) \
{ \
  lua_pushnumber(L, (lua_Number)data); \
} \
  static T get (lua_State *L, int index) \
{ \
  return (T)(luaL_checknumber(L, index)); \
} \
}

TDSTACK_NUMERIC(int);
TDSTACK_NUMERIC(unsigned int);
TDSTACK_NUMERIC(unsigned char);
TDSTACK_NUMERIC(short);
TDSTACK_NUMERIC(unsigned short);
TDSTACK_NUMERIC(long);
TDSTACK_NUMERIC(unsigned long);
TDSTACK_NUMERIC(float);
TDSTACK_NUMERIC(double);

#undef TDSTACK_NUMERIC

template <>
struct tdstack <bool>
{
  static void push (lua_State *L, bool data)
  {
    lua_pushboolean(L, data ? 1 : 0);
  }
  static bool get (lua_State *L, int index)
  {
    luaL_checktype(L, index, LUA_TBOOLEAN);
    // In MSC, disable "bool to int conversion" warning
#		ifdef _MSC_VER
#			pragma warning (push)
#			pragma warning (disable: 4800)
#		endif

    return (bool)lua_toboolean(L, index);

#		ifdef _MSC_VER
#			pragma warning (pop)
#		endif
  }
};

template <>
struct tdstack <char>
{
  static void push (lua_State *L, char data)
  {
    char str[2] = { data, 0 };
    lua_pushstring(L, str);
  }
  static char get (lua_State *L, int index)
  {
    return luaL_checkstring(L, index)[0];
  }
};

template <>
struct tdstack <const char *>
{
  static void push (lua_State *L, const char *data)
  {
    lua_pushstring(L, data);
  }
  static const char *get (lua_State *L, int index)
  {
    return luaL_checkstring(L, index);
  }
};

template <>
struct tdstack <std::string>
{
  static void push (lua_State *L, const std::string &data)
  {
    lua_pushstring(L, data.c_str());
  }
  static std::string get (lua_State *L, int index)
  {
    return std::string(luaL_checkstring(L, index));
  }
};

template <>
struct tdstack <const std::string &>
{
  static void push (lua_State *L, const std::string &data)
  {
    lua_pushstring(L, data.c_str());
  }
  static std::string get (lua_State *L, int index)
  {
    return std::string(luaL_checkstring(L, index));
  }
};

/*
* Subclass of a type/value list, constructable from the Lua stack.
*/

template <typename Typelist, int start = 1>
struct arglist {};

template <int start>
struct arglist <nil, start>:
  public typevallist<nil>
{
  arglist (lua_State *L) { (void)L; }
};

template <typename Head, typename Tail, int start>
struct arglist <typelist<Head, Tail>, start>:
  public typevallist<typelist<Head, Tail> >
{
  arglist (lua_State *L):
typevallist<typelist<Head, Tail> >
  (tdstack<Head>::get(L, start),
  arglist<Tail, start + 1>(L))
{}
};

