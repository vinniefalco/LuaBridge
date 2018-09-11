//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2018, Dmitry Tarakanov <dmitry.a.tarakanov@gmail.com>

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

#pragma once

#include <LuaBridge/detail/Stack.h>

#include <list>

namespace luabridge {

template <class T>
struct Stack <std::list <T> >
{
  static void push(lua_State* L, std::list <T> const& list)
  {
    lua_createtable (L, list.size (), 0);
    typename std::list <T>::const_iterator item = list.begin();
    for (std::size_t i = 1; i <= list.size (); ++i)
    {
      lua_pushinteger (L, static_cast <lua_Integer> (i));
      Stack <T>::push (L, *item);
      lua_settable (L, -3);
      ++item;
    }
  }

  static std::list <T> get(lua_State* L, int index)
  {
    if (!lua_istable(L, index))
    {
      luaL_error(L, "#%d argments must be table", index);
    }

    std::list <T> list;

    int const absindex = lua_absindex (L, index);
    lua_pushnil (L);
    while (lua_next (L, absindex) != 0)
    {
      list.push_back (Stack <T>::get (L, -1));
      lua_pop (L, 1);
    }
    return list;
  }
};

template <class T>
struct Stack <std::list <T> const&> : Stack <std::list <T> >
{
};

} // namespace luabridge
