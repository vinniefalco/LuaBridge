// https://github.com/vinniefalco/LuaBridge
// Copyright 2022, Stefan Frings
// SPDX-License-Identifier: MIT

#pragma once

#include <LuaBridge/detail/Stack.h>

#include <cassert>
#include <utility>

namespace luabridge {

template<class T1, class T2>
struct Stack<std::pair<T1, T2>>
{
    static void push(lua_State* L, std::pair<T1, T2> const& pair)
    {
        lua_createtable(L, 2, 0);
        lua_pushinteger(L, static_cast<lua_Integer>(1));
        Stack<T1>::push(L, pair.first);
        lua_settable(L, -3);
        lua_pushinteger(L, static_cast<lua_Integer>(2));
        Stack<T2>::push(L, pair.second);
        lua_settable(L, -3);
    }

    static std::pair<T1, T2> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        std::size_t const tableSize = static_cast<std::size_t>(get_length(L, index));

        if (tableSize != 2)
        {
            luaL_error(L, "pair size must be 2");
        }

        std::pair<T1, T2> pair;

        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);

        {
            int const next = lua_next(L, absindex);
            assert(next != 0);

            pair.first = Stack<T1>::get(L, -1);
            lua_pop(L, 1);
        }

        {
            int const next = lua_next(L, absindex);
            assert(next != 0);

            pair.second = Stack<T2>::get(L, -1);
            lua_pop(L, 1);
        }

        {
            int const next = lua_next(L, absindex);
            assert(next == 0);
        }

        return pair;
    }

    static bool isInstance(lua_State* L, int index)
    {
        return lua_istable(L, index) && get_length(L, index) == 2;
    }
};

} // namespace luabridge
