//==============================================================================
/*
  https://github.com/vinniefalco/LuaBridge

  Copyright 2018, Dmitry Tarakanov <dmitry.a.tarakanov@gmail.com>

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

#include "TestBase.h"

#include "LuaBridge/Map.h"

#include <map>

struct MapTests : TestBase
{
};

TEST_F (MapTests, LuaRef)
{
  {
    runLua ("result = {[false] = true, a = 'abc', [1] = 5, [3.14] = -1.1}");

    using Map = std::map <luabridge::LuaRef, luabridge::LuaRef>;
    Map expected {
      {luabridge::LuaRef (L, false), luabridge::LuaRef (L, true)},
      {luabridge::LuaRef (L, 'a'), luabridge::LuaRef (L, "abc")},
      {luabridge::LuaRef (L, 1), luabridge::LuaRef (L, 5)},
      {luabridge::LuaRef (L, 3.14), luabridge::LuaRef (L, -1.1)},
    };
    Map actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <Map> ());
  }

  {
    runLua ("result = {'a', 'b', 'c'}");

    using Int2Char = std::map <int, char>;
    Int2Char expected {{1, 'a'}, {2, 'b'}, {3, 'c'}};
    Int2Char actual = result ();
    ASSERT_EQ (expected, actual);
    ASSERT_EQ (expected, result ().cast <Int2Char> ());
  }
}

TEST_F (MapTests, PassToFunction)
{
  runLua (
    "function foo (map) "
    "  result = map "
    "end");

  auto foo = luabridge::getGlobal (L, "foo");
  using Int2Bool = std::map <int, bool>;

  // resetResult ();

  Int2Bool lvalue {{10, false}, {20, true}, {30, true}};
  // foo (lvalue);
  // ASSERT_TRUE (result ().isTable ());
  // ASSERT_EQ (lvalue, result ().cast <Int2Bool> ());

  resetResult ();

  const Int2Bool constLvalue = lvalue;
  foo (constLvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (constLvalue, result ().cast <Int2Bool> ());
}
