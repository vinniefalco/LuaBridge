//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright 2008, Nigel Atkinson <suprapilot+LuaCode@gmail.com>

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

// Basic "ANY" class for representing Lua objects.
// Used in calling Lua functions as parameters.

/** "Any" class to represent Lua types.
*/
class LuaVal
{
  int m_type;
  double m_doubleValue;
  std::string m_stringValue;
  lua_CFunction m_functionValue;
  LuaRef *m_object;

public:
  LuaVal()
    : m_type (LUA_TNIL)
    , m_object (0)
  {
  }

  LuaVal (double n)
    : m_doubleValue (n)
    , m_type (LUA_TNUMBER)
    , m_object (0)
  {
  }

  LuaVal (std::string s)
    : m_stringValue (s)
    , m_type (LUA_TSTRING)
    , m_object (0)
  {
  }

  LuaVal (char const* s )
    : m_stringValue (s)
    , m_type (LUA_TSTRING)
    , m_object (0)
  {
  }

  LuaVal (lua_CFunction f)
    : m_functionValue (f)
    , m_type (LUA_TFUNCTION)
    , m_object (0)
  {
  }

  LuaVal (LuaRef *o);
	
  LuaVal (LuaVal const& lv);

  ~LuaVal ();

  void push (lua_State* L);
};
