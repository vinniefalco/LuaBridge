/*
 * luabridge.cpp - Copyright (C) 2007 by Nathan Reed
 * Source for the luabridge library.
 */

#include "../include/luabridge.hpp"

/*
 * Default name for unknown types
 */

const char *luabridge::classname_unknown = "(unknown type)";

/*
 * Class type checker.  Given the index of a userdata on the stack, makes
 * sure that it's an object of the given classname or a subclass thereof.
 * If yes, returns the address of the data; otherwise, throws an error.
 * Works like the luaL_checkudata function, but better.
 */

void *luabridge::checkclass (lua_State *L, int idx, const char *tname)
{
	// If idx is relative to the top of the stack, convert it into an index
	// relative to the bottom of the stack, so we can push our own stuff
	if (idx < 0)
		idx = lua_gettop(L) + idx + 1;

	// Check that the thing on the stack is indeed a userdata
	if (!lua_isuserdata(L, idx))
		luaL_typerror(L, idx, tname);

	// Lookup the given name in the registry
	luaL_getmetatable(L, tname);

	// Lookup the metatable of the given userdata
	lua_getmetatable(L, idx);

	// Navigate up the chain of parents if necessary
	while (!lua_rawequal(L, -1, -2))
	{
		// Look for the metatable's parent field
		lua_getfield(L, -1, "__parent");

		// No parent field?  We've failed; generate appropriate error
		if (lua_isnil(L, -1))
		{
			// Lookup the __type field of the original metatable, so we can
			// generate an informative error message
			lua_getmetatable(L, idx);
			lua_getfield(L, -1, "__type");

			char buffer[256];
			sprintf(buffer, "%s expected, got %s", tname,
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
	lua_getmetatable(L, 1);
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	// Did we get a non-nil result?  If so, return it
	if (!lua_isnil(L, -1))
		return 1;
	// Look for a __parent key
	lua_pop(L, 1);
	lua_getfield(L, -1, "__parent");
	// No __parent key?  Return nil
	if (lua_isnil(L, -1))
		return 1;
	// Found a __parent, send the lookup to it
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	return 1;
}

