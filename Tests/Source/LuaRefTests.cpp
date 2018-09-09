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

struct LuaRefTests : TestBase
{
};

TEST_F (LuaRefTests, Dictionary)
{
  runLua (
    "result = {"
    "  bool = true,"
    "  int = 5,"
    "  c = 3.14,"
    "  [true] = 'D',"
    "  [8] = 'abc',"
    "  fn = function (i) result = i end"
    "}");

  ASSERT_EQ (true, result () ["bool"].isBool ());
  ASSERT_EQ (true, result () ["bool"].cast <bool> ());

  ASSERT_EQ (true, result () ["int"].isNumber ());
  ASSERT_EQ (5u, result () ["int"].cast <unsigned char> ());
  ASSERT_EQ (5, result () ["int"].cast <short> ());
  ASSERT_EQ (5u, result () ["int"].cast <unsigned short> ());
  ASSERT_EQ (5, result () ["int"].cast <int> ());
  ASSERT_EQ (5u, result () ["int"].cast <unsigned int> ());
  ASSERT_EQ (5, result () ["int"].cast <long> ());
  ASSERT_EQ (5u, result () ["int"].cast <unsigned long> ());
  ASSERT_EQ (5, result () ["int"].cast <long long> ());
  ASSERT_EQ (5u, result () ["int"].cast <unsigned long long> ());

  ASSERT_EQ (true, result () ['c'].isNumber ());
  ASSERT_FLOAT_EQ (3.14f, result () ['c'].cast <float> ());
  ASSERT_DOUBLE_EQ (3.14, result () ['c'].cast <double> ());

  ASSERT_EQ (true, result () [true].isString ());
  ASSERT_EQ ('D', result () [true].cast <char> ());
  ASSERT_EQ ("D", result () [true].cast <std::string> ());
  ASSERT_STREQ ("D", result () [true].cast <const char*> ());

  ASSERT_EQ (true, result () [8].isString ());
  ASSERT_EQ ("abc", result () [8].cast <std::string> ());
  ASSERT_STREQ ("abc", result () [8].cast <char const*> ());

  ASSERT_EQ (true, result () ["fn"].isFunction ());
  auto fnResult = result () ["fn"] (42); // Replaces result variable
  ASSERT_TRUE (fnResult.isNil ());
  ASSERT_TRUE (result ().isNumber ());
  ASSERT_EQ (42, result ().cast <int> ());
}
