// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT


#include "TestBase.h"

#include "LuaBridge/List.h"

#include <list>

struct ListTests : TestBase
{
};

TEST_F (ListTests, LuaRef)
{
  {
    runLua ("result = {1, 2, 3}");

    std::list <int> expected {1, 2, 3};
    std::list <int> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::list <int>> ());
  }

  {
    runLua ("result = {'a', 'b', 'c'}");

    std::list <std::string> expected {"a", "b", "c"};
    std::list <std::string> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::list <std::string> > ());
  }

  {
    runLua ("result = {1, 2.3, 'abc', false}");

    std::list <luabridge::LuaRef> expected {
      luabridge::LuaRef (L, 1),
      luabridge::LuaRef (L, 2.3),
      luabridge::LuaRef (L, "abc"),
      luabridge::LuaRef (L, false),
    };
    std::list <luabridge::LuaRef> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::list <luabridge::LuaRef> > ());
  }

  {
    runLua ("result = function (t) result = t end");

    std::list <int> list {1, 2, 3};
    result () (list); // Replaces result variable
    ASSERT_EQ (list, result ().cast <std::list <int> > ());
  }
}

TEST_F (ListTests, PassToFunction)
{
  runLua (
    "function foo (list) "
    "  result = list "
    "end");

  auto foo = luabridge::getGlobal (L, "foo");

  resetResult ();

  std::list <int> lvalue {10, 20, 30};
  foo (lvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (lvalue, result ().cast <std::list<int>> ());

  resetResult ();

  const std::list <int> constLvalue = lvalue;
  foo (constLvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (lvalue, result ().cast <std::list<int>> ());
}
