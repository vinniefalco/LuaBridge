//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge

  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright 2007, Nathan Reed

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

//zero 06-jan-2013
//in order to interoperate with LNUM...
#if LUA_TINT
#define IPUSHER lua_pushinteger
#define ILUATYPE lua_Integer
#define ICHECKER luaL_checkinteger
#else
#define IPUSHER lua_pushnumber
#define ILUATYPE lua_Number
#define ICHECKER luaL_checknumber
#endif

//------------------------------------------------------------------------------
/**
    Receive the lua_State* as an argument.
*/
template <>
struct Stack <lua_State*>
{
  static lua_State* get (lua_State* L, int)
  {
    return L;
  }
};

//------------------------------------------------------------------------------
/**
    Push a lua_CFunction.
*/
template <>
struct Stack <lua_CFunction>
{
  static void push (lua_State* L, lua_CFunction f)
  {
    lua_pushcfunction (L, f);
  }

  static lua_CFunction get (lua_State* L, int index)
  {
    return lua_tocfunction (L, index);
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `int`.
*/
template <>
struct Stack <int>
{
  static inline void push (lua_State* L, int value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline int get (lua_State* L, int index)
  {
    return static_cast <int > (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned int`.
*/
template <>
struct Stack <unsigned int>
{
  static inline void push (lua_State* L, unsigned int value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline unsigned int get (lua_State* L, int index)
  {
    return static_cast <unsigned int > (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned char`.
*/
template <>
struct Stack <unsigned char>
{
  static inline void push (lua_State* L, unsigned char value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline unsigned char get (lua_State* L, int index)
  {
    return static_cast <unsigned char> (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `short`.
*/
template <>
struct Stack <short>
{
  static inline void push (lua_State* L, short value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline short get (lua_State* L, int index)
  {
    return static_cast <short> (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned short`.
*/
template <>
struct Stack <unsigned short>
{
  static inline void push (lua_State* L, unsigned short value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline unsigned short get (lua_State* L, int index)
  {
    return static_cast <unsigned short> (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `long`.
*/
template <>
struct Stack <long>
{
  static inline void push (lua_State* L, long value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline long get (lua_State* L, int index)
  {
    return static_cast <long> (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `unsigned long`.
*/
template <>
struct Stack <unsigned long>
{
  static inline void push (lua_State* L, unsigned long value)
  {
    IPUSHER (L, static_cast <ILUATYPE> (value));
  }
  
  static inline unsigned long get (lua_State* L, int index)
  {
    return static_cast <unsigned long> (ICHECKER (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `float`.
*/
template <>
struct Stack <float>
{
  static inline void push (lua_State* L, float value)
  {
    lua_pushnumber (L, static_cast <lua_Number> (value));
  }
  
  static inline float get (lua_State* L, int index)
  {
    return static_cast <float> (luaL_checknumber (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `double`.
*/
template <> struct Stack <double>
{
  static inline void push (lua_State* L, double value)
  {
    lua_pushnumber (L, static_cast <lua_Number> (value));
  }
  
  static inline double get (lua_State* L, int index)
  {
    return static_cast <double> (luaL_checknumber (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `bool`.
*/
template <>
struct Stack <bool> {
  static inline void push (lua_State* L, bool value)
  {
    lua_pushboolean (L, value ? 1 : 0);
  }
  
  static inline bool get (lua_State* L, int index)
  {
    return lua_toboolean (L, index) ? true : false;
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `char`.
*/
template <>
struct Stack <char>
{
  static inline void push (lua_State* L, char value)
  {
    char str [2] = { value, 0 };
    lua_pushstring (L, str);
  }
  
  static inline char get (lua_State* L, int index)
  {
    return luaL_checkstring (L, index) [0];
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `float`.
*/
template <>
struct Stack <char const*>
{
  static inline void push (lua_State* L, char const* str)
  {
    if (str != 0)
      lua_pushstring (L, str);
    else
      lua_pushnil (L);
  }

  static inline char const* get (lua_State* L, int index)
  {
    return lua_isnil (L, index) ? 0 : luaL_checkstring (L, index);
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `std::string`.
*/
template <>
struct Stack <std::string>
{
  static inline void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str ());
  }

  static inline std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};

//------------------------------------------------------------------------------
/**
    Stack specialization for `std::string const&`.
*/
template <>
struct Stack <std::string const&>
{
  static inline void push (lua_State* L, std::string const& str)
  {
    lua_pushstring (L, str.c_str());
  }

  static inline std::string get (lua_State* L, int index)
  {
    return std::string (luaL_checkstring (L, index));
  }
};
