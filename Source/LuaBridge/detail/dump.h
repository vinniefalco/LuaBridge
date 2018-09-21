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

#pragma once

#include <sstream>
#include <string>

namespace luabridge {

std::string dumpLuaState(lua_State *L) {
	std::stringstream ostr;
	int i;
	int top = lua_gettop(L);
	ostr << "top=" << top << ":\n";
	for (i = 1; i <= top; ++i) {
		int t = lua_type(L, i);
		switch(t) {
		case LUA_TSTRING:
			ostr << "  " << i << ": '" << lua_tostring(L, i) << "'\n";
			break;
		case LUA_TBOOLEAN:
			ostr << "  " << i << ": " << 
					(lua_toboolean(L, i) ? "true" : "false") << "\n";
			break;
		case LUA_TNUMBER:
			ostr << "  " << i << ": " << lua_tonumber(L, i) << "\n";
			break;
		default:
			ostr << "  " << i << ": TYPE=" << lua_typename(L, t) << "\n";
			break;
		}
	}
	return ostr.str();
}

} // namespace luabridge
