//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>

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

  This code incorporates ideas from Nigel Atkinson.
*/
//==============================================================================

#ifndef LUABRIDGE_LUAVAL_HEADER
#define LUABRIDGE_LUAVAL_HEADER

// Basic "ANY" class for representing Lua objects.
// Used in calling Lua functions as parameters.
class LuaVal
{
  int mType;
  double d;
  std::string str;
  lua_CFunction func;
  LuaRef *obj;

public:
  LuaVal() : mType( LUA_TNIL ), obj(NULL)
  {;}

  LuaVal( double n ) : d( n ), mType( LUA_TNUMBER ), obj(NULL)
  {;}

  LuaVal( std::string n ) : str( n ), mType( LUA_TSTRING ), obj(NULL)
  {;}

  LuaVal( const char *n ) : str( n ), mType( LUA_TSTRING ), obj(NULL)
  {;}

  LuaVal( lua_CFunction n ) : func( n ), mType( LUA_TFUNCTION ), obj(NULL)
  {;}

  LuaVal( LuaRef *o );
	
  ~LuaVal();

  // Copy 
  LuaVal( const LuaVal &lv );

  void push( lua_State *L );
};

#endif
