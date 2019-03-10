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

namespace {

struct Data
{
  /* explicit */ Data (int i) : i (i) {}

  int i;
};

bool operator== (const Data& lhs, const Data& rhs)
{
  return lhs.i == rhs.i;
}

std::ostream& operator<< (std::ostream& lhs, const Data& rhs)
{
  lhs << "{" << rhs.i << "}";
  return lhs;
}

std::vector <Data> processValues(const std::vector <Data>& data)
{
  return data;
}

std::vector <Data> processPointers(const std::vector <const Data*>& data)
{
  std::vector <Data> result;
  for (const auto* item : data)
  {
    result.emplace_back (*item);
  }
  return result;
}

} // namespace

TEST_F (VectorTests, PassFromLua)
{
  luabridge::getGlobalNamespace (L)
    .beginClass <Data> ("Data")
    .addConstructor <void (*) (int)> ()
    .endClass ()
    .addFunction ("processValues", &processValues)
    .addFunction ("processPointers", &processPointers);

  resetResult ();
  runLua ("result = processValues ({Data (-1), Data (2)})");

  ASSERT_EQ (
    std::vector <Data> ({-1, 2}),
    result().cast <std::vector <Data>>());

  resetResult ();
  runLua("result = processValues ({Data (-3), Data (4)})");

  ASSERT_EQ(
    std::vector <Data> ({-3, 4}),
    result().cast <std::vector <Data>>());
}
