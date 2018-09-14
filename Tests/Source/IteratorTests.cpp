// https://github.com/vinniefalco/LuaBridge
//
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "LuaBridge/detail/Iterator.h"

struct IteratorTests : TestBase
{
};

TEST_F (IteratorTests, BasicIteration)
{
  runLua (
    "result = {"
    "  bool = true,"
    "  int = 5,"
    "  c = 3.14,"
    "  [true] = 'D',"
    "  [8] = 'abc',"
    "  fn = function (i)"
    "    result = i + 1"
    "  end"
    "}");

    std::map <luabridge::LuaRef, luabridge::LuaRef> expected {
        {{L, "bool"}, {L, true}},
        {{L, "int"}, {L, 5}},
        {{L, 'c'}, {L, 3.14}},
        {{L, true}, {L, 'D'}},
        {{L, 8}, {L, "abc"}},
        {{L, "fn"}, {L, result () ["fn"]}},
    };

    std::map <luabridge::LuaRef, luabridge::LuaRef> actual;

    for (luabridge::Iterator iterator (result ()); !iterator.isNil (); ++iterator)
    {
        actual.emplace(iterator.key (), iterator.value());
    }

    ASSERT_EQ (expected, actual);
}
