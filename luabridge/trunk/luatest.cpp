// luatest.cpp
// Copyright (C) 2006 by Nathan Reed.  All rights and priveleges reserved.
// Test scaffold for building Lua interfaces.

#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <lua.hpp>
#include "luabridge.hpp"

using namespace std;

void hello ();
float pi ();
void godel (bool truth);
float square (float x);
std::string upperCase (const char *str);

void register_lua_funcs (lua_State *L);
int traceback (lua_State *L);

class B
{
	std::string name;
	friend class A;
public:
	B (const char *name_ = "unnamed_object"): name(name_)
	{
		cout << "B::B called on '" << name << "'\n";
	}
	~B ()
	{
		cout << "B::~B called on '" << name << "'\n";
	}
};

class A
{
	std::string name;
public:
	A (const char *name_ = "unnamed_object"): name(name_)
	{
		cout << "A::A called on '" << name << "'\n";
	}
	int foo (int x, const B &b)
	{
		cout << "A::foo called on '" << name << "', passed "
			<< x << " and '" << b.name << "'\n";
		return x*x;
	}
	int bar ()
	{
		return (int)name.length();
	}
	shared_ptr<A> baz (const char *name_)
	{
		return shared_ptr<A>(new A(name_));
	}
	~A ()
	{
		cout << "A::~A called on '" << name << "'\n";
	}
};

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

// some functions that will be registered to Lua
void hello ()
{
	cout << "Hello World!\n";
}

float pi ()
{
	return 3.141592654f;
}

void godel (bool truth)
{
	if (truth)
		cout << "Godel's formula is TRUE.  Therefore, there exists a proof of its FALSITY.\n";
	else
		cout << "Godel's formula is FALSE.  Therefore, there exists a proof of its TRUTH.\n";
}

float square (float x)
{
	return x*x;
}

std::string upperCase (const char *str)
{
	std::string ret;
	size_t len = strlen(str);
	ret.reserve(len);
	for (size_t i = 0; i < len; ++i)
		ret += (char)toupper(str[i]);
	return ret;
}

// add our own functions and stuff to the Lua environment
void register_lua_funcs (lua_State *L)
{
	luabridge::module m(L);

	m	.function("hello", &hello)
		.function("pi", &pi)
		.function("godel", &godel)
		.function("square", &square)
		.function("upperCase", &upperCase);
	m.class_<A>("A")
		.constructor<const char *>()
		.method("foo", &A::foo)
		.method("bar", &A::bar)
		.method("baz", &A::baz);
	m.class_<B>("B")
		.constructor<const char *>();
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
