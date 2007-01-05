// luatest.cpp
// Copyright (C) 2006 by Nathan Reed.  All rights and priveleges reserved.
// Test scaffold for building Lua interfaces.

#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <lua.hpp>
#include "tests.hpp"

using namespace std;

int traceback (lua_State *L);

int main (int argc, char **argv)
{
	// Create the Lua state
	lua_State *L = luaL_newstate();

	// Provide the base libraries
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	luaopen_debug(L);

	// Provide user libraries
	register_lua_funcs(L);

	// Put the traceback function on the stack
	lua_pushcfunction(L, &traceback);
	int errfunc_index = lua_gettop(L);

	// Execute lua files in order
	if (argc > 1)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (luaL_loadfile(L, argv[i]) != 0)
			{
				// compile-time error
				cerr << lua_tostring(L, -1) << endl;
				lua_close(L);
				return 1;
			}
			else if (lua_pcall(L, 0, 0, errfunc_index) != 0)
			{
				// runtime error
				cerr << lua_tostring(L, -1) << endl;
				lua_close(L);
				return 1;
			}
		}
	}
	else
	{
		cerr << "luatest: no input files.\n";
		lua_close(L);
		return 1;
	}

	lua_close(L);
	return 0;
}

// traceback function, adapted from lua.c
// when a runtime error occurs, this will append the call stack to the error message
int traceback (lua_State *L)
{
	// look up Lua's 'debug.traceback' function
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}
	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */
	return 1;
}
