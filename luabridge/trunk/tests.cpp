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
	A (const string &name_): name(name_)
	{
		cout << "A::A(\"" << name << "\")\n";
	}
	~A ()
	{
		cout << "A(\"" << name << "\")::~A\n";
	}

	int testInt (int i) //const
	{
		cout << "A(\"" << name << "\")::testInt(" << i << ")\n";
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
		cout << "B::B(\"" << name << "\")\n";
	}
	~B ()
	{
		cout << "B(\"" << name << "\")::~B\n";
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
	cout << "testAPtr(A(\"" << a->getName() << "\"))\n";
}
void testAPtrConst (A * const a)
{
	cout << "testAPtrConst(A(\"" << a->getName() << "\"))\n";
}
void testConstAPtr (const A * a)
{
	cout << "testConstAPtr(A(\"" << const_cast<A*>(a)->getName() << "\"))\n";
}
shared_ptr<A> testSharedPtrA (shared_ptr<A> a)
{
	cout << "testSharedPtrA(A(\"" << a->getName() << "\"))\n";
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

	m.class_<B>("B")
		.constructor<const string &>();
		/*.method("testInt", &A::testInt)
		.method("getName", &A::getName);*/

	m	.function("testAPtr", &testAPtr)
		.function("testAPtrConst", &testAPtrConst)
		.function("testConstAPtr", &testConstAPtr)
		.function("testSharedPtrA", &testSharedPtrA);
}

