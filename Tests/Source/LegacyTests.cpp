// https://github.com/vinniefalco/LuaBridge
// Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
// Copyright 2007, Nathan Reed
// SPDX-License-Identifier: MIT

// A set of tests of different types' communication with Lua

#include "TestBase.h"

#include "LuaBridge/RefCountedPtr.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

namespace LuaBridgeTests {

using namespace std;
using namespace luabridge;

//==============================================================================

/*
 * Test classes
 */

bool g_success = true;

bool testSucceeded()
{
    bool b = g_success;
    g_success = false;
    return b;
}

typedef int fn_type;
enum
{
    FN_CTOR,
    FN_DTOR,
    FN_STATIC,
    FN_VIRTUAL,
    FN_PROPGET,
    FN_PROPSET,
    FN_STATIC_PROPGET,
    FN_STATIC_PROPSET,
    FN_OPERATOR,
    NUM_FN_TYPES
};

struct fn_called
{
    bool called[NUM_FN_TYPES];
    fn_called() { memset(called, 0, NUM_FN_TYPES * sizeof(bool)); }
};

fn_called A_functions, B_functions;

bool testAFnCalled(fn_type f)
{
    bool b = A_functions.called[f];
    A_functions.called[f] = false;
    return b;
}

bool testBFnCalled(fn_type f)
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
    A(string const& name_) : name(name_), success(false), testProp(47)
    {
        A_functions.called[FN_CTOR] = true;
    }
    virtual ~A() { A_functions.called[FN_DTOR] = true; }

    virtual void testVirtual() { A_functions.called[FN_VIRTUAL] = true; }

    const char* getName() const { return name.c_str(); }

    void setSuccess() const { success = true; }

    bool testSucceeded() const
    {
        bool b = success;
        success = false;
        return b;
    }

    static void testStatic() { A_functions.called[FN_STATIC] = true; }

    int testProp;
    int testPropGet() const
    {
        A_functions.called[FN_PROPGET] = true;
        return testProp;
    }
    void testPropSet(int x)
    {
        A_functions.called[FN_PROPSET] = true;
        testProp = x;
    }

    static int testStaticProp;
    static int testStaticPropGet()
    {
        A_functions.called[FN_STATIC_PROPGET] = true;
        return testStaticProp;
    }
    static void testStaticPropSet(int x)
    {
        A_functions.called[FN_STATIC_PROPSET] = true;
        testStaticProp = x;
    }

    RefCountedPtr<A> operator+(A const& other)
    {
        A_functions.called[FN_OPERATOR] = true;
        return new A(name + " + " + other.name);
    }
};

int A::testStaticProp = 47;

class B : public A
{
public:
    explicit B(string const& name_) : A(name_) { B_functions.called[FN_CTOR] = true; }

    virtual ~B() { B_functions.called[FN_DTOR] = true; }

    virtual void testVirtual() { B_functions.called[FN_VIRTUAL] = true; }

    static void testStatic2() { B_functions.called[FN_STATIC] = true; }
};

/*
 * Test functions
 */

int testRetInt()
{
    return 47;
}

float testRetFloat()
{
    return 47.0f;
}

char const* testRetConstCharPtr()
{
    return "Hello, world";
}

string testRetStdString()
{
    static string ret("Hello, world");
    return ret;
}

void testParamInt(int a)
{
    g_success = (a == 47);
}

void testParamBool(bool b)
{
    g_success = b;
}

void testParamFloat(float f)
{
    g_success = (f == 47.0f);
}

void testParamConstCharPtr(char const* str)
{
    g_success = !strcmp(str, "Hello, world");
}

void testParamStdString(string str)
{
    g_success = !strcmp(str.c_str(), "Hello, world");
}

void testParamStdStringRef(const string& str)
{
    g_success = !strcmp(str.c_str(), "Hello, world");
}

void testParamAPtr(A* a)
{
    a->setSuccess();
}

void testParamAPtrConst(A* const a)
{
    a->setSuccess();
}

void testParamConstAPtr(const A* a)
{
    a->setSuccess();
}

