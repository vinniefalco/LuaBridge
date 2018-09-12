// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/Vector.h"

#include <vector>

struct VectorTests : TestBase
{
};

TEST_F (VectorTests, LuaRef)
{
  {
    runLua ("result = {1, 2, 3}");

    std::vector <int> expected {1, 2, 3};
    std::vector <int> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::vector <int>> ());
  }

  {
    runLua ("result = {'a', 'b', 'c'}");

    std::vector <std::string> expected {"a", "b", "c"};
    std::vector <std::string> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::vector <std::string> > ());
  }

  {
    runLua ("result = {1, 2.3, 'abc', false}");

    std::vector <luabridge::LuaRef> expected {
      luabridge::LuaRef (L, 1),
      luabridge::LuaRef (L, 2.3),
      luabridge::LuaRef (L, "abc"),
      luabridge::LuaRef (L, false),
    };
    std::vector <luabridge::LuaRef> actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <std::vector <luabridge::LuaRef> > ());
  }

  {
    runLua ("result = function (t) result = t end");

    std::vector <int> vector {1, 2, 3};
    result () (vector); // Replaces result variable
    ASSERT_EQ (vector, result ().cast <std::vector <int> > ());
  }
}

TEST_F (VectorTests, PassToFunction)
{
  runLua (
    "function foo (vector) "
    "  result = vector "
    "end");

  auto foo = luabridge::getGlobal (L, "foo");

  resetResult ();

  std::vector <int> lvalue {10, 20, 30};
  foo (lvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (lvalue, result ().cast <std::vector<int>> ());

  resetResult ();

  const std::vector <int> constLvalue = lvalue;
  foo (constLvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (lvalue, result ().cast <std::vector<int>> ());
}
