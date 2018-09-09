//==============================================================================
/*
  https://github.com/vinniefalco/LuaBridge

  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright 2007, Nathan Reed

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

#include "TestBase.h"

struct IssueTests : TestBase
{
};

struct AbstractClass
{
  virtual int sum (int a, int b) = 0;
};

struct ConcreteClass : AbstractClass
{
  int sum (int a, int b) override
  {
    return a + b;
  }

  static AbstractClass& get()
  {
    static ConcreteClass instance;
    return instance;
  }
};

TEST_F (IssueTests, Issue87)
{
  luabridge::getGlobalNamespace (L)
    .beginClass <AbstractClass> ("Class")
    .addFunction ("sum", &AbstractClass::sum)
    .endClass ()
    .addFunction ("getAbstractClass", &ConcreteClass::get);

  runLua ("c = getAbstractClass():sum (1, 2)");
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (3, result ().cast <int> ());
}

TEST_F (IssueTests, Issue121)
{
  runLua (R"(
    first = {
      second = {
        actual = "data"
      }
    }
  )");
  auto first = luabridge::getGlobal (L, "first");
  ASSERT_TRUE (first.isTable ());
  ASSERT_EQ (0, first.length ());
  ASSERT_TRUE (first ["second"].isTable ());
  ASSERT_EQ (0, first ["second"].length ());
}

void pushArgs (lua_State*)
{
}

template <class Arg, class... Args>
void pushArgs (lua_State* L, Arg arg, Args... args)
{
  luabridge::Stack <Arg>::push (L, arg);
  pushArgs (L, args...);
}

template <class... Args>
std::vector <luabridge::LuaRef> callFunction (const luabridge::LuaRef& function, Args... args)
{
  assert (function.isFunction ());

  lua_State* L = function.state ();
  int originalTop = lua_gettop (L);
  function.push (L);
  pushArgs (L, args...);

  luabridge::LuaException::pcall (L, sizeof... (args), LUA_MULTRET);

  std::vector <luabridge::LuaRef> results;
  int top = lua_gettop (L);
  results.reserve (top - originalTop);
  for (int i = originalTop + 1; i <= top; ++i)
  {
    results.push_back (luabridge::LuaRef::fromStack (L, i));
  }
  return results;
}

TEST_F (IssueTests, Issue160)
{
  runLua (
    "function isConnected (arg1, arg2) "
    " return 1, 'srvname', 'ip:10.0.0.1', arg1, arg2 "
    "end");

  luabridge::LuaRef f_isConnected = luabridge::getGlobal (L, "isConnected");

  auto v = callFunction (f_isConnected, 2, "abc");
  ASSERT_EQ (5, v.size ());
  ASSERT_EQ (1, v [0].cast <int> ());
  ASSERT_EQ ("srvname", v [1].cast <std::string> ());
  ASSERT_EQ ("ip:10.0.0.1", v [2].cast <std::string> ());
  ASSERT_EQ (2, v [3].cast <int> ());
  ASSERT_EQ ("abc", v [4].cast <std::string> ());
}

TEST_F (IssueTests, Issue161)
{
  runLua ("t = { aa=1,bb=2, othert={1,2,3} }");

  luabridge::LuaRef t = luabridge::getGlobal (L, "t");
  luabridge::LuaRef othert = t ["othert"];
  ASSERT_TRUE (t ["othert"].isTable ());
}