void testParamSharedPtrA(RefCountedPtr<A> a)
{
    a->setSuccess();
}

RefCountedPtr<A> testRetSharedPtrA()
{
    /*static*/ RefCountedPtr<A> sp_A(new A("from C"));
    return sp_A;
}

RefCountedPtr<A const> testRetSharedPtrConstA()
{
    /*static*/ RefCountedPtr<A> sp_A(new A("const A"));
    return sp_A;
}

// add our own functions and classes to a Lua environment
void addToState(lua_State* L)
{
    getGlobalNamespace(L)
        .addFunction("testSucceeded", &testSucceeded)
        .addFunction("testAFnCalled", &testAFnCalled)
        .addFunction("testBFnCalled", &testBFnCalled)
        .addFunction("testRetInt", &testRetInt)
        .addFunction("testRetFloat", &testRetFloat)
        .addFunction("testRetConstCharPtr", &testRetConstCharPtr)
        .addFunction("testRetStdString", &testRetStdString)
        .addFunction("testParamInt", &testParamInt)
        .addFunction("testParamBool", &testParamBool)
        .addFunction("testParamFloat", &testParamFloat)
        .addFunction("testParamConstCharPtr", &testParamConstCharPtr)
        .addFunction("testParamStdString", &testParamStdString)
        .addFunction("testParamStdStringRef", &testParamStdStringRef)
        .beginClass<A>("A")
        .addConstructor<void (*)(const string&), RefCountedPtr<A>>()
        .addFunction("testVirtual", &A::testVirtual)
        .addFunction("getName", &A::getName)
        .addFunction("testSucceeded", &A::testSucceeded)
        .addFunction("__add", &A::operator+)
        .addData("testProp", &A::testProp)
        .addProperty("testProp2", &A::testPropGet, &A::testPropSet)
        .addStaticFunction("testStatic", &A::testStatic)
        .addStaticData("testStaticProp", &A::testStaticProp)
        .addStaticProperty("testStaticProp2", &A::testStaticPropGet, &A::testStaticPropSet)
        .endClass()
        .deriveClass<B, A>("B")
        .addConstructor<void (*)(const string&), RefCountedPtr<B>>()
        .addStaticFunction("testStatic2", &B::testStatic2)
        .endClass()
        .addFunction("testParamAPtr", &testParamAPtr)
        .addFunction("testParamAPtrConst", &testParamAPtrConst)
        .addFunction("testParamConstAPtr", &testParamConstAPtr)
        .addFunction("testParamSharedPtrA", &testParamSharedPtrA)
        .addFunction("testRetSharedPtrA", &testRetSharedPtrA)
        .addFunction("testRetSharedPtrConstA", &testRetSharedPtrConstA);
}

void resetTests()
{
    g_success = true;
    A::testStaticProp = 47;
}

void printValue(lua_State* L, int index)
{
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TBOOLEAN:
        std::cerr << std::boolalpha << (lua_toboolean(L, index) != 0);
        break;
    case LUA_TSTRING:
        std::cerr << lua_tostring(L, index);
        break;
    case LUA_TNUMBER:
        std::cerr << lua_tonumber(L, index);
        break;
    case LUA_TTABLE:
    case LUA_TTHREAD:
    case LUA_TFUNCTION:
        std::cerr << lua_topointer(L, index);
        break;
    }
    std::cerr << ": " << lua_typename(L, type) << " (" << type << ")" << std::endl;
}

} // namespace LuaBridgeTests

struct LegacyTests : TestBase
{
};

const char* const SCRIPT = R"(
-- test lua script to be run with the luabridge test program

print('Running LuaBridge tests:');

-- enum from C++
FN_CTOR = 0
FN_DTOR = 1
FN_STATIC = 2
FN_VIRTUAL = 3
FN_PROPGET = 4
FN_PROPSET = 5
FN_STATIC_PROPGET = 6
FN_STATIC_PROPSET = 7
FN_OPERATOR = 8
NUM_FN_TYPES = 9

