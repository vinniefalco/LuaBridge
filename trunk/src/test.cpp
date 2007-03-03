// tests.hpp
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.
// A set of tests of different types' communication with Lua

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <lua.hpp>
#include <string>
#include <vector>
#include "../include/luabridge.hpp"

using namespace std;

int traceback (lua_State *L);
void register_lua_funcs (lua_State *L);

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

/*
 * Test classes
 */
class A
{
protected:
	string name;
public:
	A (const string &name_)
	{
		name = "A(\"" + name_ + "\")";
		cout << "A::" << name << "\n";
	}
	virtual ~A ()
	{
		cout << name << "::~A\n";
	}

	virtual int testInt (int i) //const
	{
		cout << name << "::testInt(" << i << ")\n";
		return i;
	}

	const char * getName () //const
	{
		return name.c_str();
	}
};

class B: public A
{
public:
	B (const string &name_): A(name_)
	{
		name = "B(\"" + name_ + "\")";
		cout << "B::" << name << "\n";
	}
	virtual ~B ()
	{
		cout << name << "::~B\n";
	}

	virtual int testInt (int i) //const
	{
		cout << name << "::testInt(" << i << ")\n";
		return i;
	}
};

class C
{
protected:
	string name;
public:
	C (const string &name_)
	{
		name = "C(\"" + name_ + "\")";
		cout << "C::" << name << "\n";
	}
	virtual ~C ()
	{
		cout << name << "::~C\n";
	}

	const char * getName () //const
	{
		return name.c_str();
	}
};

/*
 * Test functions
 */
void testVoid ()
{
	cout << "testVoid()\n";
}
int testInt (int i)
{
	cout << "testInt(" << i << ")\n";
	return i;
}
float testFloat (float f)
{
	cout << "testFloat(" << f << ")\n";
	return f;
}
const char * testConstCharPtr (const char *str)
{
	cout << "testConstCharPtr(\"" << str << "\")\n";
	return str;
}
string testStdString (const string &str)
{
	cout << "testStdString(\"" << str << "\")\n";
	return str;
}

void testAPtr (A * a)
{
	cout << "testAPtr(" << a->getName() << ")\n";
}
void testAPtrConst (A * const a)
{
	cout << "testAPtrConst(" << a->getName() << ")\n";
}
void testConstAPtr (const A * a)
{
	cout << "testConstAPtr(" << const_cast<A*>(a)->getName() << ")\n";
}
luabridge::shared_ptr<A> testSharedPtrA (luabridge::shared_ptr<A> a)
{
	cout << "testSharedPtrA(" << a->getName() << ")\n";
	return a;
}

// add our own functions and classes to a Lua environment
void register_lua_funcs (lua_State *L)
{
	luabridge::module m(L);

	m	.function("testVoid", &testVoid)
		.function("testInt", &testInt)
		.function("testFloat", &testFloat)
		.function("testConstCharPtr", &testConstCharPtr)
		.function("testStdString", &testStdString);

	m.class_<A>("A")
		.constructor<void (*) (const string &)>()
		.method("testInt", &A::testInt)
		.method("getName", &A::getName);

	m.subclass<B, A>("B")
		.constructor<void (*) (const string &)>();

	m.class_<C>("C")
		.constructor<void (*) (const string &)>()
		.method("getName", &C::getName);

	m	.function("testAPtr", &testAPtr)
		.function("testAPtrConst", &testAPtrConst)
		.function("testConstAPtr", &testConstAPtr)
		.function("testSharedPtrA", &testSharedPtrA);
}

