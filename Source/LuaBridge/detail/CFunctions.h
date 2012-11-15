//------------------------------------------------------------------------------
/*
  https://github.com/vinniefalco/LuaBridge
  
  Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>

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

/** lua_CFunction to get a variable.

    This is used for global variables or class static data members.

    The pointer to the data is in the first upvalue.
*/
template <class T>
static int getVariable (lua_State* L)
{
  assert (lua_islightuserdata (L, lua_upvalueindex (1)));
  T const* ptr = static_cast <T const*> (lua_touserdata (L, lua_upvalueindex (1)));
  assert (ptr != 0);
  Stack <T>::push (L, *ptr);
  return 1;
}

//------------------------------------------------------------------------------
/**
    lua_CFunction to set a variable.

    This is used for global variables or class static data members.

    The pointer to the data is in the first upvalue.
*/
template <class T>
static int setVariable (lua_State* L)
{
  assert (lua_islightuserdata (L, lua_upvalueindex (1)));
  T* ptr = static_cast <T*> (lua_touserdata (L, lua_upvalueindex (1)));
  assert (ptr != 0);
  *ptr = Stack <T>::get (L, 1);
  return 0;
}

//------------------------------------------------------------------------------
/**
    lua_CFunction to call a function with a return value.

    This is used for global functions, global properties, class static methods,
    and class static properties.

    The function pointer is in the first upvalue.
*/
template <class FnPtr,
          class ReturnType = typename FuncTraits <FnPtr>::ReturnType>
struct CallFunction
{
  typedef typename FuncTraits <FnPtr>::Params Params;
  static int call (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    FnPtr const& fnptr = *static_cast <FnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params> args (L);
    Stack <typename FuncTraits <FnPtr>::ReturnType>::push (L, FuncTraits <FnPtr>::call (fnptr, args));
    return 1;
  }
};

//------------------------------------------------------------------------------
/**
    lua_CFunction to call a function with no return value.

    This is used for global functions, global properties, class static methods,
    and class static properties.

    The function pointer is in the first upvalue.
*/
template <class FnPtr>
struct CallFunction <FnPtr, void>
{
  typedef typename FuncTraits <FnPtr>::Params Params;
  static int call (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    FnPtr const& fnptr = *static_cast <FnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params> args (L);
    FuncTraits <FnPtr>::call (fnptr, args);
    return 0;
  }
};

//------------------------------------------------------------------------------
/**
    lua_CFunction to call a class member function with a return value.

    The member function pointer is in the first upvalue.
    The class userdata object is at the top of the Lua stack.
*/
template <class MemFnPtr,
          class ReturnType = typename FuncTraits <MemFnPtr>::ReturnType>
struct CallMemberFunction
{
  typedef typename FuncTraits <MemFnPtr>::ClassType T;
  typedef typename FuncTraits <MemFnPtr>::Params Params;

  static int call (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    T* const t = Userdata::get <T> (L, 1, false);
    MemFnPtr const& fnptr = *static_cast <MemFnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params, 2> args (L);
    Stack <ReturnType>::push (L, FuncTraits <MemFnPtr>::call (t, fnptr, args));
    return 1;
  }

  static int callConst (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    T const* const t = Userdata::get <T> (L, 1, true);
    MemFnPtr const& fnptr = *static_cast <MemFnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params, 2> args(L);
    Stack <ReturnType>::push (L, FuncTraits <MemFnPtr>::call (t, fnptr, args));
    return 1;
  }
};

//------------------------------------------------------------------------------
/**
    lua_CFunction to call a class member function with no return value.

    The member function pointer is in the first upvalue.
    The class userdata object is at the top of the Lua stack.
*/
template <class MemFnPtr>
struct CallMemberFunction <MemFnPtr, void>
{
  typedef typename FuncTraits <MemFnPtr>::ClassType T;
  typedef typename FuncTraits <MemFnPtr>::Params Params;

  static int call (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    T* const t = Userdata::get <T> (L, 1, false);
    MemFnPtr const& fnptr = *static_cast <MemFnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params, 2> args (L);
    FuncTraits <MemFnPtr>::call (t, fnptr, args);
    return 0;
  }

  static int callConst (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    T const* const t = Userdata::get <T> (L, 1, true);
    MemFnPtr const& fnptr = *static_cast <MemFnPtr const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    ArgList <Params, 2> args (L);
    FuncTraits <MemFnPtr>::call (t, fnptr, args);
    return 0;
  }
};

//----------------------------------------------------------------------------
/**
    lua_CFunction to call a class member lua_CFunction.

    The member function pointer is in the first upvalue.
    The class userdata object is at the top of the Lua stack.
*/
template <class T>
struct CallMemberCFunction
{
  static int call (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    typedef int (T::*MFP)(lua_State* L);
    T* const t = Userdata::get <T> (L, 1, false);
    MFP const& fnptr = *static_cast <MFP const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    return (t->*fnptr) (L);
  }

  static int callConst (lua_State* L)
  {
    assert (isfulluserdata (L, lua_upvalueindex (1)));
    typedef int (T::*MFP)(lua_State* L);
    T const* const t = Userdata::get <T> (L, 1, true);
    MFP const& fnptr = *static_cast <MFP const*> (lua_touserdata (L, lua_upvalueindex (1)));
    assert (fnptr != 0);
    return (t->*fnptr) (L);
  }
};

//----------------------------------------------------------------------------

// SFINAE Helpers

template <class MemFnPtr, bool isConst>
struct CallMemberFunctionHelper
{
  static void add (lua_State* L, char const* name, MemFnPtr mf)
  {
    new (lua_newuserdata (L, sizeof (MemFnPtr))) MemFnPtr (mf);
    lua_pushcclosure (L, &CallMemberFunction <MemFnPtr>::callConst, 1);
    lua_pushvalue (L, -1);
    rawsetfield (L, -5, name); // const table
    rawsetfield (L, -3, name); // class table
  }
};

template <class MemFnPtr>
struct CallMemberFunctionHelper <MemFnPtr, false>
{
  static void add (lua_State* L, char const* name, MemFnPtr mf)
  {
    new (lua_newuserdata (L, sizeof (MemFnPtr))) MemFnPtr (mf);
    lua_pushcclosure (L, &CallMemberFunction <MemFnPtr>::call, 1);
    rawsetfield (L, -3, name); // class table
  }
};