-- function to print contents of a table
function printtable (t)
  for k, v in pairs(t) do
    if (type(v) == 'table') then
      print(k .. ' =>', '(table)');
    elseif (type(v) == 'function') then
      print(k .. ' =>', '(function)');
    elseif (type(v) == 'userdata') then
      print(k .. ' =>', '(userdata)');
    else
      print(k .. ' =>', v);
    end
  end
end

function assert (expr)
  if (not expr) then error('assert failed', 2) end
end

-- test functions registered from C++

assert(testSucceeded());
assert(testRetInt() == 47);
assert(testRetFloat() == 47.0);
assert(testRetConstCharPtr() == 'Hello, world');
assert(testRetStdString() == 'Hello, world');

testParamInt(47);                       assert(testSucceeded());
testParamBool(true);                    assert(testSucceeded());
testParamFloat(47.0);                   assert(testSucceeded());
testParamConstCharPtr('Hello, world');  assert(testSucceeded());
testParamStdString('Hello, world');     assert(testSucceeded());
testParamStdStringRef('Hello, world');  assert(testSucceeded());

-- test static methods of classes registered from C++

A.testStatic();             assert(testAFnCalled(FN_STATIC));
B.testStatic();             assert(testAFnCalled(FN_STATIC));
B.testStatic2();            assert(testBFnCalled(FN_STATIC));

-- test static properties of classes registered from C++

assert(A.testStaticProp == 47);
assert(A.testStaticProp2 == 47);assert(testAFnCalled(FN_STATIC_PROPGET));
A.testStaticProp = 48;          assert(A.testStaticProp == 48);
A.testStaticProp2 = 49;         assert(testAFnCalled(FN_STATIC_PROPSET) and A.testStaticProp2 == 49);

-- test classes registered from C++

object1 = A('object1');          assert(testAFnCalled(FN_CTOR));
object1:testVirtual();           assert(testAFnCalled(FN_VIRTUAL));

object2 = B('object2');         assert(testAFnCalled(FN_CTOR) and testBFnCalled(FN_CTOR));
object2:testVirtual();          assert(testBFnCalled(FN_VIRTUAL) and not testAFnCalled(FN_VIRTUAL));

-- test functions taking and returning objects

testParamAPtr(object1);          assert(object1:testSucceeded());
testParamAPtrConst(object1);     assert(object1:testSucceeded());
testParamConstAPtr(object1);     assert(object1:testSucceeded());
testParamSharedPtrA(object1);    assert(object1:testSucceeded());

testParamAPtr(object2);          assert(object2:testSucceeded());
testParamAPtrConst(object2);     assert(object2:testSucceeded());
testParamConstAPtr(object2);     assert(object2:testSucceeded());
testParamSharedPtrA(object2);    assert(object2:testSucceeded());

result = testRetSharedPtrA();    assert(result:getName() == 'from C');

-- test constness

constA = testRetSharedPtrConstA();    assert(constA:getName() == 'const A');
assert(constA.testVirtual == nil);
testParamConstAPtr(constA);        assert(constA:testSucceeded());
assert(pcall(testParamAPtr, constA) == false, 'attempt to call nil value');

-- test properties

assert(object1.testProp == 47);
assert(object1.testProp2 == 47);    assert(testAFnCalled(FN_PROPGET));
assert(object2.testProp == 47);
assert(object2.testProp2 == 47);    assert(testAFnCalled(FN_PROPGET));

object1.testProp = 48;          assert(object1.testProp == 48);
object1.testProp2 = 49;          assert(testAFnCalled(FN_PROPSET) and object1.testProp2 == 49);

-- test operator overload
object1a = object1 + object1;      assert(testAFnCalled(FN_OPERATOR));
assert(object1a:getName() == 'object1 + object1');

print('All tests succeeded.');
)";

TEST_F(LegacyTests, AllTests)
{
    LuaBridgeTests::addToState(L);

    // Execute lua files in order
    if (luaL_loadstring(L, SCRIPT) != 0)
    {
        // compile-time error
        FAIL() << lua_tostring(L, -1);
    }
    if (lua_pcall(L, 0, 0, -2) != 0)
    {
        // runtime error
        FAIL() << lua_tostring(L, -1);
    }
}
