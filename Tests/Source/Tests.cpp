//==============================================================================
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright (C) 2007, Nathan Reed

  License: The MIT License (http://www.opensource.org/licenses/mit-license.php)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
//==============================================================================

// A set of tests of different types' communication with Lua

#include "Lua/LuaLibrary.h"

#include "LuaBridge/LuaBridge.h"
#include "LuaBridge/RefCountedPtr.h"

#include "JuceLibraryCode/BinaryData.h"

#include <gtest/gtest.h>

#include <cstring>
#include <string>

namespace LuaBridgeTests
{

using namespace std;
using namespace luabridge;

// traceback function, adapted from lua.c
// when a runtime error occurs, this will append the call stack to the error message
//
static int traceback (lua_State* L)
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

//==============================================================================

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
  FN_OPERATOR,
  NUM_FN_TYPES
};

struct fn_called {
  bool called [NUM_FN_TYPES];
  fn_called () { memset(called, 0, NUM_FN_TYPES * sizeof(bool)); }
};

fn_called A_functions, B_functions;

bool testAFnCalled (fn_type f)
{
  bool b = A_functions.called[f];
  A_functions.called [f] = false;
  return b;
}

bool testBFnCalled (fn_type f)
{
  bool b = B_functions.called[f];
  B_functions.called [f] = false;
  return b;
}

class A
{
protected:
  string name;
  mutable bool success;
public:
  A (string const& name_) : name (name_), success (false), testProp (47)
  {
    A_functions.called [FN_CTOR] = true;
  }
  virtual ~A ()
  {
    A_functions.called [FN_DTOR] = true;
  }

  virtual void testVirtual ()
  {
    A_functions.called [FN_VIRTUAL] = true;
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
    A_functions.called [FN_STATIC] = true;
  }

  int testProp;
  int testPropGet () const
  {
    A_functions.called [FN_PROPGET] = true;
    return testProp;
  }
  void testPropSet (int x)
  {
    A_functions.called [FN_PROPSET] = true;
    testProp = x;
  }

  static int testStaticProp;
  static int testStaticPropGet ()
  {
    A_functions.called [FN_STATIC_PROPGET] = true;
    return testStaticProp;
  }
  static void testStaticPropSet (int x)
  {
    A_functions.called [FN_STATIC_PROPSET] = true;
    testStaticProp = x;
  }
  
  RefCountedPtr <A> operator + (A const& other)
  {
    A_functions.called [FN_OPERATOR] = true;
    return new A (name + " + " + other.name);
  }
};

int A::testStaticProp = 47;

class B: public A
{
public:
  explicit B (string const& name_) : A (name_)
  {
    B_functions.called [FN_CTOR] = true;
  }

  virtual ~B ()
  {
    B_functions.called [FN_DTOR] = true;
  }

  virtual void testVirtual ()
  {
    B_functions.called [FN_VIRTUAL] = true;
  }

  static void testStatic2 ()
  {
    B_functions.called [FN_STATIC] = true;
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

char const* testRetConstCharPtr ()
{
  return "Hello, world";
}

string testRetStdString ()
{
  static string ret ("Hello, world");
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

void testParamConstCharPtr (char const* str)
{
  g_success = !strcmp (str, "Hello, world");
}

void testParamStdString (string str)
{
  g_success = !strcmp (str.c_str(), "Hello, world");
}

void testParamStdStringRef (const string &str)
{
  g_success = !strcmp (str.c_str(), "Hello, world");
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

void testParamSharedPtrA (RefCountedPtr <A> a)
{
  a->setSuccess();
}

RefCountedPtr <A> testRetSharedPtrA ()
{
  static RefCountedPtr <A> sp_A (new A("from C"));
  return sp_A;
}

RefCountedPtr <A const> testRetSharedPtrConstA ()
{
  static RefCountedPtr <A> sp_A (new A("const A"));
  return sp_A;
}

// add our own functions and classes to a Lua environment
void addToState (lua_State *L)
{
  getGlobalNamespace (L)
    .addFunction ("testSucceeded", &testSucceeded)
    .addFunction ("testAFnCalled", &testAFnCalled)
    .addFunction ("testBFnCalled", &testBFnCalled)
    .addFunction ("testRetInt", &testRetInt)
    .addFunction ("testRetFloat", &testRetFloat)
    .addFunction ("testRetConstCharPtr", &testRetConstCharPtr)
    .addFunction ("testRetStdString", &testRetStdString)
    .addFunction ("testParamInt", &testParamInt)
    .addFunction ("testParamBool", &testParamBool)
    .addFunction ("testParamFloat", &testParamFloat)
    .addFunction ("testParamConstCharPtr", &testParamConstCharPtr)
    .addFunction ("testParamStdString", &testParamStdString)
    .addFunction ("testParamStdStringRef", &testParamStdStringRef)
    .beginClass <A> ("A")
      .addConstructor <void (*) (const string &), RefCountedPtr <A> > ()
      .addFunction ("testVirtual", &A::testVirtual)
      .addFunction ("getName", &A::getName)
      .addFunction ("testSucceeded", &A::testSucceeded)
      .addFunction ("__add", &A::operator+)
      .addData ("testProp", &A::testProp)
      .addProperty ("testProp2", &A::testPropGet, &A::testPropSet)
      .addStaticFunction ("testStatic", &A::testStatic)
      .addStaticData ("testStaticProp", &A::testStaticProp)
      .addStaticProperty ("testStaticProp2", &A::testStaticPropGet, &A::testStaticPropSet)
    .endClass ()
    .deriveClass <B, A> ("B")
      .addConstructor <void (*) (const string &), RefCountedPtr <B> > ()
      .addStaticFunction ("testStatic2", &B::testStatic2)
    .endClass ()
    .addFunction ("testParamAPtr", &testParamAPtr)
    .addFunction ("testParamAPtrConst", &testParamAPtrConst)
    .addFunction ("testParamConstAPtr", &testParamConstAPtr)
    .addFunction ("testParamSharedPtrA", &testParamSharedPtrA)
    .addFunction ("testRetSharedPtrA", &testRetSharedPtrA)
    .addFunction ("testRetSharedPtrConstA", &testRetSharedPtrConstA)
  ;
}

void resetTests ()
{
  g_success = true;
  A::testStaticProp = 47;
}

void printValue (lua_State* L, int index)
{
  int type = lua_type (L, index);
  switch (type)
  {
    case LUA_TBOOLEAN:
      std::cerr << std::boolalpha << (lua_toboolean (L, index) != 0);
      break;
    case LUA_TSTRING:
      std::cerr << lua_tostring (L, index);
      break;
    case LUA_TNUMBER:
      std::cerr << lua_tonumber (L, index);
      break;
    case LUA_TTABLE:
    case LUA_TTHREAD:
    case LUA_TFUNCTION:
      std::cerr << lua_topointer (L, index);
      break;
  }
  std::cerr << ": " << lua_typename (L, type) << " (" << type << ")" << std::endl;
}

void printStack (lua_State* L)
{
  std::cerr << "===== Stack =====\n";
  for (int i = 1; i <= lua_gettop (L); ++i)
  {
    std::cerr << "@" << i << " = ";
    printValue (L, i);
  }
}

struct LuaBridgeTest : public ::testing::Test
{
  lua_State* L = nullptr;
  int errorFunctionRef = 0;


  void SetUp () override
  {
    L = nullptr;
    L = luaL_newstate ();
    luaL_openlibs (L);
    lua_pushcfunction (L, &traceback);
  }

  void TearDown () override
  {
    lua_close (L);
  }

  void runLua (const std::string& script)
  {
    luabridge::setGlobal (L, luabridge::LuaRef(L), "result");

    if (luaL_loadstring (L, script.c_str()) != 0)
    {
      throw std::runtime_error (lua_tostring (L, -1));
    }

    if (lua_pcall (L, 0, 0, -2) != 0)
    {
      throw std::runtime_error (lua_tostring (L, -1));
    }
  }

  luabridge::LuaRef result()
  {
    return luabridge::getGlobal (L, "result");
  }
};

template <class T>
T identityCFunction(T value)
{
  return value;
}

TEST_F (LuaBridgeTest, CFunction)
{
  luabridge::getGlobalNamespace(L)
    .addFunction ("boolFn", &identityCFunction <bool>)
    .addFunction ("shortFn", &identityCFunction <short>)
    .addFunction ("ushortFn", &identityCFunction <unsigned short>)
    .addFunction ("intFn", &identityCFunction <int>)
    .addFunction ("uintFn", &identityCFunction <unsigned int>)
    .addFunction ("longFn", &identityCFunction <long>)
    .addFunction ("ulongFn", &identityCFunction <unsigned long>)
    .addFunction ("longlongFn", &identityCFunction <long long>)
    .addFunction ("ulonglongFn", &identityCFunction <unsigned long long>)
    .addFunction ("floatFn", &identityCFunction <float>)
    .addFunction ("doubleFn", &identityCFunction <double>)
    .addFunction ("charFn", &identityCFunction <char>)
    .addFunction ("cstringFn", &identityCFunction <const char*>)
    .addFunction ("stringFn", &identityCFunction <std::string>)
  ;

  {
    runLua ("result = boolFn (false)");
    ASSERT_EQ (true, result().isBool ());
    ASSERT_EQ (false, result().cast <bool> ());
  }
  {
    runLua ("result = boolFn (true)");
    ASSERT_EQ (true, result().isBool ());
    ASSERT_EQ (true, result().cast <bool> ());
  }

  {
    runLua ("result = shortFn (-32768)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (-32768, result().cast <int> ());
  }

  {
    runLua ("result = ushortFn (32767)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (32767, result().cast <unsigned int> ());
  }
  {
    runLua ("result = intFn (-500)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (-500, result().cast <int> ());
  }

  {
    runLua ("result = uintFn (42)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (42, result().cast <unsigned int> ());
  }

  {
    runLua ("result = longFn (-8000)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (-8000, result().cast <long> ());
  }

  {
    runLua ("result = ulongFn (9000)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (9000, result().cast <unsigned long> ());
  }

  {
    runLua ("result = longlongFn (-8000)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (-8000, result().cast <long long> ());
  }

  {
    runLua ("result = ulonglongFn (9000)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_EQ (9000, result().cast <unsigned long long> ());
  }

  {
    runLua ("result = floatFn (3.14)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_FLOAT_EQ (3.14, result().cast <float> ());
  }

  {
    runLua ("result = doubleFn (-12.3)");
    ASSERT_EQ (true, result().isNumber ());
    ASSERT_FLOAT_EQ (-12.3, result().cast <double> ());
  }

  {
    runLua ("result = charFn ('a')");
    ASSERT_EQ (true, result().isString ());
    ASSERT_EQ ('a', result().cast <char> ());
  }

  {
    runLua ("result = cstringFn ('abc')");
    ASSERT_EQ (true, result().isString ());
    ASSERT_STREQ ("abc", result().cast <const char*> ());
  }

  {
    runLua ("result = stringFn ('lua')");
    ASSERT_EQ (true, result().isString ());
    ASSERT_EQ ("lua", result().cast <std::string> ());
  }
}

}
