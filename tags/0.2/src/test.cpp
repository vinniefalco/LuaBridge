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
	(void)argc;
	(void)argv;

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
	if (luaL_loadfile(L, "src/test.lua") != 0)
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

	lua_close(L);
	return 0;
}

// traceback function, adapted from lua.c
// when a runtime error occurs, this will append the call stack to the error message
int traceback (lua_State *L)
{
	// look up Lua's 'debug.traceback' function
	lua_getglobal(L, "debug");
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

bool g_success = true;

bool testSucceeded ()
{
	bool b = g_success;
	g_success = false;
	return b;
}

typedef int fn_type;
enum {
	FN_CTOR,
	FN_DTOR,
	FN_STATIC,
	FN_VIRTUAL,
	FN_PROPGET,
	FN_PROPSET,
	FN_STATIC_PROPGET,
	FN_STATIC_PROPSET,
	NUM_FN_TYPES
};

struct fn_called {
	bool called[NUM_FN_TYPES];
	fn_called () { memset(called, 0, NUM_FN_TYPES * sizeof(bool)); }
};

fn_called A_functions, B_functions;

bool testAFnCalled (fn_type f)
{
	bool b = A_functions.called[f];
	A_functions.called[f] = false;
	return b;
}

bool testBFnCalled (fn_type f)
{
	bool b = B_functions.called[f];
	B_functions.called[f] = false;
	return b;
}

class A
{
protected:
	string name;
	mutable bool success;
public:
	A (const string &name_): name(name_), success(false), testProp(47)
	{
		A_functions.called[FN_CTOR] = true;
	}
	virtual ~A ()
	{
		A_functions.called[FN_DTOR] = true;
	}

	virtual void testVirtual ()
	{
		A_functions.called[FN_VIRTUAL] = true;
	}

	const char * getName () const
	{
		return name.c_str();
	}

	void setSuccess () const
	{
		success = true;
	}

	bool testSucceeded () const
	{
		bool b = success;
		success = false;
		return b;
	}

	static void testStatic ()
	{
		A_functions.called[FN_STATIC] = true;
	}

	int testProp;
	int testPropGet () const
	{
		A_functions.called[FN_PROPGET] = true;
		return testProp;
	}
	void testPropSet (int x)
	{
		A_functions.called[FN_PROPSET] = true;
		testProp = x;
	}

	static int testStaticProp;
	static int testStaticPropGet ()
	{
		A_functions.called[FN_STATIC_PROPGET] = true;
		return testStaticProp;
	}
	static void testStaticPropSet (int x)
	{
		A_functions.called[FN_STATIC_PROPSET] = true;
		testStaticProp = x;
	}
};

int A::testStaticProp = 47;

class B: public A
{
public:
	B (const string &name_): A(name_)
	{
		B_functions.called[FN_CTOR] = true;
	}
	virtual ~B ()
	{
		B_functions.called[FN_DTOR] = true;
	}

	virtual void testVirtual ()
	{
		B_functions.called[FN_VIRTUAL] = true;
	}

	static void testStatic2 ()
	{
		B_functions.called[FN_STATIC] = true;
	}

};

/*
 * Test functions
 */

int testRetInt ()
{
	return 47;
}
float testRetFloat ()
{
	return 47.0f;
}
const char * testRetConstCharPtr ()
{
	return "Hello, world";
}
string testRetStdString ()
{
	static string ret("Hello, world");
	return ret;
}

void testParamInt (int a)
{
	g_success = (a == 47);
}
void testParamBool (bool b)
{
	g_success = b;
}
void testParamFloat (float f)
{
	g_success = (f == 47.0f);
}
void testParamConstCharPtr (const char *str)
{
	g_success = !strcmp(str, "Hello, world");
}
void testParamStdString (string str)
{
	g_success = !strcmp(str.c_str(), "Hello, world");
}
void testParamStdStringRef (const string &str)
{
	g_success = !strcmp(str.c_str(), "Hello, world");
}

void testParamAPtr (A * a)
{
	a->setSuccess();
}
void testParamAPtrConst (A * const a)
{
	a->setSuccess();
}
void testParamConstAPtr (const A * a)
{
	a->setSuccess();
}
void testParamSharedPtrA (luabridge::shared_ptr<A> a)
{
	a->setSuccess();
}
luabridge::shared_ptr<A> testRetSharedPtrA ()
{
	static luabridge::shared_ptr<A> sp_A(new A("from C"));
	return sp_A;
}
luabridge::shared_ptr<const A> testRetSharedPtrConstA ()
{
	static luabridge::shared_ptr<A> sp_A(new A("const A"));
	return sp_A;
}

// add our own functions and classes to a Lua environment
void register_lua_funcs (lua_State *L)
{
	luabridge::module m(L);

	m	.function("testSucceeded", &testSucceeded)
		.function("testAFnCalled", &testAFnCalled)
		.function("testBFnCalled", &testBFnCalled)
		.function("testRetInt", &testRetInt)
		.function("testRetFloat", &testRetFloat)
		.function("testRetConstCharPtr", &testRetConstCharPtr)
		.function("testRetStdString", &testRetStdString)
		.function("testParamInt", &testParamInt)
		.function("testParamBool", &testParamBool)
		.function("testParamFloat", &testParamFloat)
		.function("testParamConstCharPtr", &testParamConstCharPtr)
		.function("testParamStdString", &testParamStdString)
		.function("testParamStdStringRef", &testParamStdStringRef);

	m.class_<A>("A")
		.constructor<void (*) (const string &)>()
		.method("testVirtual", &A::testVirtual)
		.method("getName", &A::getName)
		.method("testSucceeded", &A::testSucceeded)
		.property_rw("testProp", &A::testProp)
		.property_rw("testProp2", &A::testPropGet, &A::testPropSet)
		.static_method("testStatic", &A::testStatic)
		.static_property_rw("testStaticProp", &A::testStaticProp)
		.static_property_rw("testStaticProp2", &A::testStaticPropGet,
			&A::testStaticPropSet);

	m.subclass<B, A>("B")
		.constructor<void (*) (const string &)>()
		.static_method("testStatic2", &B::testStatic2);

	m	.function("testParamAPtr", &testParamAPtr)
		.function("testParamAPtrConst", &testParamAPtrConst)
		.function("testParamConstAPtr", &testParamConstAPtr)
		.function("testParamSharedPtrA", &testParamSharedPtrA)
		.function("testRetSharedPtrA", &testRetSharedPtrA)
		.function("testRetSharedPtrConstA", &testRetSharedPtrConstA);
}

