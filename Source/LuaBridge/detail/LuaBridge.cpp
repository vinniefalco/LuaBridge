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

// These have to be defined after LuaRef as one needs to know about the "push"
// method and the others need the size of LuaRef.

#if 0

LuaVal::LuaVal (LuaRef* o)
  : m_type (LUA_TLIGHTUSERDATA)
{
  m_object = new LuaRef (o);
}

LuaVal::~LuaVal ()
{
  if (m_object)
    delete m_object;
}

LuaVal::LuaVal (LuaVal const& rhs)
  : m_type (rhs.m_type)
{
  switch (rhs.m_type)
  {
  case LUA_TNIL:
    break;

  case LUA_TNUMBER:
    m_doubleValue = rhs.m_doubleValue;
    break;

  case LUA_TSTRING:
    m_stringValue = rhs.m_stringValue;
    break;

  case LUA_TFUNCTION:
    m_functionValue = rhs.m_functionValue;
    break;

  case LUA_TLIGHTUSERDATA:
    m_object = new LuaRef (rhs.m_object);
    break;

  default:
    throw std::runtime_error( "LuaVal to be copied got messed up somehow." );
  }
}

void LuaVal::push (lua_State* L)
{
  switch( m_type )
  {
  case LUA_TNIL:
    lua_pushnil (L);
    break;

  case LUA_TNUMBER:
    lua_pushnumber (L, m_doubleValue);
    break;

  case LUA_TSTRING:
    lua_pushstring (L, m_stringValue.c_str ());
    break;

  case LUA_TFUNCTION:
    lua_pushcfunction (L, m_functionValue);
    break;

  case LUA_TLIGHTUSERDATA:
    m_object->push ();
    break;

  default:
    throw LuaException( L, "LuaVal got messed up somehow.", __FILE__, __LINE__ );
  }
}

std::ostream& operator<< (std::ostream &os, LuaRef& ref)
{
  switch (ref.type ())
  {
  case LUA_TNIL:
    os << "NIL";
    break;

  case LUA_TNUMBER:
    os << (double) ref;
    break;

  case LUA_TSTRING:
    os << std::string (ref);
    break;

  case LUA_TFUNCTION:
    os << "function";
    break;

  case LUA_TLIGHTUSERDATA:
    os << "A reference to another object";
    break;
  }

  return os;
}

#endif
