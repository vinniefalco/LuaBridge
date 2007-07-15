/*
 * luabridge.cpp - Copyright (C) 2007 by Nathan Reed
 * Source for the luabridge library.
 */

#include "../include/luabridge.hpp"
#include <cstdio>

#ifdef _MSC_VER
#	if (_MSC_VER >= 1400)
#		define snprintf _snprintf_s
#	else
#		define snprintf _snprintf
#	endif
#endif

/*
 * Default name for unknown types
 */

const char *luabridge::classname_unknown = "(unknown type)";

/*
 * Class type checker.  Given the index of a userdata on the stack, makes
 * sure that it's an object of the given classname or a subclass thereof.
 * If yes, returns the address of the data; otherwise, throws an error.
 * Works like the luaL_checkudata function.
 */

void *luabridge::checkclass (lua_State *L, int idx, const char *tname,
	bool exact)
{
	// If idx is relative to the top of the stack, convert it into an index
	// relative to the bottom of the stack, so we can push our own stuff
	if (idx < 0)
		idx += lua_gettop(L) + 1;

	// Check that the thing on the stack is indeed a userdata
	if (!lua_isuserdata(L, idx))
		luaL_typerror(L, idx, tname);

	// Lookup the given name in the registry
	luaL_getmetatable(L, tname);

	// Lookup the metatable of the given userdata
	lua_getmetatable(L, idx);
	
	// If exact match required, simply test for identity.
	if (exact)
	{
		// Ignore "const" for exact tests (which are used for destructors).
		if (!strncmp(tname, "const ", 6))
			tname += 6;

		if (lua_rawequal(L, -1, -2))
			return lua_touserdata(L, idx);
		else
		{
			// Generate an informative error message
			rawgetfield(L, -1, "__type");
			char buffer[256];
			snprintf(buffer, 256, "%s expected, got %s", tname,
				lua_tostring(L, -1));
			// luaL_argerror does not return
			luaL_argerror(L, idx, buffer);
			return 0;
		}
	}

	// Navigate up the chain of parents if necessary
	while (!lua_rawequal(L, -1, -2))
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
		if (lua_isnil(L, -1))
		{
			// Lookup the __type field of the original metatable, so we can
			// generate an informative error message
			lua_getmetatable(L, idx);
			rawgetfield(L, -1, "__type");

			char buffer[256];
			snprintf(buffer, 256, "%s expected, got %s", tname,
				lua_tostring(L, -1));
			// luaL_argerror does not return
			luaL_argerror(L, idx, buffer);
			return 0;
		}

		// Remove the old metatable from the stack
		lua_remove(L, -2);
	}
	
	// Found a matching metatable; return the userdata
	return lua_touserdata(L, idx);
}

/*
 * Index metamethod for C++ classes exposed to Lua.  This searches the
 * metatable for the given key, but if it can't find it, it looks for a
 * __parent key and delegates the lookup to that.
 */

int luabridge::subclass_indexer (lua_State *L)
{
	// Attempt to lookup the key in the metatable
	// (using a rawget so we don't invoke __index again)
	lua_getmetatable(L, 1);
	lua_pushvalue(L, 2);
	lua_rawget(L, -2);
	// Did we get a non-nil result?  If so, return it
	if (!lua_isnil(L, -1))
		return 1;

	// Look for a __props key
	lua_pop(L, 1);
	rawgetfield(L, -1, "__props");
	// If we found a __props key, look up the value in that
	if (!lua_isnil(L, -1))
	{
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		// If we got a non-nil result, call it and return its value(s)
		if (!lua_isnil(L, -1))
		{
			int top = lua_gettop(L);
			lua_pushvalue(L, 1);
			lua_call(L, 1, LUA_MULTRET);
			return lua_gettop(L) - top + 1;
		}
		lua_pop(L, 1);
	}
	
	// Look for a __const key
	lua_pop(L, 1);
	rawgetfield(L, -1, "__const");
	// If we found a __const key, do a raw lookup in that table
	if (!lua_isnil(L, -1))
	{
		lua_pushvalue(L, 2);
		lua_rawget(L, -2);
		if (!lua_isnil(L, -1))
			return 1;
		lua_pop(L, 1);
	}
	
	// Look for a __parent key
	lua_pop(L, 1);
	rawgetfield(L, -1, "__parent");
	// No __parent key?  Return nil
	if (lua_isnil(L, -1))
		return 1;
	// Found a __parent, send the lookup to it
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	return 1;
}

