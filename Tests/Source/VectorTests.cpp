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

#include "LuaBridge/Vector.h"

struct VectorTests : TestBase
{
};

TEST_F (VectorTests, LuaRef)
{
  {
    runLua ("result = {1, 2, 3}");

    std::vector <int> expected {1, 2, 3};
    std::vector <int> vector = result ();
    ASSERT_EQ (expected, vector);
    ASSERT_EQ (expected, result ().cast <std::vector <int>> ());
  }

  {
    runLua ("result = {'a', 'b', 'c'}");

    std::vector <std::string> expected {"a", "b", "c"};
    std::vector <std::string> vector = result ();
    ASSERT_EQ (expected, vector);
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
    std::vector <luabridge::LuaRef> vector = result ();
    ASSERT_EQ (expected, vector);
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
