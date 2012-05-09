//==============================================================================
/*
  Copyright (C) 2012, Vinnie Falco <vinnie.falco@gmail.com>
  Copyright (C) 2007, Nathan Reed

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

/*
* Lua-registerable C function templates for getting and setting the value of
* a global/static variable through a pointer.  Also used for static props.
* These work similiarly to the function proxies above.
*/

template <typename T>
int varget_proxy (lua_State *L)
{
  T *data = (T *)lua_touserdata(L, lua_upvalueindex(1));
  tdstack<T>::push(L, *data);
  return 1;
}

template <typename T>
int varset_proxy (lua_State *L)
{
  T *data = (T *)lua_touserdata(L, lua_upvalueindex(1));
  *data = tdstack<T>::get(L, 1);
  return 0;
}

/*
* Perform variable registration in a scope.  The proxy functions are stored
* in the static __propget and __propset tables.
*/

template <typename T>
scope& scope::variable_ro (const char *name, const T *data)
{
  // Currently can't register properties at global scope.
  assert(this->name.length() > 0);

  util::findStaticTable(L, this->name.c_str());
  rawgetfield(L, -1, "__propget");
  lua_pushlightuserdata(L, (void *)data);
  lua_pushcclosure(L, &varget_proxy<T>, 1);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

template <typename T>
scope& scope::variable_ro (const char *name, T (*get) ())
{
  // Currently can't register properties at global scope.
  assert(this->name.length() > 0);

  util::findStaticTable(L, this->name.c_str());
  rawgetfield(L, -1, "__propget");
  lua_pushlightuserdata(L, (void *)get);
  lua_pushcclosure(L, &util::functionProxy<T (*) ()>::f, 1);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

template <typename T>
scope& scope::variable_rw (const char *name, T *data)
{
  // Currently can't register properties at global scope.
  assert(this->name.length() > 0);

  variable_ro<T>(name, data);
  util::findStaticTable(L, this->name.c_str());
  rawgetfield(L, -1, "__propset");
  lua_pushlightuserdata(L, (void *)data);
  lua_pushcclosure(L, &varset_proxy<T>, 1);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

template <typename T>
scope& scope::variable_rw (const char *name, T (*get) (), void (*set) (T))
{
  // Currently can't register properties at global scope.
  assert(this->name.length() > 0);

  variable_ro<T>(name, get);
  util::findStaticTable(L, this->name.c_str());
  rawgetfield(L, -1, "__propset");
  lua_pushlightuserdata(L, (void *)set);
  lua_pushcclosure(L, &util::functionProxy<void (*) (T)>::f, 1);
  rawsetfield(L, -2, name);
  lua_pop(L, 2);
  return *this;
}

/*
* Perform class registration in a scope.
*/

template <typename T>
class__<T> scope::class_ ()
{
  return class__<T>(L);
}

template <typename T, typename Base>
class__<T> scope::subclass (const char *name)
{
  assert(classname<Base>::name() != classname_unknown);
  return class__<T>(L, name, classname<Base>::name());
}

template <typename T>
class__<T> scope::class_ (const char *name)
{
  return class__<T>(L, name);
}
