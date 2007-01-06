// tests.hpp
// Copyright (C) 2007 by Nathan Reed.  All rights and priveleges reserved.
// A set of tests of different types' communication with Lua

#include <iostream>
#include <string>
#include "luabridge.hpp"
#include "tests.hpp"

using namespace std;

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
		.constructor<const string &>()
		.method("testInt", &A::testInt)
		.method("getName", &A::getName);

	m.subclass<B, A>("B")
		.constructor<const string &>();

	m.class_<C>("C")
		.constructor<const string &>()
		.method("getName", &C::getName);

	m	.function("testAPtr", &testAPtr)
		.function("testAPtrConst", &testAPtrConst)
		.function("testConstAPtr", &testConstAPtr)
		.function("testSharedPtrA", &testSharedPtrA);
}

