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

#include "TestBase.h"

#include "LuaBridge/Vector.h"

struct VectorTests : TestBase
{
};

TEST_F (VectorTests, LuaRef)
{
  {
    runLua ("result = {1, 2, 3}");

    std::vector <int> expected;
    expected.push_back (1);
    expected.push_back (2);
    expected.push_back (3);
    std::vector <int> vec = result ();
    ASSERT_EQ (expected, vec);
    ASSERT_EQ (expected, result ().cast <std::vector <int>> ());
  }

  {
    runLua ("result = {'a', 'b', 'c'}");

    std::vector <std::string> expected;
    expected.push_back ("a");
    expected.push_back ("b");
    expected.push_back ("c");
    std::vector <std::string> vec = result ();
    ASSERT_EQ (expected, vec);
    ASSERT_EQ (expected, result ().cast <std::vector <std::string> > ());
  }

  {
    runLua ("result = {1, 2.3, 'abc', false}");

    std::vector <luabridge::LuaRef> expected;
    expected.push_back (luabridge::LuaRef (L, 1));
    expected.push_back (luabridge::LuaRef (L, 2.3));
    expected.push_back (luabridge::LuaRef (L, "abc"));
    expected.push_back (luabridge::LuaRef (L, false));
    std::vector <luabridge::LuaRef> vec = result ();
    ASSERT_EQ (expected, vec);
    ASSERT_EQ (expected, result ().cast <std::vector <luabridge::LuaRef> > ());
  }

  {
    runLua ("result = function (t) result = t end");

    std::vector <int> vec;
    vec.push_back (1);
    vec.push_back (2);
    vec.push_back (3);
    result () (vec); // Replaces result variable
    ASSERT_EQ (vec, result ().cast <std::vector <int> > ());
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

  std::vector <int> lvalue{ 10, 20, 30 };
  foo (lvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (
    std::vector <int> ({ 10, 20, 30 }),
    result ().cast <std::vector<int>> ());

  resetResult ();

  const std::vector <int> constLvalue{ 10, 20, 30 };
  foo (constLvalue);
  ASSERT_TRUE (result ().isTable ());
  ASSERT_EQ (
    std::vector <int> ({ 10, 20, 30 }),
    result ().cast <std::vector<int>> ());
}
