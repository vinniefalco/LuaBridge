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

class LuaException : public std::runtime_error 
{
  std::string luaError;
  std::string file;
  long line;
  lua_State *mL;

  // Do we we need a copy constuctor? I think the default supplied one works.

public:
  LuaException( lua_State *L, const char *str, const char *filename, long fileline ) 
    : std::runtime_error(str), file(filename), line(fileline)
  {
    if( lua_gettop( L ) != 0 )
      if( lua_isstring( L, -1 ) )
        luaError = lua_tostring( L, -1 );
    mL = L;
  }

  std::string getLuaError() { return luaError; }
  lua_State *getLuaState() { return mL; }
  const char *what() const throw ()
  {
    std::stringstream ss;

    ss << "*** " << std::runtime_error::what() << " ***" << std::endl; 
    ss << "*** " << luaError << " ***" << std::endl;
    ss << "*** In file: " << file << " Line: " << line << " ***" << std::endl;

    return ss.str().c_str();
  }

  ~LuaException() throw () {}
};
